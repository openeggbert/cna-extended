// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Ray2.cs -- an OLDER, SEPARATE, minimal 2D ray type,
// distinct from the richer, actively-used Ray2D.cs (already ported as Ray2D.hpp/.cpp). Both
// types genuinely coexist in upstream, in the same MonoGame.Extended namespace, with different
// names (`Ray2` here, `Ray2D` for the other): different field name (`Position` here vs
// `Ray2D::Origin`), different feature surface (this type has exactly one method,
// `Intersects(BoundingRectangle, out, out)`; `Ray2D` has a dozen `Intersects` overloads plus
// GetPoint/ClosestPoint/Normalize/Deconstruct/CreateFromPoints). Confirmed via grep across both
// upstream `source/` and `tests/`: this type has ZERO call sites anywhere in upstream itself
// (only a one-line mention in a PrimitivesHelper.cs comment, "Used by Ray2 and Segment2") and no
// test coverage -- it appears to be legacy/superseded by the Ray2D+Collision2D+Line2D/
// LineSegment2D family, left in place rather than deleted. Ported anyway, at the project owner's
// explicit request, for full fidelity even though it is dead code upstream.
//
// `IEquatableByRef<Ray2>` NOT implemented: matching the established precedent already set by
// every sibling geometric value type in this project (BoundingBox2D, BoundingCircle2D,
// BoundingCapsule2D, BoundingPolygon2D, OrientedBoundingBox2D, Line2D, LineSegment2D, Ray2D) --
// none of them inherit `IEquatableByRef<T>` even where upstream implements it, since C++ value
// types don't need runtime-polymorphic equality dispatch the way upstream's interface constraint
// implies; a plain `Equals(const Ray2&) const` + `operator==` covers the same practical need.
// C#'s `object obj` overload of `Equals` has no C++ equivalent (no universal object base) and is
// not ported, matching `Line2D.hpp`'s identical precedent.
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    class BoundingRectangle;

    /**
     * @brief A two dimensional ray defined by a starting Vector2 and a direction Vector2.
     *
     * Distinct from the richer Ray2D (see this file's own header comment) -- prefer Ray2D for
     * new code; this type is a minimal, legacy upstream type with a single Intersects overload.
     */
    struct Ray2
    {
        /** @brief The starting Vector2 of this Ray2. */
        Vector2 Position;

        /** @brief The direction Vector2 of this Ray2. */
        Vector2 Direction;

        Ray2() = default;

        /**
         * @brief Initializes a new instance of the Ray2 structure from the specified position and direction.
         * @param position The starting point.
         * @param direction The direction vector.
         */
        Ray2(const Vector2& position, const Vector2& direction);

        /**
         * @brief Determines whether this Ray2 intersects with a specified BoundingRectangle.
         * @param boundingRectangle The bounding rectangle.
         * @param rayNearDistance When this method returns, contains the distance along the ray to
         * the first intersection point with @p boundingRectangle, if an intersection was found;
         * otherwise, NaN.
         * @param rayFarDistance When this method returns, contains the distance along the ray to
         * the second intersection point with @p boundingRectangle, if an intersection was found;
         * otherwise, NaN.
         * @return true if this Ray2 intersects with @p boundingRectangle; otherwise, false.
         */
        [[nodiscard]] bool Intersects(const BoundingRectangle& boundingRectangle, float& rayNearDistance, float& rayFarDistance) const;

        [[nodiscard]] bool Equals(const Ray2& ray) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const Ray2& first, const Ray2& second) { return first.Equals(second); }
        friend bool operator!=(const Ray2& first, const Ray2& second) { return !first.Equals(second); }
    };
}
