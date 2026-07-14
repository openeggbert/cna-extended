// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/TilemapWorldRenderer.cs. See
// TilemapWorldRenderer.hpp for design notes.
#include "CNA/Extended/Tilemaps/Rendering/TilemapWorldRenderer.hpp"

#include "CNA/Extended/Tilemaps/Rendering/TilemapRendererShared.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileset.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

#include <cmath>

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;
    using Microsoft::Xna::Framework::Graphics::VertexPositionColorTexture;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    namespace
    {
        /**
         * @brief Accumulates tile quads for one (WorldDepth, texture, parallax factor) group
         * during Load(). Local to Load()'s implementation only -- see this file's header comment
         * for why a linear-scan std::vector is used here instead of an unordered_map keyed on a
         * Vector2-containing struct.
         */
        struct TileAccumulator
        {
            Texture2D* Texture;
            int WorldDepth;
            Vector2 ParallaxFactor;
            std::vector<VertexPositionColorTexture> Vertices;
            std::vector<int> Indices;
        };

        TileAccumulator& FindOrCreateAccumulator(std::vector<TileAccumulator>& accumulators, int worldDepth, Texture2D* texture,
                                                  const Vector2& parallaxFactor)
        {
            for (TileAccumulator& candidate : accumulators)
            {
                if (candidate.WorldDepth == worldDepth && candidate.Texture == texture && candidate.ParallaxFactor == parallaxFactor)
                {
                    return candidate;
                }
            }

            accumulators.push_back(TileAccumulator{texture, worldDepth, parallaxFactor, {}, {}});
            return accumulators.back();
        }
    }

    TilemapWorldRenderer::TilemapWorldRenderer(GraphicsDevice& graphicsDevice)
        : graphicsDevice_(graphicsDevice), defaultEffect_(graphicsDevice), effect_(&defaultEffect_)
    {
        defaultEffect_.setTextureEnabledProperty(true);
        defaultEffect_.VertexColorEnabled = true;
    }

    TilemapWorldRenderer::~TilemapWorldRenderer()
    {
        Dispose();
    }

    const BlendState& TilemapWorldRenderer::getBlendStateProperty() const
    {
        ThrowIfDisposed();
        return blendState_;
    }

    void TilemapWorldRenderer::setBlendStateProperty(const BlendState& value)
    {
        ThrowIfDisposed();
        blendState_ = value;
    }

    const SamplerState* TilemapWorldRenderer::getSamplerStateProperty() const
    {
        ThrowIfDisposed();
        return samplerState_;
    }

    void TilemapWorldRenderer::setSamplerStateProperty(const SamplerState* value)
    {
        ThrowIfDisposed();
        System::ArgumentNullException::ThrowIfNull(value, "value");
        samplerState_ = value;
    }

    BasicEffect& TilemapWorldRenderer::getEffectProperty() const
    {
        ThrowIfDisposed();
        return *effect_;
    }

    void TilemapWorldRenderer::setEffectProperty(BasicEffect& value)
    {
        ThrowIfDisposed();
        effect_ = &value;
    }

    void TilemapWorldRenderer::Load(const std::vector<Tilemap*>& tilemaps)
    {
        ThrowIfDisposed();

        Unload();

        std::vector<TileAccumulator> accumulators;

        for (Tilemap* tilemap : tilemaps)
        {
            const int depth = tilemap->getWorldDepthProperty();
            const Vector2 worldPos = tilemap->getWorldPositionProperty();

            for (const std::unique_ptr<TilemapLayer>& layerPtr : tilemap->getLayersProperty())
            {
                auto* tileLayer = dynamic_cast<TilemapTileLayer*>(layerPtr.get());
                if (tileLayer == nullptr)
                {
                    continue;
                }

                for (const TilemapTileEntry& entry : tileLayer->GetTiles())
                {
                    TilemapTileset* tileset = tilemap->getTilesetsProperty().GetTilesetForGid(entry.Tile.getGlobalIdProperty());
                    if (tileset == nullptr)
                    {
                        continue;
                    }

                    const int localId = entry.Tile.getGlobalIdProperty() - tileset->getFirstGlobalIdProperty();
                    Texture2D* tileTexture = nullptr;
                    Rectangle sourceRect;
                    tileset->GetRenderSource(localId, tileTexture, sourceRect);

                    if (tileTexture == nullptr)
                    {
                        continue;
                    }

                    TileAccumulator& accumulator =
                        FindOrCreateAccumulator(accumulators, depth, tileTexture, tileLayer->getParallaxFactorProperty());

                    const Point tilePos = tilemap->TileToWorldPosition(entry.X, entry.Y);
                    Vector2 position = Vector2(static_cast<float>(tilePos.X), static_cast<float>(tilePos.Y)) + worldPos
                        + tileLayer->getOffsetProperty() + tileset->getTileOffsetProperty();
                    // Tiled bottom-aligns all tiles: shifts oversized tiles up and undersized tiles down.
                    position.Y += static_cast<float>(tileLayer->getTileHeightProperty() - sourceRect.Height);

                    const Color tileColor(1.0f, 1.0f, 1.0f, tileLayer->getOpacityProperty());
                    AddTileQuad(accumulator.Vertices, accumulator.Indices, position, sourceRect.Width, sourceRect.Height, sourceRect,
                                entry.Tile.getFlipFlagsProperty(), *tileTexture, tileColor);
                }
            }
        }

        for (TileAccumulator& accumulator : accumulators)
        {
            if (accumulator.Vertices.empty())
            {
                continue;
            }

            LayerModel model = CreateLayerModel(graphicsDevice_, accumulator.Vertices, accumulator.Indices, accumulator.Texture);
            model.setParallaxFactorProperty(accumulator.ParallaxFactor);
            worldBatches_.push_back(WorldBatch{std::move(model), accumulator.WorldDepth});
        }

        isLoaded_ = true;
    }

    void TilemapWorldRenderer::Load(const TilemapWorld& world)
    {
        ThrowIfDisposed();

        std::vector<Tilemap*> tilemaps;
        tilemaps.reserve(world.getLevelsProperty().size());
        for (const std::unique_ptr<Tilemap>& level : world.getLevelsProperty())
        {
            tilemaps.push_back(level.get());
        }

        Load(tilemaps);
    }

    void TilemapWorldRenderer::Unload()
    {
        ThrowIfDisposed();

        // Clearing worldBatches_ releases every owned VertexBuffer/IndexBuffer via RAII (see
        // LayerModel.hpp's header comment); no manual Dispose() walk is needed here, unlike upstream.
        worldBatches_.clear();
        isLoaded_ = false;
    }

    void TilemapWorldRenderer::Draw(OrthographicCamera& camera)
    {
        Draw(camera, 0);
    }

    void TilemapWorldRenderer::Draw(OrthographicCamera& camera, int worldDepth)
    {
        ThrowIfDisposed();

        if (!isLoaded_)
        {
            throw System::InvalidOperationException("No world loaded. Call Load first.");
        }

        graphicsDevice_.setBlendStateProperty(blendState_);
        graphicsDevice_.getSamplerStatesProperty()[0] = *samplerState_;

        Matrix worldView = camera.GetViewMatrix();
        // Snap view translation to integer pixels to prevent sub-pixel edge bleeding.
        worldView.M41 = std::round(worldView.M41);
        worldView.M42 = std::round(worldView.M42);
        effect_->View = worldView;
        const Viewport& viewport = graphicsDevice_.getViewportProperty();
        effect_->Projection = Matrix::CreateOrthographicOffCenter(
            0.0f, static_cast<float>(viewport.getWidthProperty()),
            static_cast<float>(viewport.getHeightProperty()), 0.0f,
            0.0f, 1.0f);
        effect_->World = Matrix::getIdentityProperty();

        for (WorldBatch& batch : worldBatches_)
        {
            if (batch.WorldDepth != worldDepth)
            {
                continue;
            }

            if (batch.Model.getParallaxFactorProperty() == Vector2::One)
            {
                effect_->World = Matrix::getIdentityProperty();
            }
            else
            {
                const Vector2 offset = camera.getPositionProperty() * (Vector2::One - batch.Model.getParallaxFactorProperty());
                effect_->World = Matrix::CreateTranslation(offset.X, offset.Y, 0.0f);
            }

            DrawModel(batch.Model);
        }

        effect_->World = Matrix::getIdentityProperty();
    }

    void TilemapWorldRenderer::DrawModel(const LayerModel& model)
    {
        effect_->setTextureProperty(model.getTextureProperty());
        graphicsDevice_.SetVertexBuffer(&model.getVertexBufferProperty());
        graphicsDevice_.SetIndexBuffer(&model.getIndexBufferProperty());

        effect_->Apply();
        graphicsDevice_.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0,
                                               model.getVertexBufferProperty().getVertexCountProperty(), 0,
                                               model.getPrimitiveCountProperty());
    }

    void TilemapWorldRenderer::Dispose()
    {
        if (isDisposed_)
        {
            return;
        }

        Unload();
        isDisposed_ = true;
    }

    void TilemapWorldRenderer::ThrowIfDisposed() const
    {
        System::ObjectDisposedException::ThrowIf(isDisposed_, "TilemapWorldRenderer");
    }
}
