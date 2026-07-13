// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Triangulation/LineSegment.cs. Upstream's own file header
// additionally credits nickgravelyn's Triangulator project
// (https://github.com/nickgravelyn/Triangulator, an ear-clipping triangulation implementation)
// as the origin of this code ("MIT Licensed"). Carried forward here for the same reason
// MonoGame.Extended carries it: this code traces back further than Craftwork Games. See
// NOTICE.md for a provenance note about this specific attribution.
//
// C#'s `float?`/`Vector2?` (Nullable<T>) map to std::optional<float>/std::optional<Vector2>,
// matching this project's established convention (see Line2D.hpp, Matrix3x2.hpp).
#pragma once

#include "CNA/Extended/Triangulation/Vertex.hpp"

#include <optional>

namespace CNA::Extended::Triangulation
{
    /** @brief A line segment between two Triangulation::Vertex endpoints, used during polygon triangulation. */
    struct LineSegment
    {
        /** @brief The first endpoint of this LineSegment. */
        Vertex A;

        /** @brief The second endpoint of this LineSegment. */
        Vertex B;

        LineSegment() = default;

        /** @brief Initializes a new LineSegment from the specified endpoints. */
        LineSegment(const Vertex& a, const Vertex& b);

        /**
         * @brief Computes the distance from @p origin to the point where a ray cast from
         * @p origin in @p direction intersects this LineSegment, or std::nullopt if it does
         * not intersect.
         */
        [[nodiscard]] std::optional<float> IntersectsWithRay(const Vector2& origin, const Vector2& direction) const;

        /**
         * @brief Finds the point where two LineSegment instances intersect, or std::nullopt if
         * they do not intersect within both segments' bounds.
         */
        [[nodiscard]] static std::optional<Vector2> FindIntersection(const LineSegment& a, const LineSegment& b);
    };
}
