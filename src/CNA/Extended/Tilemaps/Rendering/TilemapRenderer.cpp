// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/TilemapRenderer.cs. See TilemapRenderer.hpp
// for the DefaultEffect -> BasicEffect and Dictionary<K,V> -> std::unordered_map design notes.
#include "CNA/Extended/Tilemaps/Rendering/TilemapRenderer.hpp"

#include "CNA/Extended/Tilemaps/Rendering/TilemapRendererShared.hpp"
#include "CNA/Extended/Tilemaps/Tilemap.hpp"
#include "CNA/Extended/Tilemaps/TilemapImageLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapObjectLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileData.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileset.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Vector4;
    using Microsoft::Xna::Framework::Graphics::BufferUsage;
    using Microsoft::Xna::Framework::Graphics::IndexElementSize;
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;
    using Microsoft::Xna::Framework::Graphics::SetDataOptions;
    using Microsoft::Xna::Framework::Graphics::VertexPositionColorTexture;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    namespace
    {
        /**
         * @brief One contiguous run of tile quads sharing a texture and parallax factor. Local
         * helper for BuildLayerModels(TilemapTileLayer&)/RebuildLayerGroupInternal/
         * BuildMergedUngroupedModels -- never a member field's type, so (unlike LayerGroup/
         * RepeatImageLayerModel) it lives entirely in this .cpp, matching
         * TilemapWorldSpriteBatchRenderer.cpp's identical treatment of its own local grouping keys.
         */
        struct TileBatch
        {
            Texture2D* Texture;
            Vector2 ParallaxFactor;
            std::vector<VertexPositionColorTexture> Vertices;
            std::vector<int> Indices;
        };
    }

    TilemapRenderer::TilemapRenderer(GraphicsDevice& graphicsDevice)
        : graphicsDevice_(graphicsDevice), defaultEffect_(graphicsDevice), effect_(&defaultEffect_)
    {
        defaultEffect_.setTextureEnabledProperty(true);
        defaultEffect_.VertexColorEnabled = true;
    }

    TilemapRenderer::~TilemapRenderer()
    {
        Dispose();
    }

    RenderMode TilemapRenderer::getDefaultRenderModeProperty() const
    {
        ThrowIfDisposed();
        return defaultRenderMode_;
    }

    const BlendState& TilemapRenderer::getBlendStateProperty() const
    {
        ThrowIfDisposed();
        return blendState_;
    }

    void TilemapRenderer::setBlendStateProperty(const BlendState& value)
    {
        ThrowIfDisposed();
        blendState_ = value;
    }

    const SamplerState* TilemapRenderer::getSamplerStateProperty() const
    {
        ThrowIfDisposed();
        return samplerState_;
    }

    void TilemapRenderer::setSamplerStateProperty(const SamplerState* value)
    {
        ThrowIfDisposed();
        System::ArgumentNullException::ThrowIfNull(value, "value");
        samplerState_ = value;
    }

    BasicEffect& TilemapRenderer::getEffectProperty() const
    {
        ThrowIfDisposed();
        return *effect_;
    }

    void TilemapRenderer::setEffectProperty(BasicEffect& value)
    {
        ThrowIfDisposed();
        effect_ = &value;
    }

    std::vector<std::string> TilemapRenderer::getLayerGroupsProperty() const
    {
        ThrowIfDisposed();

        std::vector<std::string> names;
        names.reserve(layerGroups_.size());
        for (const auto& [name, group] : layerGroups_)
        {
            names.push_back(name);
        }
        return names;
    }

    void TilemapRenderer::LoadTilemap(Tilemap* tilemap)
    {
        ThrowIfDisposed();
        System::ArgumentNullException::ThrowIfNull(tilemap, "tilemap");

        UnloadTilemap();

        tilemap_ = tilemap;

        BuildLayerModels();
    }

    void TilemapRenderer::UnloadTilemap()
    {
        ThrowIfDisposed();

        // Clearing/erasing these containers releases every owned VertexBuffer/IndexBuffer via
        // RAII (see LayerModel.hpp's header comment); no manual Dispose() walk is needed here,
        // unlike upstream.
        layerModels_.clear();
        imageLayerModels_.clear();
        repeatImageLayerModels_.clear();
        objectLayerModels_.clear();
        layerGroups_.clear();
        layerToGroup_.Clear();
        mergedUngroupedModels_.clear();
        mergedUngroupedDirty_ = false;

        tilemap_ = nullptr;
        animatedTiles_.clear();
        groupsWithAnimations_.clear();
        mergedUngroupedHasAnimations_ = false;
    }

    void TilemapRenderer::SetDefaultRenderMode(RenderMode mode)
    {
        ThrowIfDisposed();
        defaultRenderMode_ = mode;
    }

    void TilemapRenderer::DefineLayerGroup(const std::string& groupName, const std::vector<std::string>& layerNames)
    {
        ThrowIfDisposed();
        ThrowIfNoTilemap();

        std::vector<TilemapLayer*> layers;
        layers.reserve(layerNames.size());
        for (const std::string& layerName : layerNames)
        {
            layers.push_back(tilemap_->getLayersProperty()[layerName]);
        }

        if (layerGroups_.find(groupName) != layerGroups_.end())
        {
            RemoveLayerGroup(groupName);
        }

        for (TilemapLayer* layer : layers)
        {
            std::string existingGroupName;
            if (layerToGroup_.TryGetValue(layer, existingGroupName))
            {
                auto it = layerGroups_.find(existingGroupName);
                if (it != layerGroups_.end())
                {
                    LayerGroup& existingGroup = it->second;
                    existingGroup.Layers.erase(std::remove(existingGroup.Layers.begin(), existingGroup.Layers.end(), layer),
                                                existingGroup.Layers.end());
                    existingGroup.IsDirty = true;
                }
                layerToGroup_.Remove(layer);
            }
        }

        LayerGroup group;
        group.Layers = layers;
        layerGroups_[groupName] = std::move(group);

        for (TilemapLayer* layer : layers)
        {
            layerToGroup_[layer] = groupName;
        }

        mergedUngroupedDirty_ = true;
        UpdateAnimationSets();
    }

    void TilemapRenderer::DefineLayerGroup(const std::string& groupName, int startIndex, int count)
    {
        ThrowIfDisposed();
        ThrowIfNoTilemap();

        if (startIndex < 0 || startIndex >= tilemap_->getLayersProperty().getCountProperty())
        {
            throw System::ArgumentOutOfRangeException("startIndex");
        }

        if (count <= 0 || startIndex + count > tilemap_->getLayersProperty().getCountProperty())
        {
            throw System::ArgumentOutOfRangeException("count");
        }

        std::vector<std::string> layerNames;
        layerNames.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i)
        {
            layerNames.push_back(tilemap_->getLayersProperty()[startIndex + i]->getNameProperty());
        }

        DefineLayerGroup(groupName, layerNames);
    }

    void TilemapRenderer::RemoveLayerGroup(const std::string& groupName)
    {
        ThrowIfDisposed();

        auto it = layerGroups_.find(groupName);
        if (it != layerGroups_.end())
        {
            LayerGroup& group = it->second;

            for (TilemapLayer* layer : group.Layers)
            {
                layerToGroup_.Remove(layer);
            }

            layerGroups_.erase(it);
            mergedUngroupedDirty_ = true;
            UpdateAnimationSets();
        }
    }

    bool TilemapRenderer::HasLayerGroup(const std::string& groupName) const
    {
        ThrowIfDisposed();
        return layerGroups_.find(groupName) != layerGroups_.end();
    }

    void TilemapRenderer::MarkGroupDirty(const std::string& groupName)
    {
        ThrowIfDisposed();

        auto it = layerGroups_.find(groupName);
        if (it == layerGroups_.end())
        {
            throw System::ArgumentException("Layer group '" + groupName + "' does not exist.", "groupName");
        }

        it->second.IsDirty = true;
    }

    void TilemapRenderer::RebuildLayerGroup(const std::string& groupName)
    {
        ThrowIfDisposed();

        auto it = layerGroups_.find(groupName);
        if (it == layerGroups_.end())
        {
            throw System::ArgumentException("Layer group '" + groupName + "' does not exist.", "groupName");
        }

        RebuildLayerGroupInternal(it->second);
    }

    void TilemapRenderer::BeginDraw(OrthographicCamera& camera)
    {
        ThrowIfDisposed();
        ThrowIfNoTilemap();

        if (isDrawing_)
        {
            throw System::InvalidOperationException("BeginDraw already called. Call EndDraw first.");
        }

        isDrawing_ = true;
        camera_ = &camera;

        SaveGraphicsDeviceState();

        graphicsDevice_.getSamplerStatesProperty()[0] = *samplerState_;
        graphicsDevice_.setBlendStateProperty(blendState_);

        Matrix view = camera.GetViewMatrix();

        // Snap view translation to integer pixels to prevent sub-pixel edge bleeding when
        // LinearClamp is configured. Matches TilemapSpriteBatchRenderer's identical behavior.
        view.M41 = std::round(view.M41);
        view.M42 = std::round(view.M42);

        effect_->View = view;
        const Viewport& viewport = graphicsDevice_.getViewportProperty();
        effect_->Projection = Matrix::CreateOrthographicOffCenter(
            0.0f, static_cast<float>(viewport.getWidthProperty()),
            static_cast<float>(viewport.getHeightProperty()), 0.0f,
            0.0f, 1.0f);
        effect_->World = Matrix::getIdentityProperty();
    }

    void TilemapRenderer::DrawLayerGroup(const std::string& groupName)
    {
        ThrowIfDisposed();
        ThrowIfNotDrawing();

        auto it = layerGroups_.find(groupName);
        if (it == layerGroups_.end())
        {
            throw System::ArgumentException("Layer group '" + groupName + "' does not exist.", "groupName");
        }

        LayerGroup& group = it->second;
        if (group.IsDirty)
        {
            RebuildLayerGroupInternal(group);
        }

        for (const LayerModel& model : group.MergedModels)
        {
            ApplyParallaxWorld(model.getParallaxFactorProperty());
            DrawLayerModel(model);
        }
    }

    void TilemapRenderer::DrawLayer(const std::string& layerName)
    {
        ThrowIfDisposed();
        ThrowIfNotDrawing();

        DrawLayerInternal(*tilemap_->getLayersProperty()[layerName]);
    }

    void TilemapRenderer::DrawLayer(int layerIndex)
    {
        ThrowIfDisposed();
        ThrowIfNotDrawing();

        if (layerIndex < 0 || layerIndex >= tilemap_->getLayersProperty().getCountProperty())
        {
            throw System::ArgumentOutOfRangeException("layerIndex");
        }

        DrawLayerInternal(*tilemap_->getLayersProperty()[layerIndex]);
    }

    void TilemapRenderer::DrawLayers(OrthographicCamera& camera, const std::vector<std::string>& layerNames)
    {
        ThrowIfDisposed();
        ThrowIfNoTilemap();

        BeginDraw(camera);

        for (const std::string& name : layerNames)
        {
            DrawLayerInternal(*tilemap_->getLayersProperty()[name]);
        }

        EndDraw();
    }

    void TilemapRenderer::EndDraw()
    {
        ThrowIfDisposed();
        ThrowIfNotDrawing();

        isDrawing_ = false;
        effect_->World = Matrix::getIdentityProperty();
        camera_ = nullptr;

        RestoreGraphicsDeviceState();
    }

    void TilemapRenderer::Draw(OrthographicCamera& camera)
    {
        ThrowIfDisposed();
        ThrowIfNoTilemap();

        BeginDraw(camera);

        drawnGroupsBuffer_.clear();

        if (defaultRenderMode_ == RenderMode::Merged)
        {
            if (mergedUngroupedDirty_)
            {
                BuildMergedUngroupedModels();
            }

            // Find the last ungrouped tile layer so image/object layers that follow it in
            // z-order are drawn after the merged buffer, not before.
            const TilemapLayerCollection& layers = tilemap_->getLayersProperty();
            int lastTileLayerIndex = -1;
            for (int i = 0; i < layers.getCountProperty(); ++i)
            {
                TilemapLayer* layer = layers[i];
                if (dynamic_cast<TilemapTileLayer*>(layer) != nullptr && !layerToGroup_.ContainsKey(layer))
                {
                    lastTileLayerIndex = i;
                }
            }

            for (int i = 0; i <= lastTileLayerIndex; ++i)
            {
                TilemapLayer* layer = layers[i];
                std::string groupName;
                if (layerToGroup_.TryGetValue(layer, groupName))
                {
                    if (drawnGroupsBuffer_.insert(groupName).second)
                    {
                        DrawLayerGroup(groupName);
                    }
                }
                else if (dynamic_cast<TilemapImageLayer*>(layer) != nullptr || dynamic_cast<TilemapObjectLayer*>(layer) != nullptr)
                {
                    DrawLayerInternal(*layer);
                }
            }

            for (const LayerModel& model : mergedUngroupedModels_)
            {
                ApplyParallaxWorld(model.getParallaxFactorProperty());
                DrawLayerModel(model);
            }

            for (int i = lastTileLayerIndex + 1; i < layers.getCountProperty(); ++i)
            {
                TilemapLayer* layer = layers[i];
                std::string groupName;
                if (layerToGroup_.TryGetValue(layer, groupName))
                {
                    if (drawnGroupsBuffer_.insert(groupName).second)
                    {
                        DrawLayerGroup(groupName);
                    }
                }
                else if (dynamic_cast<TilemapImageLayer*>(layer) != nullptr || dynamic_cast<TilemapObjectLayer*>(layer) != nullptr)
                {
                    DrawLayerInternal(*layer);
                }
            }
        }
        else
        {
            for (const std::unique_ptr<TilemapLayer>& layerPtr : tilemap_->getLayersProperty())
            {
                TilemapLayer* layer = layerPtr.get();
                std::string groupName;
                if (layerToGroup_.TryGetValue(layer, groupName))
                {
                    if (drawnGroupsBuffer_.insert(groupName).second)
                    {
                        DrawLayerGroup(groupName);
                    }
                }
                else
                {
                    DrawLayerInternal(*layer);
                }
            }
        }

        EndDraw();
    }

    void TilemapRenderer::Update(const GameTime& gameTime)
    {
        ThrowIfDisposed();

        if (animatedTiles_.empty())
        {
            return;
        }

        const float dt = static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
        bool anyFrameChanged = false;

        for (TilemapTileData* tileData : animatedTiles_)
        {
            TilemapTileAnimation* animation = tileData->getAnimationProperty();
            const int prevFrame = animation->getCurrentFrameIndexProperty();
            animation->Update(dt);
            if (animation->getCurrentFrameIndexProperty() != prevFrame)
            {
                anyFrameChanged = true;
            }
        }

        if (anyFrameChanged)
        {
            for (const std::string& groupName : groupsWithAnimations_)
            {
                MarkGroupDirty(groupName);
            }

            if (mergedUngroupedHasAnimations_)
            {
                mergedUngroupedDirty_ = true;
            }
        }
    }

    void TilemapRenderer::SaveGraphicsDeviceState()
    {
        ThrowIfDisposed();

        savedBlendState_ = graphicsDevice_.getBlendStateProperty();
        savedSamplerState_ = graphicsDevice_.getSamplerStatesProperty()[0];
        savedRasterizerState_ = graphicsDevice_.getRasterizerStateProperty();
        savedDepthStencilState_ = graphicsDevice_.getDepthStencilStateProperty();
    }

    void TilemapRenderer::RestoreGraphicsDeviceState()
    {
        ThrowIfDisposed();

        graphicsDevice_.setBlendStateProperty(savedBlendState_);
        graphicsDevice_.getSamplerStatesProperty()[0] = savedSamplerState_;
        graphicsDevice_.setRasterizerStateProperty(savedRasterizerState_);
        graphicsDevice_.setDepthStencilStateProperty(savedDepthStencilState_);
    }

    void TilemapRenderer::Dispose()
    {
        if (isDisposed_)
        {
            return;
        }

        UnloadTilemap();
        isDisposed_ = true;
    }

    void TilemapRenderer::BuildLayerModels()
    {
        for (const std::unique_ptr<TilemapLayer>& layerPtr : tilemap_->getLayersProperty())
        {
            TilemapLayer* layer = layerPtr.get();

            if (auto* tileLayer = dynamic_cast<TilemapTileLayer*>(layer))
            {
                std::vector<LayerModel> models = BuildLayerModels(*tileLayer);
                if (!models.empty())
                {
                    layerModels_.emplace(tileLayer, std::move(models));
                }
            }
            else if (auto* imageLayer = dynamic_cast<TilemapImageLayer*>(layer))
            {
                if (imageLayer->getRepeatXProperty() || imageLayer->getRepeatYProperty())
                {
                    std::optional<RepeatImageLayerModel> model = BuildRepeatImageLayerModel(*imageLayer);
                    if (model.has_value())
                    {
                        repeatImageLayerModels_.emplace(imageLayer, std::move(*model));
                    }
                }
                else
                {
                    std::optional<LayerModel> model = BuildImageLayerModel(*imageLayer);
                    if (model.has_value())
                    {
                        imageLayerModels_.emplace(imageLayer, std::move(*model));
                    }
                }
            }
            else if (auto* objectLayer = dynamic_cast<TilemapObjectLayer*>(layer))
            {
                std::vector<LayerModel> models = BuildObjectLayerModels(*objectLayer);
                if (!models.empty())
                {
                    objectLayerModels_.emplace(objectLayer, std::move(models));
                }
            }
        }

        mergedUngroupedDirty_ = true;
        BuildAnimatedTilesList();
        UpdateAnimationSets();
    }

    std::optional<LayerModel> TilemapRenderer::BuildImageLayerModel(TilemapImageLayer& imageLayer)
    {
        Texture2D* texture = imageLayer.getTextureProperty();
        if (texture == nullptr)
        {
            return std::nullopt;
        }

        std::vector<VertexPositionColorTexture> vertices;
        std::vector<int> indices;

        const Vector2 position = imageLayer.getPositionProperty() + imageLayer.getOffsetProperty();
        const Rectangle sourceRect(0, 0, texture->getWidthProperty(), texture->getHeightProperty());
        const Color color(1.0f, 1.0f, 1.0f, imageLayer.getOpacityProperty());

        AddTileQuad(vertices, indices, position, texture->getWidthProperty(), texture->getHeightProperty(), sourceRect,
                    TilemapTileFlipFlags::None, *texture, color);

        LayerModel model = CreateLayerModel(graphicsDevice_, vertices, indices, texture);
        model.setParallaxFactorProperty(imageLayer.getParallaxFactorProperty());
        return model;
    }

    std::optional<TilemapRenderer::RepeatImageLayerModel> TilemapRenderer::BuildRepeatImageLayerModel(TilemapImageLayer& imageLayer)
    {
        Texture2D* texture = imageLayer.getTextureProperty();
        if (texture == nullptr)
        {
            return std::nullopt;
        }

        DynamicVertexBuffer vertexBuffer(graphicsDevice_, VertexPositionColorTexture::getVertexDeclarationStatic(), 4,
                                          BufferUsage::WriteOnly);
        const Color transparentBlack(0.0f, 0.0f, 0.0f, 0.0f);
        const VertexPositionColorTexture zeroedVertices[4] = {
            VertexPositionColorTexture(Vector3::Zero, transparentBlack, Vector2::Zero),
            VertexPositionColorTexture(Vector3::Zero, transparentBlack, Vector2::Zero),
            VertexPositionColorTexture(Vector3::Zero, transparentBlack, Vector2::Zero),
            VertexPositionColorTexture(Vector3::Zero, transparentBlack, Vector2::Zero),
        };
        vertexBuffer.SetData(zeroedVertices, 0, 4, SetDataOptions::None);

        IndexBuffer indexBuffer(graphicsDevice_, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
        const std::uint16_t quadIndices[6] = {0, 1, 2, 1, 3, 2};
        indexBuffer.SetData(quadIndices, 6);

        return RepeatImageLayerModel(std::move(vertexBuffer), std::move(indexBuffer), texture,
                                      imageLayer.getRepeatXProperty(), imageLayer.getRepeatYProperty(),
                                      imageLayer.getPositionProperty() + imageLayer.getOffsetProperty(),
                                      Color(1.0f, 1.0f, 1.0f, imageLayer.getOpacityProperty()));
    }

    // Only TilemapTileObject instances are rendered; other object types are skipped. Objects are
    // batched by texture to minimize draw calls -- a new batch opens whenever the texture changes,
    // preserving back-to-front draw order within the layer. Tile object positions follow the
    // Tiled convention: the position is at the bottom-left corner, and rotation is clockwise in
    // screen (Y-down) space. Vertex positions are pre-computed at load time so rotation is fully
    // resolved in the GPU buffer.
    std::vector<LayerModel> TilemapRenderer::BuildObjectLayerModels(TilemapObjectLayer& objectLayer)
    {
        std::vector<LayerModel> models;
        std::vector<VertexPositionColorTexture> currentVertices;
        std::vector<int> currentIndices;
        Texture2D* currentTexture = nullptr;

        const Color layerColor(1.0f, 1.0f, 1.0f, objectLayer.getOpacityProperty());

        for (const std::unique_ptr<TilemapObject>& objPtr : objectLayer.getObjectsProperty())
        {
            TilemapObject* obj = objPtr.get();
            if (!obj->getIsVisibleProperty())
            {
                continue;
            }

            auto* tileObj = dynamic_cast<TilemapTileObject*>(obj);
            if (tileObj == nullptr)
            {
                continue;
            }

            TilemapTileset* tileset = nullptr;
            const int localId = tileObj->getTileProperty().GetLocalId(tilemap_->getTilesetsProperty(), tileset);

            if (tileset == nullptr)
            {
                continue;
            }

            Texture2D* texture = nullptr;
            Rectangle sourceRect;
            tileset->GetRenderSource(localId, texture, sourceRect);

            if (texture == nullptr)
            {
                continue;
            }

            if (currentTexture != nullptr && currentTexture != texture)
            {
                LayerModel flushed = CreateLayerModel(graphicsDevice_, currentVertices, currentIndices, currentTexture);
                flushed.setParallaxFactorProperty(objectLayer.getParallaxFactorProperty());
                models.push_back(std::move(flushed));
                currentVertices.clear();
                currentIndices.clear();
            }

            currentTexture = texture;

            AddObjectTileQuad(currentVertices, currentIndices, tileObj->getPositionProperty(), tileObj->getSizeProperty().X,
                               tileObj->getSizeProperty().Y, tileObj->getRotationProperty(), sourceRect,
                               tileObj->getTileProperty().getFlipFlagsProperty(), *texture, layerColor);
        }

        if (!currentVertices.empty())
        {
            LayerModel last = CreateLayerModel(graphicsDevice_, currentVertices, currentIndices, currentTexture);
            last.setParallaxFactorProperty(objectLayer.getParallaxFactorProperty());
            models.push_back(std::move(last));
        }

        return models;
    }

    void TilemapRenderer::AddObjectTileQuad(std::vector<VertexPositionColorTexture>& vertices, std::vector<int>& indices,
                                             const Vector2& pivot, float width, float height, float rotation,
                                             const Rectangle& sourceRect, TilemapTileFlipFlags flipFlags, const Texture2D& texture,
                                             const Color& color)
    {
        const float cosR = std::cos(rotation);
        const float sinR = std::sin(rotation);

        // Corners relative to the bottom-left pivot before rotation. In Y-down space the top edge
        // is at y = -height. Clockwise rotation: rotate(vx, vy) = (vx*cos - vy*sin, vx*sin + vy*cos).
        const Vector3 topLeft(pivot.X + height * sinR, pivot.Y - height * cosR, 0.0f);
        const Vector3 topRight(pivot.X + width * cosR + height * sinR, pivot.Y + width * sinR - height * cosR, 0.0f);
        const Vector3 bottomLeft(pivot.X, pivot.Y, 0.0f);
        const Vector3 bottomRight(pivot.X + width * cosR, pivot.Y + width * sinR, 0.0f);

        const std::array<Vector2, 4> uvs = CalculateTextureCoordinates(sourceRect, flipFlags, texture);

        const auto vertexOffset = static_cast<int>(vertices.size());
        vertices.emplace_back(topLeft, color, uvs[0]);
        vertices.emplace_back(topRight, color, uvs[1]);
        vertices.emplace_back(bottomLeft, color, uvs[2]);
        vertices.emplace_back(bottomRight, color, uvs[3]);

        indices.push_back(vertexOffset);
        indices.push_back(vertexOffset + 1);
        indices.push_back(vertexOffset + 2);
        indices.push_back(vertexOffset + 1);
        indices.push_back(vertexOffset + 3);
        indices.push_back(vertexOffset + 2);
    }

    std::vector<LayerModel> TilemapRenderer::BuildLayerModels(TilemapTileLayer& tileLayer)
    {
        std::vector<TileBatch> batches;
        const Color layerColor(1.0f, 1.0f, 1.0f, tileLayer.getOpacityProperty());
        const Vector2 layerParallax = tileLayer.getParallaxFactorProperty();

        for (const TilemapTileEntry& entry : tileLayer.GetTiles())
        {
            TilemapTileset* tileset = nullptr;
            const int localId = entry.Tile.GetLocalId(tilemap_->getTilesetsProperty(), tileset);

            if (tileset == nullptr)
            {
                continue;
            }

            Texture2D* texture = nullptr;
            Rectangle sourceRect;
            tileset->GetRenderSource(localId, texture, sourceRect);

            if (texture == nullptr)
            {
                continue;
            }

            const Point worldPos = tilemap_->TileToWorldPosition(entry.X, entry.Y);
            Vector2 position = Vector2(static_cast<float>(worldPos.X), static_cast<float>(worldPos.Y)) + tileLayer.getOffsetProperty()
                + tileset->getTileOffsetProperty();
            // Tiled bottom-aligns all tiles: shifts oversized tiles up and undersized tiles down.
            position.Y += static_cast<float>(tileLayer.getTileHeightProperty() - sourceRect.Height);

            if (batches.empty() || batches.back().Texture != texture || batches.back().ParallaxFactor != layerParallax)
            {
                batches.push_back(TileBatch{texture, layerParallax, {}, {}});
            }

            TileBatch& last = batches.back();
            AddTileQuad(last.Vertices, last.Indices, position, sourceRect.Width, sourceRect.Height, sourceRect,
                        entry.Tile.getFlipFlagsProperty(), *texture, layerColor);
        }

        std::vector<LayerModel> result;
        for (TileBatch& batch : batches)
        {
            if (batch.Vertices.empty())
            {
                continue;
            }

            LayerModel model = CreateLayerModel(graphicsDevice_, batch.Vertices, batch.Indices, batch.Texture);
            model.setParallaxFactorProperty(layerParallax);
            result.push_back(std::move(model));
        }

        return result;
    }

    void TilemapRenderer::ApplyParallaxWorld(const Vector2& parallaxFactor)
    {
        if (parallaxFactor == Vector2::One)
        {
            effect_->World = Matrix::getIdentityProperty();
            return;
        }

        // The viewport top-left in world space is the correct parallax scroll reference, not
        // camera.Position (which is the top-left only at zoom=1).
        const RectangleF camBounds = camera_->getBoundingRectangleProperty();
        const Vector2 topLeft(camBounds.X, camBounds.Y);
        const Vector2 parallaxOrigin = tilemap_->getParallaxOriginProperty();
        const Vector2 offset = (Vector2::One - parallaxFactor) * (topLeft - parallaxOrigin);
        effect_->World = Matrix::CreateTranslation(offset.X, offset.Y, 0.0f);
    }

    void TilemapRenderer::DrawLayerModel(const LayerModel& model)
    {
        effect_->setTextureProperty(model.getTextureProperty());

        graphicsDevice_.SetVertexBuffer(&model.getVertexBufferProperty());
        graphicsDevice_.SetIndexBuffer(&model.getIndexBufferProperty());

        effect_->Apply();
        graphicsDevice_.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0,
                                               model.getVertexBufferProperty().getVertexCountProperty(), 0,
                                               model.getPrimitiveCountProperty());
    }

    void TilemapRenderer::DrawRepeatImageLayerModel(RepeatImageLayerModel& model)
    {
        const SamplerState previous = graphicsDevice_.getSamplerStatesProperty()[0];

        // Repeating image layers use UV coordinates that extend beyond [0, 1], so a wrap sampler
        // is required. Use the wrap equivalent of the user's configured sampler state to preserve
        // their point vs. linear preference.
        graphicsDevice_.getSamplerStatesProperty()[0] = *GetWrapSamplerState(samplerState_);

        effect_->setTextureProperty(model.texture);
        graphicsDevice_.SetVertexBuffer(&model.vertexBuffer);
        graphicsDevice_.SetIndexBuffer(&model.indexBuffer);

        effect_->Apply();
        graphicsDevice_.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, model.vertexBuffer.getVertexCountProperty(), 0, 2);

        graphicsDevice_.getSamplerStatesProperty()[0] = previous;
    }

    void TilemapRenderer::DrawLayerInternal(TilemapLayer& layer)
    {
        if (!layer.getIsVisibleProperty())
        {
            return;
        }

        if (auto* tileLayer = dynamic_cast<TilemapTileLayer*>(&layer))
        {
            // Find the corresponding layer models by layer identity, not by index. Index-based
            // lookup breaks when non-tile layers are interleaved with tile layers because
            // layerModels_ only contains entries for layers that produced at least one tile.
            auto it = layerModels_.find(tileLayer);
            if (it != layerModels_.end())
            {
                ApplyParallaxWorld(tileLayer->getParallaxFactorProperty());

                for (const LayerModel& tileModel : it->second)
                {
                    DrawLayerModel(tileModel);
                }
            }

            return;
        }

        if (auto* imageLayer = dynamic_cast<TilemapImageLayer*>(&layer))
        {
            auto repeatIt = repeatImageLayerModels_.find(imageLayer);
            if (repeatIt != repeatImageLayerModels_.end())
            {
                RepeatImageLayerModel& repeatModel = repeatIt->second;
                const Matrix invViewProj = Matrix::Invert(effect_->View * effect_->Projection);
                const Vector2 parallaxOffset = camera_->getPositionProperty() * (Vector2::One - imageLayer->getParallaxFactorProperty());
                repeatModel.UpdateVertices(invViewProj, parallaxOffset);
                ApplyParallaxWorld(imageLayer->getParallaxFactorProperty());
                DrawRepeatImageLayerModel(repeatModel);
            }
            else
            {
                auto imageIt = imageLayerModels_.find(imageLayer);
                if (imageIt != imageLayerModels_.end())
                {
                    ApplyParallaxWorld(imageLayer->getParallaxFactorProperty());
                    DrawLayerModel(imageIt->second);
                }
            }

            return;
        }

        if (auto* objectLayer = dynamic_cast<TilemapObjectLayer*>(&layer))
        {
            auto it = objectLayerModels_.find(objectLayer);
            if (it != objectLayerModels_.end())
            {
                for (const LayerModel& model : it->second)
                {
                    ApplyParallaxWorld(model.getParallaxFactorProperty());
                    DrawLayerModel(model);
                }
            }
        }
    }

    void TilemapRenderer::RebuildLayerGroupInternal(LayerGroup& group)
    {
        group.MergedModels.clear();

        // Accumulate one batch per contiguous run of tiles with the same texture and parallax
        // factor. A new batch is opened on either change to preserve back-to-front draw order and
        // to ensure each batch can be drawn with a single World matrix.
        std::vector<TileBatch> orderedBatches;

        for (TilemapLayer* layer : group.Layers)
        {
            if (!layer->getIsVisibleProperty())
            {
                continue;
            }

            auto* tileLayer = dynamic_cast<TilemapTileLayer*>(layer);
            if (tileLayer == nullptr)
            {
                continue;
            }

            // Bake per-layer opacity into the vertex color so that layers with different opacity
            // values render correctly within a single merged draw call.
            const Color layerColor(1.0f, 1.0f, 1.0f, tileLayer->getOpacityProperty());
            const Vector2 layerParallax = layer->getParallaxFactorProperty();

            for (const TilemapTileEntry& entry : tileLayer->GetTiles())
            {
                TilemapTileset* tileset = nullptr;
                const int localId = entry.Tile.GetLocalId(tilemap_->getTilesetsProperty(), tileset);

                if (tileset == nullptr)
                {
                    continue;
                }

                Texture2D* texture = nullptr;
                Rectangle sourceRect;
                tileset->GetRenderSource(localId, texture, sourceRect);

                if (texture == nullptr)
                {
                    continue;
                }

                const Point worldPos = tilemap_->TileToWorldPosition(entry.X, entry.Y);
                Vector2 position = Vector2(static_cast<float>(worldPos.X), static_cast<float>(worldPos.Y)) + tileLayer->getOffsetProperty()
                    + tileset->getTileOffsetProperty();
                // Tiled bottom-aligns all tiles: shifts oversized tiles up and undersized tiles down.
                position.Y += static_cast<float>(tileLayer->getTileHeightProperty() - sourceRect.Height);

                if (orderedBatches.empty() || orderedBatches.back().Texture != texture
                    || orderedBatches.back().ParallaxFactor != layerParallax)
                {
                    orderedBatches.push_back(TileBatch{texture, layerParallax, {}, {}});
                }

                TileBatch& last = orderedBatches.back();
                AddTileQuad(last.Vertices, last.Indices, position, sourceRect.Width, sourceRect.Height, sourceRect,
                            entry.Tile.getFlipFlagsProperty(), *texture, layerColor);
            }
        }

        for (TileBatch& batch : orderedBatches)
        {
            if (batch.Vertices.empty())
            {
                continue;
            }

            LayerModel model = CreateLayerModel(graphicsDevice_, batch.Vertices, batch.Indices, batch.Texture);
            model.setParallaxFactorProperty(batch.ParallaxFactor);
            group.MergedModels.push_back(std::move(model));
        }

        group.IsDirty = false;
    }

    void TilemapRenderer::BuildMergedUngroupedModels()
    {
        mergedUngroupedModels_.clear();

        std::vector<TileBatch> batches;

        for (const std::unique_ptr<TilemapLayer>& layerPtr : tilemap_->getLayersProperty())
        {
            TilemapLayer* layer = layerPtr.get();
            if (layerToGroup_.ContainsKey(layer) || !layer->getIsVisibleProperty())
            {
                continue;
            }

            auto* tileLayer = dynamic_cast<TilemapTileLayer*>(layer);
            if (tileLayer == nullptr)
            {
                continue;
            }

            const Color layerColor(1.0f, 1.0f, 1.0f, tileLayer->getOpacityProperty());
            const Vector2 layerParallax = layer->getParallaxFactorProperty();

            for (const TilemapTileEntry& entry : tileLayer->GetTiles())
            {
                TilemapTileset* tileset = nullptr;
                const int localId = entry.Tile.GetLocalId(tilemap_->getTilesetsProperty(), tileset);

                if (tileset == nullptr)
                {
                    continue;
                }

                Texture2D* texture = nullptr;
                Rectangle sourceRect;
                tileset->GetRenderSource(localId, texture, sourceRect);

                if (texture == nullptr)
                {
                    continue;
                }

                const Point worldPos = tilemap_->TileToWorldPosition(entry.X, entry.Y);
                Vector2 position = Vector2(static_cast<float>(worldPos.X), static_cast<float>(worldPos.Y)) + tileLayer->getOffsetProperty()
                    + tileset->getTileOffsetProperty();

                // Tiled bottom-aligns all tiles: shifts oversized tiles up and undersized tiles down.
                position.Y += static_cast<float>(tileLayer->getTileHeightProperty() - sourceRect.Height);

                if (batches.empty() || batches.back().Texture != texture || batches.back().ParallaxFactor != layerParallax)
                {
                    batches.push_back(TileBatch{texture, layerParallax, {}, {}});
                }

                TileBatch& last = batches.back();
                AddTileQuad(last.Vertices, last.Indices, position, sourceRect.Width, sourceRect.Height, sourceRect,
                            entry.Tile.getFlipFlagsProperty(), *texture, layerColor);
            }
        }

        for (TileBatch& batch : batches)
        {
            if (batch.Vertices.empty())
            {
                continue;
            }

            LayerModel model = CreateLayerModel(graphicsDevice_, batch.Vertices, batch.Indices, batch.Texture);
            model.setParallaxFactorProperty(batch.ParallaxFactor);
            mergedUngroupedModels_.push_back(std::move(model));
        }

        mergedUngroupedDirty_ = false;
    }

    void TilemapRenderer::BuildAnimatedTilesList()
    {
        animatedTiles_.clear();
        if (tilemap_ == nullptr)
        {
            return;
        }

        for (const std::unique_ptr<TilemapTileset>& tilesetPtr : tilemap_->getTilesetsProperty())
        {
            const std::vector<TilemapTileData*>& animated = tilesetPtr->GetAnimatedTiles();
            for (TilemapTileData* tile : animated)
            {
                animatedTiles_.push_back(tile);
            }
        }
    }

    void TilemapRenderer::UpdateAnimationSets()
    {
        groupsWithAnimations_.clear();
        mergedUngroupedHasAnimations_ = false;

        if (tilemap_ == nullptr || animatedTiles_.empty())
        {
            return;
        }

        for (const auto& [groupName, group] : layerGroups_)
        {
            for (TilemapLayer* layer : group.Layers)
            {
                if (dynamic_cast<TilemapTileLayer*>(layer) != nullptr)
                {
                    groupsWithAnimations_.insert(groupName);
                    break;
                }
            }
        }

        for (const std::unique_ptr<TilemapLayer>& layerPtr : tilemap_->getLayersProperty())
        {
            TilemapLayer* layer = layerPtr.get();
            if (!layerToGroup_.ContainsKey(layer) && dynamic_cast<TilemapTileLayer*>(layer) != nullptr)
            {
                mergedUngroupedHasAnimations_ = true;
                break;
            }
        }
    }

    void TilemapRenderer::ThrowIfDisposed() const
    {
        System::ObjectDisposedException::ThrowIf(isDisposed_, "TilemapRenderer");
    }

    void TilemapRenderer::ThrowIfNoTilemap() const
    {
        if (tilemap_ == nullptr)
        {
            throw System::InvalidOperationException("No tilemap loaded. Call LoadTilemap first.");
        }
    }

    void TilemapRenderer::ThrowIfNotDrawing() const
    {
        if (!isDrawing_)
        {
            throw System::InvalidOperationException("Not in drawing state. Call BeginDraw first.");
        }
    }

    void TilemapRenderer::RepeatImageLayerModel::UpdateVertices(const Matrix& inverseViewProjection, const Vector2& parallaxOffset)
    {
        const int texW = texture->getWidthProperty();
        const int texH = texture->getHeightProperty();

        // Unproject the screen corners to world space. The GPU represents the visible screen as
        // a -1 to +1 range on both axes, with Y inverted (+1 = screen top, -1 = screen bottom).
        // Transforming these fixed corners back to world space gives correct bounds regardless of
        // viewport adapter scale (virtual vs physical resolution).
        const Vector4 tlWorld = Vector4::Transform(Vector4(-1.0f, 1.0f, 0.0f, 1.0f), inverseViewProjection);
        const Vector4 brWorld = Vector4::Transform(Vector4(1.0f, -1.0f, 0.0f, 1.0f), inverseViewProjection);

        const float screenLeft = tlWorld.X / tlWorld.W;
        const float screenTop = tlWorld.Y / tlWorld.W;
        const float screenRight = brWorld.X / brWorld.W;
        const float screenBottom = brWorld.Y / brWorld.W;

        float left, right, top, bottom;
        float uLeft, uRight, vTop, vBottom;

        if (repeatX)
        {
            // Vertex position = unprojected world X minus parallax offset (which is added back
            // by the World matrix in ApplyParallaxWorld before drawing). The vertex position is
            // also the parallax-corrected world X, so UVs use it directly: this makes the texture
            // scroll at the parallax factor's speed.
            left = screenLeft - parallaxOffset.X;
            right = screenRight - parallaxOffset.X;
            uLeft = (left - layerPosition.X) / static_cast<float>(texW);
            uRight = (right - layerPosition.X) / static_cast<float>(texW);
        }
        else
        {
            left = layerPosition.X;
            right = layerPosition.X + static_cast<float>(texW);
            uLeft = 0.0f;
            uRight = 1.0f;
        }

        if (repeatY)
        {
            top = screenTop - parallaxOffset.Y;
            bottom = screenBottom - parallaxOffset.Y;
            vTop = (top - layerPosition.Y) / static_cast<float>(texH);
            vBottom = (bottom - layerPosition.Y) / static_cast<float>(texH);
        }
        else
        {
            top = layerPosition.Y;
            bottom = layerPosition.Y + static_cast<float>(texH);
            vTop = 0.0f;
            vBottom = 1.0f;
        }

        const VertexPositionColorTexture verts[4] = {
            VertexPositionColorTexture(Vector3(left, top, 0.0f), color, Vector2(uLeft, vTop)),
            VertexPositionColorTexture(Vector3(right, top, 0.0f), color, Vector2(uRight, vTop)),
            VertexPositionColorTexture(Vector3(left, bottom, 0.0f), color, Vector2(uLeft, vBottom)),
            VertexPositionColorTexture(Vector3(right, bottom, 0.0f), color, Vector2(uRight, vBottom)),
        };

        vertexBuffer.SetData(verts, 0, 4, SetDataOptions::None);
    }
}
