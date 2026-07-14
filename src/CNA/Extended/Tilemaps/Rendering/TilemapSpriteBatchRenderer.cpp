// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/TilemapSpriteBatchRenderer.cs. See
// TilemapSpriteBatchRenderer.hpp for design notes.
#include "CNA/Extended/Tilemaps/Rendering/TilemapSpriteBatchRenderer.hpp"

#include "CNA/Extended/Tilemaps/TilemapTileObject.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"

#include <algorithm>
#include <cmath>
#include <type_traits>

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Graphics::SpriteEffects;

    namespace
    {
        SpriteEffects CombineEffects(SpriteEffects left, SpriteEffects right)
        {
            using U = std::underlying_type_t<SpriteEffects>;
            return static_cast<SpriteEffects>(static_cast<U>(left) | static_cast<U>(right));
        }
    }

    void TilemapSpriteBatchRenderer::LoadTilemap(Tilemap* tilemap)
    {
        System::ArgumentNullException::ThrowIfNull(tilemap, "tilemap");
        tilemap_ = tilemap;
        BuildAnimatedTilesList();
        tileRenderPadding_ = ComputeTileRenderPadding(*tilemap_);
    }

    void TilemapSpriteBatchRenderer::UnloadTilemap()
    {
        tilemap_ = nullptr;
        animatedTiles_.clear();
        tileRenderPadding_ = TileRenderPadding{};
    }

    void TilemapSpriteBatchRenderer::Draw(SpriteBatch& spriteBatch, OrthographicCamera& camera)
    {
        ThrowIfNoTilemap();

        bool inBatch = false;
        Vector2 currentParallax;

        for (const std::unique_ptr<TilemapLayer>& layerPtr : tilemap_->getLayersProperty())
        {
            TilemapLayer* layer = layerPtr.get();

            if (!layer->getIsVisibleProperty())
            {
                if (inBatch)
                {
                    spriteBatch.End();
                    inBatch = false;
                }

                continue;
            }

            if (auto* imageLayer = dynamic_cast<TilemapImageLayer*>(layer))
            {
                if (inBatch)
                {
                    spriteBatch.End();
                    inBatch = false;
                }

                DrawImageLayerCore(spriteBatch, camera, *imageLayer);
                continue;
            }

            if (auto* objectLayer = dynamic_cast<TilemapObjectLayer*>(layer))
            {
                if (inBatch)
                {
                    spriteBatch.End();
                    inBatch = false;
                }

                DrawObjectLayerCore(spriteBatch, camera, *objectLayer);
                continue;
            }

            auto* tileLayer = dynamic_cast<TilemapTileLayer*>(layer);
            if (tileLayer == nullptr)
            {
                if (inBatch)
                {
                    spriteBatch.End();
                    inBatch = false;
                }

                continue;
            }

            const Vector2 parallax = tileLayer->getParallaxFactorProperty();

            if (!inBatch || currentParallax != parallax)
            {
                if (inBatch)
                {
                    spriteBatch.End();
                }

                currentParallax = parallax;
                BeginLayerBatch(spriteBatch, camera, parallax);
                inBatch = true;
            }

            DrawTileLayerCore(spriteBatch, camera, *tileLayer);
        }

        if (inBatch)
        {
            spriteBatch.End();
        }
    }

    void TilemapSpriteBatchRenderer::DrawLayer(SpriteBatch& spriteBatch, OrthographicCamera& camera, const std::string& layerName)
    {
        ThrowIfNoTilemap();

        TilemapLayer* layer = tilemap_->getLayersProperty()[layerName];
        DrawLayerInternal(spriteBatch, camera, layer);
    }

    void TilemapSpriteBatchRenderer::DrawLayer(SpriteBatch& spriteBatch, OrthographicCamera& camera, int layerIndex)
    {
        ThrowIfNoTilemap();

        if (layerIndex < 0 || layerIndex >= tilemap_->getLayersProperty().getCountProperty())
        {
            throw System::ArgumentOutOfRangeException("layerIndex");
        }

        TilemapLayer* layer = tilemap_->getLayersProperty()[layerIndex];
        DrawLayerInternal(spriteBatch, camera, layer);
    }

    void TilemapSpriteBatchRenderer::DrawLayers(SpriteBatch& spriteBatch, OrthographicCamera& camera, const std::vector<std::string>& layerNames)
    {
        ThrowIfNoTilemap();

        bool inBatch = false;
        Vector2 currentParallax;

        for (const std::string& name : layerNames)
        {
            TilemapLayer* layer = tilemap_->getLayersProperty()[name];

            if (!layer->getIsVisibleProperty())
            {
                if (inBatch)
                {
                    spriteBatch.End();
                    inBatch = false;
                }

                continue;
            }

            if (auto* imageLayer = dynamic_cast<TilemapImageLayer*>(layer))
            {
                if (inBatch)
                {
                    spriteBatch.End();
                    inBatch = false;
                }

                DrawImageLayerCore(spriteBatch, camera, *imageLayer);
                continue;
            }

            if (auto* objectLayer = dynamic_cast<TilemapObjectLayer*>(layer))
            {
                if (inBatch)
                {
                    spriteBatch.End();
                    inBatch = false;
                }

                DrawObjectLayerCore(spriteBatch, camera, *objectLayer);
                continue;
            }

            auto* tileLayer = dynamic_cast<TilemapTileLayer*>(layer);
            if (tileLayer == nullptr)
            {
                if (inBatch)
                {
                    spriteBatch.End();
                    inBatch = false;
                }

                continue;
            }

            const Vector2 parallax = tileLayer->getParallaxFactorProperty();

            if (!inBatch || currentParallax != parallax)
            {
                if (inBatch)
                {
                    spriteBatch.End();
                }

                currentParallax = parallax;
                BeginLayerBatch(spriteBatch, camera, parallax);
                inBatch = true;
            }

            DrawTileLayerCore(spriteBatch, camera, *tileLayer);
        }

        if (inBatch)
        {
            spriteBatch.End();
        }
    }

    void TilemapSpriteBatchRenderer::Update(const GameTime& gameTime)
    {
        if (animatedTiles_.empty())
        {
            return;
        }

        const float dt = static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
        for (TilemapTileData* tile : animatedTiles_)
        {
            tile->getAnimationProperty()->Update(dt);
        }
    }

    void TilemapSpriteBatchRenderer::DrawLayerInternal(SpriteBatch& spriteBatch, OrthographicCamera& camera, TilemapLayer* layer)
    {
        if (!layer->getIsVisibleProperty())
        {
            return;
        }

        if (auto* imageLayer = dynamic_cast<TilemapImageLayer*>(layer))
        {
            DrawImageLayerCore(spriteBatch, camera, *imageLayer);
            return;
        }

        if (auto* objectLayer = dynamic_cast<TilemapObjectLayer*>(layer))
        {
            DrawObjectLayerCore(spriteBatch, camera, *objectLayer);
            return;
        }

        if (auto* tileLayer = dynamic_cast<TilemapTileLayer*>(layer))
        {
            BeginLayerBatch(spriteBatch, camera, tileLayer->getParallaxFactorProperty());
            DrawTileLayerCore(spriteBatch, camera, *tileLayer);
            spriteBatch.End();
        }
    }

    void TilemapSpriteBatchRenderer::DrawImageLayerCore(SpriteBatch& spriteBatch, OrthographicCamera& camera, TilemapImageLayer& imageLayer)
    {
        Texture2D* texture = imageLayer.getTextureProperty();
        if (texture == nullptr)
        {
            return;
        }

        BeginLayerBatch(spriteBatch, camera, imageLayer.getParallaxFactorProperty());

        const int texW = texture->getWidthProperty();
        const int texH = texture->getHeightProperty();
        const Rectangle src(0, 0, texW, texH);
        const Vector2 offset = imageLayer.getOffsetProperty();
        const Vector2 imagePosition = imageLayer.getPositionProperty();

        const Color color = imageLayer.getTintColorProperty().has_value()
            ? imageLayer.getTintColorProperty().value() * imageLayer.getOpacityProperty()
            : Color::White * imageLayer.getOpacityProperty();

        if (!imageLayer.getRepeatXProperty() && !imageLayer.getRepeatYProperty())
        {
            spriteBatch.Draw(*texture, imagePosition + offset, src, color);
        }
        else
        {
            const RectangleF camBounds = camera.getBoundingRectangleProperty();
            const Vector2 parallax = imageLayer.getParallaxFactorProperty();
            const Vector2 parallaxOrigin = tilemap_->getParallaxOriginProperty();

            // Parallax shifts which part of the layer is visible, using the same formula as ComputeVisibleTileRegion.
            const float visLeft = parallaxOrigin.X + (camBounds.X - parallaxOrigin.X) * parallax.X - offset.X;
            const float visTop = parallaxOrigin.Y + (camBounds.Y - parallaxOrigin.Y) * parallax.Y - offset.Y;

            const float startX = imageLayer.getRepeatXProperty()
                ? std::floor((visLeft - imagePosition.X) / static_cast<float>(texW)) * static_cast<float>(texW) + imagePosition.X
                : imagePosition.X;
            const float startY = imageLayer.getRepeatYProperty()
                ? std::floor((visTop - imagePosition.Y) / static_cast<float>(texH)) * static_cast<float>(texH) + imagePosition.Y
                : imagePosition.Y;

            // endX/Y are inclusive stopping bounds: one tile width/height past the visible edge.
            // For the non-repeat axis, +1 guarantees exactly one iteration.
            const float endX = imageLayer.getRepeatXProperty() ? visLeft + camBounds.Width + static_cast<float>(texW) : startX + 1.0f;
            const float endY = imageLayer.getRepeatYProperty() ? visTop + camBounds.Height + static_cast<float>(texH) : startY + 1.0f;

            for (float y = startY; y < endY; y += static_cast<float>(texH))
            {
                for (float x = startX; x < endX; x += static_cast<float>(texW))
                {
                    spriteBatch.Draw(*texture, Vector2(x, y) + offset, src, color);
                }
            }
        }

        spriteBatch.End();
    }

    void TilemapSpriteBatchRenderer::DrawObjectLayerCore(SpriteBatch& spriteBatch, OrthographicCamera& camera, TilemapObjectLayer& objectLayer)
    {
        BeginLayerBatch(spriteBatch, camera, objectLayer.getParallaxFactorProperty());

        const Color layerColor = objectLayer.getTintColorProperty().has_value()
            ? objectLayer.getTintColorProperty().value() * objectLayer.getOpacityProperty()
            : Color::White * objectLayer.getOpacityProperty();

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

            const Vector2 objSize = tileObj->getSizeProperty();
            const Vector2 scale(objSize.X / static_cast<float>(sourceRect.Width), objSize.Y / static_cast<float>(sourceRect.Height));

            SpriteEffects effects = SpriteEffects::None;
            const TilemapTileFlipFlags flip = tileObj->getTileProperty().getFlipFlagsProperty();
            if ((flip & TilemapTileFlipFlags::FlipHorizontally) != TilemapTileFlipFlags::None)
            {
                effects = CombineEffects(effects, SpriteEffects::FlipHorizontally);
            }

            if ((flip & TilemapTileFlipFlags::FlipVertically) != TilemapTileFlipFlags::None)
            {
                effects = CombineEffects(effects, SpriteEffects::FlipVertically);
            }

            // Tiled positions tile objects at their bottom-left corner.
            // An origin of (0, sourceRect.Height) places that pivot at tileObj.Position in world space,
            // so the tile draws upward from there as expected.
            // Round to integer world pixels so sub-pixel object positions don't cause the linear
            // filter to blend anti-aliased edge texels with their transparent neighbors, which
            // would produce dark fringe artifacts where objects overlap.
            const Vector2 objPosition = tileObj->getPositionProperty();
            const Vector2 drawPos(std::round(objPosition.X), std::round(objPosition.Y));
            spriteBatch.Draw(
                *texture,
                drawPos,
                sourceRect,
                layerColor,
                tileObj->getRotationProperty(),
                Vector2(0.0f, static_cast<float>(sourceRect.Height)),
                scale,
                effects,
                0.0f);
        }

        spriteBatch.End();
    }

    void TilemapSpriteBatchRenderer::BeginLayerBatch(SpriteBatch& spriteBatch, OrthographicCamera& camera, const Vector2& parallax)
    {
        Matrix view = camera.GetViewMatrix();

        if (parallax != Vector2::One)
        {
            // Correct parallax: scroll relative to the viewport top-left (camBounds), not camera.Position.
            // The parallax offset in world space is (1 - F) * (TL - parallaxOrigin), left-multiplied
            // into the view matrix so it acts as a pre-transform in world space.
            const RectangleF camBounds = camera.getBoundingRectangleProperty();
            const Vector2 tl(camBounds.X, camBounds.Y);
            const Vector2 parallaxOrigin = tilemap_->getParallaxOriginProperty();
            const Vector2 offset = (Vector2::One - parallax) * (tl - parallaxOrigin);
            view = Matrix::CreateTranslation(offset.X, offset.Y, 0.0f) * view;
        }

        // Snap the view translation to integer pixels to prevent sub-pixel sampling artifacts.
        // When the camera is at a fractional position, linear filtering blends edge texels with
        // the transparent background, producing dark fringe artifacts that flicker as the camera moves.
        view.M41 = std::round(view.M41);
        view.M42 = std::round(view.M42);

        // SpriteBatch::Begin takes a non-const SamplerState*, but the PointClamp/LinearClamp/...
        // presets this renderer defaults samplerState_ to are `static const`; this renderer only
        // ever reads from whatever SamplerState it's given, never mutates it, so the const_cast is
        // safe (cna's own SpriteBatch::Begin never mutates the SamplerState it's passed either).
        spriteBatch.Begin(spriteSortMode_, blendState_, const_cast<SamplerState*>(samplerState_), nullptr, nullptr, effect_, view);
    }

    void TilemapSpriteBatchRenderer::DrawTileLayerCore(SpriteBatch& spriteBatch, OrthographicCamera& camera, TilemapTileLayer& tileLayer)
    {
        const Rectangle visibleRegion = ComputeVisibleTileRegion(camera, tileLayer);

        if (visibleRegion.Width <= 0 || visibleRegion.Height <= 0)
        {
            return;
        }

        const Color layerColor = ComputeLayerColor(tileLayer);

        for (const TilemapTileEntry& entry : tileLayer.GetTilesInRegion(visibleRegion))
        {
            TilemapTileset* tileset = nullptr;
            const int localId = entry.Tile.GetLocalId(tilemap_->getTilesetsProperty(), tileset);

            if (tileset == nullptr)
            {
                continue;
            }

            Texture2D* tileTexture = nullptr;
            Rectangle sourceRect;
            tileset->GetRenderSource(localId, tileTexture, sourceRect);

            if (tileTexture == nullptr)
            {
                continue;
            }

            const Point worldPos = tilemap_->TileToWorldPosition(entry.X, entry.Y);
            Vector2 drawPosition = Vector2(static_cast<float>(worldPos.X), static_cast<float>(worldPos.Y)) + tileLayer.getOffsetProperty() + tileset->getTileOffsetProperty();

            // Tiled bottom-aligns all tiles: shifts oversized tiles up and undersized tiles down.
            drawPosition.Y += static_cast<float>(tileLayer.getTileHeightProperty() - sourceRect.Height);

            DrawTile(spriteBatch, *tileTexture, drawPosition, sourceRect,
                     entry.Tile.getFlipFlagsProperty(), sourceRect.Width, sourceRect.Height, layerColor);
        }
    }

    Rectangle TilemapSpriteBatchRenderer::ComputeVisibleTileRegion(OrthographicCamera& camera, TilemapTileLayer& tileLayer) const
    {
        const RectangleF camBounds = camera.getBoundingRectangleProperty();
        const Vector2 parallax = tileLayer.getParallaxFactorProperty();
        const Vector2 parallaxOrigin = tilemap_->getParallaxOriginProperty();
        const Vector2 layerOffset = tileLayer.getOffsetProperty();

        // The visible region's top-left in world space for a parallax layer is the point that
        // maps to the screen top-left after the parallax transform. With origin O and factor F,
        // the effective world left is O + F * (TL - O), where TL is the viewport top-left.
        const float worldLeft = parallaxOrigin.X + (camBounds.X - parallaxOrigin.X) * parallax.X - layerOffset.X;
        const float worldTop = parallaxOrigin.Y + (camBounds.Y - parallaxOrigin.Y) * parallax.Y - layerOffset.Y;

        const float tileWidth = static_cast<float>(tileLayer.getTileWidthProperty());
        const float tileHeight = static_cast<float>(tileLayer.getTileHeightProperty());

        // Oversized tiles can render outside their owning cell, especially image-collection
        // tiles and tilesets with tile offsets. Expand the queried tile region by the maximum
        // render overhang so partially visible tiles are not culled too early at the viewport edge.
        // https://github.com/MonoGame-Extended/Monogame-Extended/issues/1139
        const int startX = std::max(0, static_cast<int>(std::floor((worldLeft - tileRenderPadding_.Right) / tileWidth)));
        const int startY = std::max(0, static_cast<int>(std::floor((worldTop - tileRenderPadding_.Bottom) / tileHeight)));
        const int endX = std::min(tileLayer.getWidthProperty(), static_cast<int>(std::ceil((worldLeft + camBounds.Width + tileRenderPadding_.Left) / tileWidth)));
        const int endY = std::min(tileLayer.getHeightProperty(), static_cast<int>(std::ceil((worldTop + camBounds.Height + tileRenderPadding_.Top) / tileHeight)));

        return Rectangle(startX, startY, endX - startX, endY - startY);
    }

    // https://github.com/MonoGame-Extended/Monogame-Extended/issues/1139
    TilemapSpriteBatchRenderer::TileRenderPadding TilemapSpriteBatchRenderer::ComputeTileRenderPadding(const Tilemap& tilemap)
    {
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;

        for (const std::unique_ptr<TilemapTileset>& tilesetPtr : tilemap.getTilesetsProperty())
        {
            const TilemapTileset& tileset = *tilesetPtr;
            const Vector2 tileOffset = tileset.getTileOffsetProperty();
            const float tileLeft = tileOffset.X;
            const float tileTop = tileOffset.Y + static_cast<float>(tilemap.getTileHeightProperty()) - static_cast<float>(tileset.getTileHeightProperty());
            const float tileRight = tileOffset.X + static_cast<float>(tileset.getTileWidthProperty());
            const float tileBottom = tileOffset.Y + static_cast<float>(tilemap.getTileHeightProperty());

            left = std::max(left, std::max(0.0f, -tileLeft));
            top = std::max(top, std::max(0.0f, -tileTop));
            right = std::max(right, std::max(0.0f, tileRight - static_cast<float>(tilemap.getTileWidthProperty())));
            bottom = std::max(bottom, std::max(0.0f, tileBottom - static_cast<float>(tilemap.getTileHeightProperty())));
        }

        return TileRenderPadding{left, top, right, bottom};
    }

    Color TilemapSpriteBatchRenderer::ComputeLayerColor(const TilemapLayer& layer)
    {
        if (layer.getTintColorProperty().has_value())
        {
            return layer.getTintColorProperty().value() * layer.getOpacityProperty();
        }

        return Color::White * layer.getOpacityProperty();
    }

    void TilemapSpriteBatchRenderer::DrawTile(SpriteBatch& spriteBatch, Texture2D& texture, const Vector2& position, const Rectangle& sourceRect,
                                               TilemapTileFlipFlags flipFlags, int tileWidth, int tileHeight, const Color& color)
    {
        const bool flipH = (flipFlags & TilemapTileFlipFlags::FlipHorizontally) != TilemapTileFlipFlags::None;
        const bool flipV = (flipFlags & TilemapTileFlipFlags::FlipVertically) != TilemapTileFlipFlags::None;
        const bool flipD = (flipFlags & TilemapTileFlipFlags::FlipDiagonally) != TilemapTileFlipFlags::None;

        // Scale maps source rect dimensions to the tile's destination size.
        // For standard tilemaps this is (1,1) but may differ when the tileset tile size
        // does not match the layer tile size.
        const Vector2 scale(static_cast<float>(tileWidth) / static_cast<float>(sourceRect.Width),
                             static_cast<float>(tileHeight) / static_cast<float>(sourceRect.Height));

        if (!flipD)
        {
            SpriteEffects effects = SpriteEffects::None;
            if (flipH)
            {
                effects = CombineEffects(effects, SpriteEffects::FlipHorizontally);
            }

            if (flipV)
            {
                effects = CombineEffects(effects, SpriteEffects::FlipVertically);
            }

            spriteBatch.Draw(texture, position, sourceRect, color, 0.0f, Vector2::Zero, scale, effects, 0.0f);
        }
        else
        {
            // Diagonal flip encodes 90-degree rotations per Tiled's convention.
            // The four diagonal combinations map to specific rotation + flip pairs:
            //   H=0 V=1 D=1  -> rotate 90 degrees counterclockwise
            //   H=1 V=0 D=1  -> rotate 90 degrees clockwise
            //   H=1 V=1 D=1  -> rotate 90 degrees clockwise, flip horizontal
            //   H=0 V=0 D=1  -> rotate 90 degrees counterclockwise, flip horizontal
            float rotation;
            SpriteEffects effects;

            if (!flipH && flipV)
            {
                rotation = -MathHelper::PiOver2;
                effects = SpriteEffects::None;
            }
            else if (flipH && !flipV)
            {
                rotation = MathHelper::PiOver2;
                effects = SpriteEffects::None;
            }
            else if (flipH && flipV)
            {
                rotation = MathHelper::PiOver2;
                effects = SpriteEffects::FlipHorizontally;
            }
            else
            {
                rotation = -MathHelper::PiOver2;
                effects = SpriteEffects::FlipHorizontally;
            }

            // Rotation pivots around the tile's center in source space. Position is moved
            // to the world-space tile center so the tile renders in its correct grid cell.
            const Vector2 origin(static_cast<float>(sourceRect.Width) * 0.5f, static_cast<float>(sourceRect.Height) * 0.5f);
            const Vector2 center = position + Vector2(static_cast<float>(tileWidth) * 0.5f, static_cast<float>(tileHeight) * 0.5f);

            spriteBatch.Draw(texture, center, sourceRect, color, rotation, origin, scale, effects, 0.0f);
        }
    }

    void TilemapSpriteBatchRenderer::BuildAnimatedTilesList()
    {
        animatedTiles_.clear();
        for (const std::unique_ptr<TilemapTileset>& tilesetPtr : tilemap_->getTilesetsProperty())
        {
            const std::vector<TilemapTileData*>& animated = tilesetPtr->GetAnimatedTiles();
            for (TilemapTileData* tile : animated)
            {
                animatedTiles_.push_back(tile);
            }
        }
    }

    void TilemapSpriteBatchRenderer::ThrowIfNoTilemap() const
    {
        if (tilemap_ == nullptr)
        {
            throw System::InvalidOperationException("No tilemap loaded. Call LoadTilemap first.");
        }
    }
}
