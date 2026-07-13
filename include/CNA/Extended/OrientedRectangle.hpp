// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/OrientedRectangle.cs (cites "Real-Time Collision
// Detection", Christer Ericson, 2005, Ch. 4.4 -- an algorithm citation, not a third-party
// code/license dependency; see plan.md). This type was previously deferred in full (task 17,
// "RectangleF family") because its Orientation field is Matrix3x2 itself; Matrix3x2 landed in
// task 20, unblocking most of this type.
//
// Still deferred (transitively, via RectangleF::Transform, which itself needs
// PrimitivesHelper.TransformRectangle -- not ported yet):
//   - `BoundingRectangle` property (upstream: `(RectangleF)this`)
//   - `static Transform(OrientedRectangle, ref Matrix3x2)` (upstream's private overload calls
//     PrimitivesHelper.TransformOrientedRectangle directly)
//   - `explicit operator RectangleF(OrientedRectangle)` (calls RectangleF::Transform internally)
// Everything else -- fields, constructor, Points, Position (get; set throws, matching
// upstream), Equals/GetHashCode/ToString/operators, the OrientedRectangle(RectangleF)
// conversion (does NOT need Transform), and the self-contained SAT Intersects(...) -- is fully
// portable now and ported below.
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

        /**
         * @brief Tests two OrientedRectangle instances for intersection using the separating
         * axis theorem, returning whether they intersect and (if so) the minimum translation
         * vector that separates them.
         */
        [[nodiscard]] static OrientedRectangleIntersection Intersects(const OrientedRectangle& rectangle, const OrientedRectangle& other);
    };
}
