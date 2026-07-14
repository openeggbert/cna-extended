// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/TilemapWorldSpriteBatchRenderer.cs. See
// TilemapWorldSpriteBatchRenderer.hpp for design notes.
//
// *** GENUINE UPSTREAM INCONSISTENCY, PRESERVED FOR FIDELITY -- see this file's DrawTile ***
// This class's own private DrawTile has the OPPOSITE rotation signs from
// TilemapSpriteBatchRenderer::DrawTile for the "H xor V" (exactly one of flipH/flipV set) cases:
// there, !flipH&&flipV -> -PiOver2 and flipH&&!flipV -> +PiOver2; here it's the reverse. Confirmed
// directly against both upstream .cs files side by side -- this is a real difference between the
// two renderers in MonoGame.Extended itself, not a typo introduced during porting, and is
// reproduced exactly rather than "fixed"/normalized between the two ported classes.
#include "CNA/Extended/Tilemaps/Rendering/TilemapWorldSpriteBatchRenderer.hpp"

#include "CNA/Extended/Tilemaps/TilemapTileLayer.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/InvalidOperationException.hpp"

#include <cmath>
#include <type_traits>
#include <unordered_set>
#include <utility>

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

    /** @brief One baked, culled-in-isolation tile draw within a room, resolved into world space at Load time. */
    struct WorldTileSprite
    {
        Texture2D* Texture = nullptr;
        Rectangle SourceRect;
        TilemapTileData* AnimatedTileData = nullptr;
        TilemapTileset* Tileset = nullptr;
        Vector2 WorldPosition;
        TilemapTileFlipFlags FlipFlags = TilemapTileFlipFlags::None;
        int TileWidth = 0;
        int TileHeight = 0;
        Color TileColor = Color::White;
    };

    /** @brief All baked tiles belonging to one (tilemap, depth, parallax) combination, plus that tilemap's world-space bounds for room-level culling. */
    struct WorldRoomBatch
    {
        RectangleF WorldBounds;
        std::vector<WorldTileSprite> Tiles;
    };

    /** @brief All room batches sharing one (WorldDepth, ParallaxFactor) combination across every loaded tilemap. */
    struct TilemapWorldSpriteBatchRenderer::WorldGroup
    {
        int WorldDepth = 0;
        Vector2 ParallaxFactor;
        std::vector<std::unique_ptr<WorldRoomBatch>> Rooms;
    };

    TilemapWorldSpriteBatchRenderer::TilemapWorldSpriteBatchRenderer() = default;
    TilemapWorldSpriteBatchRenderer::~TilemapWorldSpriteBatchRenderer() = default;
    TilemapWorldSpriteBatchRenderer::TilemapWorldSpriteBatchRenderer(TilemapWorldSpriteBatchRenderer&&) noexcept = default;
    TilemapWorldSpriteBatchRenderer& TilemapWorldSpriteBatchRenderer::operator=(TilemapWorldSpriteBatchRenderer&&) noexcept = default;

    void TilemapWorldSpriteBatchRenderer::Load(const std::vector<Tilemap*>& tilemaps)
    {
        Unload();

        struct GroupKey
        {
            int WorldDepth;
            Vector2 ParallaxFactor;
        };
        struct RoomKey
        {
            int WorldDepth;
            Vector2 ParallaxFactor;
            const Tilemap* TilemapPtr;
        };

        std::vector<std::pair<GroupKey, WorldGroup*>> groupIndex;
        std::vector<std::pair<RoomKey, WorldRoomBatch*>> roomIndex;
        std::unordered_set<const TilemapTileData*> animatedSet;

        for (Tilemap* tilemap : tilemaps)
        {
            const int depth = tilemap->getWorldDepthProperty();
            const Vector2 worldPos = tilemap->getWorldPositionProperty();
            const RectangleF roomBounds(worldPos.X, worldPos.Y,
                                         static_cast<float>(tilemap->getWidthProperty() * tilemap->getTileWidthProperty()),
                                         static_cast<float>(tilemap->getHeightProperty() * tilemap->getTileHeightProperty()));

            for (const std::unique_ptr<TilemapLayer>& layerPtr : tilemap->getLayersProperty())
            {
                auto* tileLayer = dynamic_cast<TilemapTileLayer*>(layerPtr.get());
                if (tileLayer == nullptr || !tileLayer->getIsVisibleProperty())
                {
                    continue;
                }

                const Vector2 parallax = tileLayer->getParallaxFactorProperty();

                WorldGroup* group = nullptr;
                for (auto& [key, candidate] : groupIndex)
                {
                    if (key.WorldDepth == depth && key.ParallaxFactor == parallax)
                    {
                        group = candidate;
                        break;
                    }
                }

                if (group == nullptr)
                {
                    auto newGroup = std::make_unique<WorldGroup>();
                    newGroup->WorldDepth = depth;
                    newGroup->ParallaxFactor = parallax;
                    group = newGroup.get();
                    groupIndex.emplace_back(GroupKey{depth, parallax}, group);
                    worldGroups_.push_back(std::move(newGroup));
                }

                WorldRoomBatch* roomBatch = nullptr;
                for (auto& [key, candidate] : roomIndex)
                {
                    if (key.WorldDepth == depth && key.ParallaxFactor == parallax && key.TilemapPtr == tilemap)
                    {
                        roomBatch = candidate;
                        break;
                    }
                }

                if (roomBatch == nullptr)
                {
                    auto newRoom = std::make_unique<WorldRoomBatch>();
                    newRoom->WorldBounds = roomBounds;
                    roomBatch = newRoom.get();
                    roomIndex.emplace_back(RoomKey{depth, parallax, tilemap}, roomBatch);
                    group->Rooms.push_back(std::move(newRoom));
                }

                const Color layerColor = tileLayer->getTintColorProperty().has_value()
                    ? tileLayer->getTintColorProperty().value() * tileLayer->getOpacityProperty()
                    : Color::White * tileLayer->getOpacityProperty();

                for (const TilemapTileEntry& entry : tileLayer->GetTiles())
                {
                    TilemapTileset* tileset = nullptr;
                    const int localId = entry.Tile.GetLocalId(tilemap->getTilesetsProperty(), tileset);
                    if (tileset == nullptr)
                    {
                        continue;
                    }

                    TilemapTileData* tileData = tileset->GetTileData(localId);
                    TilemapTileData* animatedData = nullptr;

                    if (tileData != nullptr && tileData->getAnimationProperty() != nullptr)
                    {
                        animatedData = tileData;
                        if (animatedSet.insert(tileData).second)
                        {
                            animatedTiles_.push_back(tileData);
                        }
                    }

                    Texture2D* tileTexture = nullptr;
                    Rectangle sourceRect;
                    tileset->GetRenderSource(localId, tileTexture, sourceRect);

                    if (tileTexture == nullptr)
                    {
                        continue;
                    }

                    const Point tilePos = tilemap->TileToWorldPosition(entry.X, entry.Y);
                    Vector2 drawPosition = Vector2(static_cast<float>(tilePos.X), static_cast<float>(tilePos.Y)) + worldPos + tileLayer->getOffsetProperty() + tileset->getTileOffsetProperty();
                    // Tiled bottom-aligns all tiles: shifts oversized tiles up and undersized tiles down.
                    drawPosition.Y += static_cast<float>(tileLayer->getTileHeightProperty() - sourceRect.Height);

                    WorldTileSprite sprite;
                    sprite.Texture = tileTexture;
                    sprite.SourceRect = sourceRect;
                    sprite.AnimatedTileData = animatedData;
                    sprite.Tileset = tileset;
                    sprite.WorldPosition = drawPosition;
                    sprite.FlipFlags = entry.Tile.getFlipFlagsProperty();
                    sprite.TileWidth = sourceRect.Width;
                    sprite.TileHeight = sourceRect.Height;
                    sprite.TileColor = layerColor;
                    roomBatch->Tiles.push_back(sprite);
                }
            }
        }

        isLoaded_ = true;
    }

    void TilemapWorldSpriteBatchRenderer::Load(const TilemapWorld& world)
    {
        std::vector<Tilemap*> tilemaps;
        tilemaps.reserve(world.getLevelsProperty().size());
        for (const std::unique_ptr<Tilemap>& level : world.getLevelsProperty())
        {
            tilemaps.push_back(level.get());
        }
        Load(tilemaps);
    }

    void TilemapWorldSpriteBatchRenderer::Unload()
    {
        worldGroups_.clear();
        animatedTiles_.clear();
        isLoaded_ = false;
    }

    void TilemapWorldSpriteBatchRenderer::Update(const GameTime& gameTime)
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

    void TilemapWorldSpriteBatchRenderer::Draw(SpriteBatch& spriteBatch, OrthographicCamera& camera, int worldDepth)
    {
        if (!isLoaded_)
        {
            throw System::InvalidOperationException("No world loaded. Call Load first.");
        }

        const RectangleF camBounds = camera.getBoundingRectangleProperty();

        Matrix view = camera.GetViewMatrix();
        view.M41 = std::round(view.M41);
        view.M42 = std::round(view.M42);

        int visibleRooms = 0;

        for (const std::unique_ptr<WorldGroup>& groupPtr : worldGroups_)
        {
            WorldGroup& group = *groupPtr;
            if (group.WorldDepth != worldDepth)
            {
                continue;
            }

            const Vector2 parallax = group.ParallaxFactor;

            // Compute the effective visible world region for this parallax factor.
            // With parallax origin at (0,0): worldLeft = camBounds.X * parallax.X
            const float cullLeft = camBounds.X * parallax.X;
            const float cullTop = camBounds.Y * parallax.Y;
            const float cullRight = cullLeft + camBounds.Width;
            const float cullBottom = cullTop + camBounds.Height;

            Matrix batchView = view;
            if (parallax != Vector2::One)
            {
                const Vector2 tl(camBounds.X, camBounds.Y);
                const Vector2 offset = (Vector2::One - parallax) * tl;
                batchView = Matrix::CreateTranslation(offset.X, offset.Y, 0.0f) * view;
            }

            // SpriteBatch::Begin takes a non-const SamplerState*, but the PointClamp/LinearClamp/...
            // presets this renderer defaults samplerState_ to are `static const`; this renderer
            // only ever reads from whatever SamplerState it's given, never mutates it, so the
            // const_cast is safe (see TilemapSpriteBatchRenderer.cpp's identical comment).
            spriteBatch.Begin(spriteSortMode_, blendState_, const_cast<SamplerState*>(samplerState_), nullptr, nullptr, effect_, batchView);

            for (const std::unique_ptr<WorldRoomBatch>& roomPtr : group.Rooms)
            {
                const WorldRoomBatch& room = *roomPtr;
                const RectangleF& b = room.WorldBounds;
                if (b.getRightProperty() <= cullLeft || b.getLeftProperty() >= cullRight ||
                    b.getBottomProperty() <= cullTop || b.getTopProperty() >= cullBottom)
                {
                    continue;
                }

                ++visibleRooms;

                for (const WorldTileSprite& tile : room.Tiles)
                {
                    const Vector2 pos = tile.WorldPosition;
                    if (pos.X + static_cast<float>(tile.TileWidth) <= cullLeft || pos.X >= cullRight ||
                        pos.Y + static_cast<float>(tile.TileHeight) <= cullTop || pos.Y >= cullBottom)
                    {
                        continue;
                    }

                    Texture2D* drawTexture;
                    Rectangle src;

                    if (tile.AnimatedTileData != nullptr && tile.AnimatedTileData->getAnimationProperty() != nullptr)
                    {
                        tile.Tileset->GetRenderSource(tile.AnimatedTileData->getLocalIdProperty(), drawTexture, src);

                        if (drawTexture == nullptr)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        drawTexture = tile.Texture;
                        src = tile.SourceRect;
                    }

                    DrawTile(spriteBatch, *drawTexture, pos, src, tile.FlipFlags, src.Width, src.Height, tile.TileColor);
                }
            }

            spriteBatch.End();
        }

        lastVisibleRoomCount_ = visibleRooms;
    }

    void TilemapWorldSpriteBatchRenderer::DrawTile(SpriteBatch& spriteBatch, Texture2D& texture, const Vector2& position, const Rectangle& sourceRect,
                                                     TilemapTileFlipFlags flipFlags, int tileWidth, int tileHeight, const Color& color)
    {
        const bool flipH = (flipFlags & TilemapTileFlipFlags::FlipHorizontally) != TilemapTileFlipFlags::None;
        const bool flipV = (flipFlags & TilemapTileFlipFlags::FlipVertically) != TilemapTileFlipFlags::None;
        const bool flipD = (flipFlags & TilemapTileFlipFlags::FlipDiagonally) != TilemapTileFlipFlags::None;

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
            // Diagonal flip encodes 90-degree rotations per Tiled's convention. NOTE: these
            // rotation signs are the OPPOSITE of TilemapSpriteBatchRenderer::DrawTile's for the
            // H-xor-V cases -- a genuine upstream inconsistency, preserved exactly; see this
            // file's header comment.
            float rotation;
            SpriteEffects effects;

            if (!flipH && flipV)
            {
                rotation = MathHelper::PiOver2;
                effects = SpriteEffects::None;
            }
            else if (flipH && !flipV)
            {
                rotation = -MathHelper::PiOver2;
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

            const Vector2 origin(static_cast<float>(sourceRect.Width) * 0.5f, static_cast<float>(sourceRect.Height) * 0.5f);
            const Vector2 center = position + Vector2(static_cast<float>(tileWidth) * 0.5f, static_cast<float>(tileHeight) * 0.5f);
            spriteBatch.Draw(texture, center, sourceRect, color, rotation, origin, scale, effects, 0.0f);
        }
    }
}
