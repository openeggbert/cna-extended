// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/BoundingRectangle.cs. Deferred members:
//   - `BoundingRectangle(Vector2 center, SizeF halfExtents)` constructor: needs `SizeF` (Phase 1,
//     "Size, SizeF, Interval, Thickness") -- the field itself is Vector2, but upstream's
//     constructor takes SizeF and relies on an implicit SizeF->Vector2 conversion.
//   - The two `Transform(...)` overloads: need `Matrix3x2` + `PrimitivesHelper.TransformRectangle`.
//   - `CreateFrom(IReadOnlyList<Vector2> points, ...)` (both overloads) and `UpdateFromPoints`:
//     need `PrimitivesHelper.CreateRectangleFromPoints`.
//   - `SquaredDistanceTo`/`ClosestPointTo`: need `PrimitivesHelper`.
//   - `implicit operator BoundingRectangle(Rectangle)` and
//     `implicit operator BoundingRectangle(RectangleF)`: both construct a local SizeF upstream
//     before delegating to the deferred SizeF-taking constructor.
// NOT deferred, despite going through the same conversion family: `implicit operator
// Rectangle(BoundingRectangle)` and `implicit operator RectangleF(BoundingRectangle)` -- both are
// pure Vector2/float arithmetic in upstream with no SizeF involved, so they're fully portable now.
#pragma once

#include "CNA/Extended/RectangleF.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;

    /**
     * @brief An axis-aligned, four sided, two dimensional box defined by a centre Vector2 and a
     * radii Vector2.
     */
    struct BoundingRectangle
    {
        /** @brief The BoundingRectangle with Center and HalfExtents both set to Vector2::Zero. */
        static const BoundingRectangle Empty;

        /** @brief The centre position of this BoundingRectangle. */
        Vector2 Center;

        /**
         * @brief The distance from the Center point along both axes to any point on the
         * boundary of this BoundingRectangle.
         */
        Vector2 HalfExtents;

        // Deliberately no user-declared constructor: keeps this an aggregate, so
        // `BoundingRectangle()` (default) and the C++20 parenthesized-aggregate-init form
        // `BoundingRectangle(center, halfExtents)` both work, without adding a constructor
        // upstream doesn't have (its only constructor takes a deferred SizeF parameter -- see
        // the header comment above).

        /**
         * @brief Computes the BoundingRectangle from a minimum Vector2 and maximum Vector2.
         * @param result Receives the resulting bounding rectangle.
         */
        static void CreateFrom(const Vector2& minimum, const Vector2& maximum, BoundingRectangle& result);

        /** @brief Computes the BoundingRectangle from a minimum Vector2 and maximum Vector2. */
        [[nodiscard]] static BoundingRectangle CreateFrom(const Vector2& minimum, const Vector2& maximum);

        /**
         * @brief Computes the BoundingRectangle that contains the two specified
         * BoundingRectangle structures.
         * @param result Receives the resulting bounding rectangle.
         */
        static void Union(const BoundingRectangle& first, const BoundingRectangle& second, BoundingRectangle& result);

        /**
         * @brief Computes the BoundingRectangle that contains the two specified
         * BoundingRectangle structures.
         */
        [[nodiscard]] static BoundingRectangle Union(const BoundingRectangle& first, const BoundingRectangle& second);

        /** @brief Computes the BoundingRectangle that contains both @p boundingRectangle and this one. */
        [[nodiscard]] BoundingRectangle Union(const BoundingRectangle& boundingRectangle) const;

        /**
         * @brief Computes the BoundingRectangle that is in common between the two specified
         * BoundingRectangle structures.
         * @param result Receives the intersection, or BoundingRectangle::Empty if there is none.
         */
        static void Intersection(const BoundingRectangle& first, const BoundingRectangle& second, BoundingRectangle& result);

        /**
         * @brief Computes the BoundingRectangle that is in common between the two specified
         * BoundingRectangle structures, or BoundingRectangle::Empty if there is none.
         */
        [[nodiscard]] static BoundingRectangle Intersection(const BoundingRectangle& first, const BoundingRectangle& second);

        /**
         * @brief Computes the BoundingRectangle that is in common between @p boundingRectangle
         * and this one, or BoundingRectangle::Empty if there is none.
         */
        [[nodiscard]] BoundingRectangle Intersection(const BoundingRectangle& boundingRectangle) const;

        /** @brief Determines whether the two specified BoundingRectangle structures intersect. */
        [[nodiscard]] static bool Intersects(const BoundingRectangle& first, const BoundingRectangle& second);

        /** @brief Determines whether the specified BoundingRectangle intersects with this one. */
        [[nodiscard]] bool Intersects(const BoundingRectangle& boundingRectangle) const;

        /** @brief Determines whether the specified BoundingRectangle contains the specified Vector2. */
        [[nodiscard]] static bool Contains(const BoundingRectangle& boundingRectangle, const Vector2& point);

        /** @brief Determines whether this BoundingRectangle contains the specified Vector2. */
        [[nodiscard]] bool Contains(const Vector2& point) const;

        [[nodiscard]] bool Equals(const BoundingRectangle& boundingRectangle) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const BoundingRectangle& first, const BoundingRectangle& second) { return first.Equals(second); }
        friend bool operator!=(const BoundingRectangle& first, const BoundingRectangle& second) { return !first.Equals(second); }

        /** @brief Implicitly converts this BoundingRectangle to a Rectangle. */
        [[nodiscard]] operator Rectangle() const; // NOLINT(*-explicit-constructor)

        /** @brief Implicitly converts this BoundingRectangle to a RectangleF. */
        [[nodiscard]] operator RectangleF() const; // NOLINT(*-explicit-constructor)
    };
}
