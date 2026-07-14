// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CollisionShapeKind3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::CollisionShapeKind2D, scoped down to the two shape
// kinds plan3d.md's Phase 5 actually calls for (BoundingBox/BoundingSphere) rather than
// upstream CollisionShape2D's full 5-kind set (Box/Circle/OrientedBox/Capsule/Polygon) --
// OrientedBox/Capsule/Polygon 3D counterparts are not requested by plan3d.md and can be
// added later if a real need appears, matching this phase's "start with the simplest
// correct version" scope decision (plan3d.md section 4, Phase 5).
#pragma once

namespace CNA::Extended::World3DEXT
{
    /** @brief Identifies which shape a CollisionShape3DEXT currently represents. */
    enum class CollisionShapeKind3DEXT
    {
        /** @brief No shape; never intersects anything. */
        None,
        /** @brief An axis-aligned bounding box (Microsoft::Xna::Framework::BoundingBox). */
        Box,
        /** @brief A bounding sphere (Microsoft::Xna::Framework::BoundingSphere). */
        Sphere,
    };
}
