// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/PrimitivesHelper.cs (cites "Real-Time Collision
// Detection", Christer Ericson, 2005 -- an algorithm citation, not a third-party code/license
// dependency, matching CircleF.cs/Segment2.cs's earlier citations of the same book). Upstream's
// `internal` visibility has no C++ equivalent; kept as a regular type in this namespace, matching
// the precedent set by TransformFlags in Transform.hpp (documented as an implementation detail,
// not part of the public API contract even though visible). This type was the single most
// referenced blocker across Phase 1 so far -- see plan.md for the sweep of follow-ups landed
// alongside this task.
#pragma once

#include "CNA/Extended/Matrix3x2.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <vector>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    class PrimitivesHelper
    {
    public:
        PrimitivesHelper() = delete;

        /**
         * @brief Tests whether a ray intersects an axis-aligned "slab" (the range between
         * @p slabMinimum and @p slabMaximum along one coordinate axis), narrowing
         * @p rayMinimumDistance/@p rayMaximumDistance to the intersection of the ray's current
         * valid range and the slab. Used by Ray2D and Segment2.
         */
        static bool IntersectsSlab(float positionCoordinate, float directionCoordinate, float slabMinimum,
            float slabMaximum, float& rayMinimumDistance, float& rayMaximumDistance);

        /** @brief Computes the axis-aligned minimum/maximum corners enclosing the given points. */
        static void CreateRectangleFromPoints(const std::vector<Vector2>& points, Vector2& minimum, Vector2& maximum);

        /** @brief Transforms an axis-aligned rectangle (given as center/halfExtents) by a Matrix3x2. */
        static void TransformRectangle(Vector2& center, Vector2& halfExtents, const Matrix3x2& transformMatrix);

        /** @brief Transforms an oriented rectangle's center and orientation by a Matrix3x2. */
        static void TransformOrientedRectangle(Vector2& center, Matrix3x2& orientation, const Matrix3x2& transformMatrix);

        /** @brief Computes the squared distance from a point to the closest point on an axis-aligned rectangle. */
        [[nodiscard]] static float SquaredDistanceToPointFromRectangle(const Vector2& minimum, const Vector2& maximum, const Vector2& point);

        /** @brief Computes the closest point on an axis-aligned rectangle to the given point. */
        static void ClosestPointToPointFromRectangle(const Vector2& minimum, const Vector2& maximum, const Vector2& point, Vector2& result);
    };
}
