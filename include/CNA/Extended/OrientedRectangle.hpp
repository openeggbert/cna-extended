// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/OrientedRectangle.cs (cites "Real-Time Collision
// Detection", Christer Ericson, 2005, Ch. 4.4 -- an algorithm citation, not a third-party
// code/license dependency; see plan.md). This type was previously deferred in full (task 17,
// "RectangleF family") because its Orientation field is Matrix3x2 itself; Matrix3x2 landed in
// task 20, unblocking most of this type. All members are now ported: `BoundingRectangle`
// property, `static Transform(OrientedRectangle, ref Matrix3x2)`, and
// `explicit operator RectangleF(OrientedRectangle)` landed once `RectangleF::Transform` (via
// `PrimitivesHelper.TransformRectangle`) was ported (task 22 -- verified genuinely unblocked,
// not assumed, before landing these). Upstream's `private static Transform(ref OrientedRectangle,
// ref Matrix3x2, out OrientedRectangle)` overload is not part of the public API contract; its
// logic is inlined directly into the public static `Transform` below, matching how other
// upstream-private implementation details have been handled elsewhere in this port.
#pragma once

#include "CNA/Extended/Matrix3x2.hpp"
#include "CNA/Extended/RectangleF.hpp"
#include "CNA/Extended/SizeF.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    /**
     * @brief The result of testing two OrientedRectangle instances for intersection via the
     * separating axis theorem.
     */
    struct OrientedRectangleIntersection
    {
        /** @brief Whether the two rectangles intersect. */
        bool Intersects = false;

        /** @brief The minimum translation vector that separates the two rectangles. */
        Vector2 MinimumTranslationVector;
    };

    /**
     * @brief An oriented bounding rectangle: a rectangular block, much like BoundingRectangle,
     * but with an arbitrary orientation.
     */
    struct OrientedRectangle
    {
        /** @brief The centre position of this OrientedRectangle. */
        Vector2 Center;

        /**
         * @brief The distance from the Center point along both axes to any point on the
         * boundary of this OrientedRectangle.
         */
        Vector2 Radii;

        /** @brief The rotation matrix of this OrientedRectangle. */
        Matrix3x2 Orientation;

        OrientedRectangle() = default;

        /**
         * @brief Initializes a new OrientedRectangle from the specified centre, radii, and
         * orientation.
         * @param center The centre position.
         * @param radii The radii (half-extents along each axis).
         * @param orientation The rotation matrix.
         */
        OrientedRectangle(const Vector2& center, const SizeF& radii, const Matrix3x2& orientation);

        /** @brief Gets a list of points defining the corner points of the oriented rectangle. */
        [[nodiscard]] std::vector<Vector2> getPointsProperty() const;

        /** @brief Gets the position (top-left-most corner, in the rectangle's local space, transformed by Orientation). */
        [[nodiscard]] Vector2 getPositionProperty() const;

        /** @brief Setting the position is not supported by upstream MonoGame.Extended either. */
        void setPositionProperty(const Vector2& value);

        /** @brief Gets the axis-aligned RectangleF that bounds this OrientedRectangle (upstream: `(RectangleF)this`). */
        [[nodiscard]] RectangleF getBoundingRectangleProperty() const;

        /**
         * @brief Computes the OrientedRectangle from the specified OrientedRectangle transformed
         * by the specified Matrix3x2.
         */
        [[nodiscard]] static OrientedRectangle Transform(OrientedRectangle rectangle, Matrix3x2& transformMatrix);

        [[nodiscard]] bool Equals(const OrientedRectangle& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const OrientedRectangle& left, const OrientedRectangle& right) { return left.Equals(right); }
        friend bool operator!=(const OrientedRectangle& left, const OrientedRectangle& right) { return !left.Equals(right); }

        /**
         * @brief Converts a RectangleF to an axis-aligned OrientedRectangle (identity
         * orientation).
         */
        explicit OrientedRectangle(const RectangleF& rectangle);

        /** @brief Explicitly converts this OrientedRectangle to the axis-aligned RectangleF that bounds it. */
        [[nodiscard]] explicit operator RectangleF() const;

        /**
         * @brief Tests two OrientedRectangle instances for intersection using the separating
         * axis theorem, returning whether they intersect and (if so) the minimum translation
         * vector that separates them.
         */
        [[nodiscard]] static OrientedRectangleIntersection Intersects(const OrientedRectangle& rectangle, const OrientedRectangle& other);
    };
}
