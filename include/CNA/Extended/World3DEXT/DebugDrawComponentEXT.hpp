// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::DebugDrawComponentEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// A per-entity list of world-space debug line segments, drawn every frame by
// DebugDrawSystemEXT. Adapted from cna-scene::DebugDraw's "lines/boxes for development
// visualization" concept (read as reference material only; see CubeMeshComponentEXT.hpp's
// header comment for why no NOTICE.md entry is needed for cna-scene-informed design) --
// re-implemented as an ECS component (a per-entity line list) rather than cna-scene's own
// immediate-mode global draw-list API, so an entity can carry its own persistent debug
// visualization (e.g. always drawing its own collision bounds) instead of every call site
// re-submitting lines each frame.
//
// AddDebugBoxLinesEXT/AddDebugFrustumLinesEXT cover the two cases plan3d.md's Phase 6
// bullet explicitly calls for ("drawing Collisions3DEXT bounds and Camera3DEXT frustums
// for debugging") -- BoundingBox::GetCorners()/BoundingFrustum::GetCorners() share the
// same real CNA 8-corner ordering (near/front face 0-3, far/back face 4-7), so one shared
// wireframe-from-corners helper covers both.
//
// AddDebugSphereLinesEXT (Phase 12 A, 2026-07-15, user-requested "extend later" item) adds
// sphere wireframes -- deferred in the first pass since CollisionShape3DEXT already exposes
// a BoundingBox for any shape via getBoundingBoxProperty(), so box wireframes alone covered
// every collision shape kind at the time; added now on request. No sphere-wireframe helper
// exists anywhere in cna/easy-3d to reuse, so this draws 3 orthogonal great circles (XY/XZ/YZ
// planes through Center, each approximated by segmentsPerCircle line segments) -- the
// standard sphere-wireframe approximation, not a full lat/long grid (which would need far
// more line segments for a debug-only visualization with no real added clarity).
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <vector>

namespace Microsoft::Xna::Framework
{
    struct BoundingBox;
    class BoundingFrustum;
    struct BoundingSphere;
}

namespace CNA::Extended::World3DEXT
{
    /** @brief One world-space debug line segment. */
    struct DebugLineEXT
    {
        Microsoft::Xna::Framework::Vector3 Start;
        Microsoft::Xna::Framework::Vector3 End;
        Microsoft::Xna::Framework::Color LineColor = Microsoft::Xna::Framework::Color::White;
    };

    /**
     * @brief ECS component holding a list of world-space debug line segments, drawn every
     * frame by DebugDrawSystemEXT.
     * @see DebugDrawSystemEXT, which draws every active entity's LinesEXT each frame.
     */
    struct DebugDrawComponentEXT
    {
        std::vector<DebugLineEXT> LinesEXT;
    };

    /** @brief Appends the 12 wireframe edges of @p box, in @p color, to @p lines. */
    void AddDebugBoxLinesEXT(std::vector<DebugLineEXT>& lines, const Microsoft::Xna::Framework::BoundingBox& box,
                              const Microsoft::Xna::Framework::Color& color);

    /** @brief Appends the 12 wireframe edges of @p frustum, in @p color, to @p lines. */
    void AddDebugFrustumLinesEXT(std::vector<DebugLineEXT>& lines, const Microsoft::Xna::Framework::BoundingFrustum& frustum,
                                  const Microsoft::Xna::Framework::Color& color);

    /** @brief Appends a 3-great-circle wireframe approximation of @p sphere, in @p color, to @p lines. */
    void AddDebugSphereLinesEXT(std::vector<DebugLineEXT>& lines, const Microsoft::Xna::Framework::BoundingSphere& sphere,
                                 const Microsoft::Xna::Framework::Color& color, int segmentsPerCircle = 24);
}
