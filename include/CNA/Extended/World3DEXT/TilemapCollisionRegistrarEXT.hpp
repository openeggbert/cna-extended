// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::RegisterTilemapCollisionActorsEXT -- new, non-upstream
// addition. See 3d.md/plan3d.md at the repository root for the design (Phase 12 B,
// 2026-07-15, user-requested "extend later" item).
//
// A free function, not a method on Tilemap3DEXT or CollisionWorld3DEXT (both are focused,
// single-purpose types that don't already depend on each other -- Tilemap3DEXT doesn't know
// about collision, CollisionWorld3DEXT doesn't know about tilemaps -- this function is the
// bridge, matching NextUnitVector3EXT's own established free-function-bridging-two-types
// precedent from Phase 11 C-2). Bulk-creates one TilemapCollisionActor3DEXT per populated
// Tilemap3DEXT cell and inserts it into a CollisionWorld3DEXT, so a voxel/block world's
// static geometry doesn't need manual per-tile actor registration (the exact gap this phase
// closes -- previously documented in NEXT.md as "today: insert one ICollisionActor3DEXT per
// populated tile manually").
#pragma once

#include "CNA/Extended/World3DEXT/CollisionWorld3DEXT.hpp"

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    class Tilemap3DEXT;
    class TilemapCollisionActor3DEXT;

    /**
     * @brief Creates and inserts one TilemapCollisionActor3DEXT per populated cell in
     * @p tilemap into @p world.
     * @param world The collision world to insert into.
     * @param tilemap The tilemap whose populated cells become collision actors.
     * @param layerName The layer to insert into; defaults to CollisionWorld3DEXT::DefaultLayerName.
     * @return The created actors, owned by the caller. Must stay alive at least as long as
     * @p world holds references to them (matching every other ICollisionActor3DEXT usage in
     * this project: CollisionWorld3DEXT never takes ownership of inserted actors).
     */
    [[nodiscard]] std::vector<std::unique_ptr<TilemapCollisionActor3DEXT>> RegisterTilemapCollisionActorsEXT(
        CollisionWorld3DEXT& world, const Tilemap3DEXT& tilemap, const std::string& layerName = CollisionWorld3DEXT::DefaultLayerName);
}
