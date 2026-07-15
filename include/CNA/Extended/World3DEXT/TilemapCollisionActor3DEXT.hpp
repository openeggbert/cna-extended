// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::TilemapCollisionActor3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design (Phase 12 B, 2026-07-15,
// user-requested "extend later" item: a Collisions3DEXT <-> Tilemaps3DEXT broadphase
// shortcut).
//
// A lightweight ICollisionActor3DEXT wrapping one populated Tilemap3DEXT cell's world-space
// AABB, so a voxel/block world's static geometry doesn't need a full ECS Entity/Component
// per tile just to participate in collision queries. Caches TileXEXT/TileYEXT/TileZEXT/
// TileIdEXT so a caller receiving this actor back from a CollisionWorld3DEXT query (as a
// bare ICollisionActor3DEXT*) can identify which tile it came from via
// dynamic_cast<TilemapCollisionActor3DEXT*> without a separate lookup table.
// @see RegisterTilemapCollisionActorsEXT, which bulk-creates and registers one of these per
// populated tile in a Tilemap3DEXT.
#pragma once

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    class TilemapCollisionActor3DEXT final : public ICollisionActor3DEXT
    {
    public:
        TilemapCollisionActor3DEXT(int id, int tileX, int tileY, int tileZ, int tileId, const CollisionShape3DEXT& shape)
            : id_(id), tileXEXT_(tileX), tileYEXT_(tileY), tileZEXT_(tileZ), tileIdEXT_(tileId), shape_(shape)
        {
        }

        [[nodiscard]] int getIdProperty() const override { return id_; }
        [[nodiscard]] CollisionShape3DEXT getShapeProperty() const override { return shape_; }

        /** @brief Gets the Tilemap3DEXT cell coordinate this actor represents. */
        [[nodiscard]] int getTileXEXTProperty() const { return tileXEXT_; }
        [[nodiscard]] int getTileYEXTProperty() const { return tileYEXT_; }
        [[nodiscard]] int getTileZEXTProperty() const { return tileZEXT_; }

        /** @brief Gets the tile ID this actor's cell held at registration time. */
        [[nodiscard]] int getTileIdEXTProperty() const { return tileIdEXT_; }

    private:
        int id_;
        int tileXEXT_;
        int tileYEXT_;
        int tileZEXT_;
        int tileIdEXT_;
        CollisionShape3DEXT shape_;
    };
}
