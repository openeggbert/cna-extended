// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/RectangleF.cs. All members are now ported -- `Size`
// property and the `RectangleF(Vector2 position, SizeF size)` constructor landed once `SizeF`
// was ported; `Transform`, `CreateFrom(points, ...)`, `UpdateFromPoints`, `SquaredDistanceTo`,
// `DistanceTo`, and `ClosestPointTo` landed once `Matrix3x2` and `PrimitivesHelper` were ported
// (Phase 1, "PrimitivesHelper, ShapeExtensions" -- verified genuinely unblocked, not assumed,
// before landing these).
#pragma once

#include "CNA/Extended/Matrix3x2.hpp"
#include "CNA/Extended/SizeF.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>
#include <vector>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;

    /**
     * @brief An axis-aligned, four sided, two dimensional box defined by a top-left position
     * (X and Y) and a size (Width and Height).
     */
    struct RectangleF
    {
        /** @brief The RectangleF with X, Y, Width and Height all set to 0.0f. */
        static const RectangleF Empty;

        /** @brief The x-coordinate of the top-left corner position of this RectangleF. */
        float X = 0.0f;

        /** @brief The y-coordinate of the top-left corner position of this RectangleF. */
        float Y = 0.0f;

        /** @brief The width of this RectangleF. */
        float Width = 0.0f;

        /** @brief The height of this RectangleF. */
        float Height = 0.0f;

        /** @brief Gets the x-coordinate of the left edge of this RectangleF. */
        [[nodiscard]] float getLeftProperty() const { return X; }

        /** @brief Gets the x-coordinate of the right edge of this RectangleF. */
        [[nodiscard]] float getRightProperty() const { return X + Width; }

        /** @brief Gets the y-coordinate of the top edge of this RectangleF. */
        [[nodiscard]] float getTopProperty() const { return Y; }

        /** @brief Gets the y-coordinate of the bottom edge of this RectangleF. */
        [[nodiscard]] float getBottomProperty() const { return Y + Height; }

        /** @brief Gets whether X, Y, Width, and Height are all equal to 0.0f. */
        [[nodiscard]] bool getIsEmptyProperty() const { return Width == 0 && Height == 0 && X == 0 && Y == 0; }

        /** @brief Gets the Vector2 representing the top-left of this RectangleF. */
        [[nodiscard]] Vector2 getPositionProperty() const { return Vector2(X, Y); }
        /** @brief Sets the top-left position of this RectangleF. */
        void setPositionProperty(const Vector2& value)
        {
            X = value.X;
            Y = value.Y;
        }

        /** @brief Gets this RectangleF (an IRectangularF-style self-reference, matching upstream's own BoundingRectangle property). */
        [[nodiscard]] RectangleF getBoundingRectangleProperty() const { return *this; }

        /** @brief Gets the SizeF representing the extents of this RectangleF. */
        [[nodiscard]] SizeF getSizeProperty() const { return SizeF(Width, Height); }
        /** @brief Sets the extents (Width/Height) of this RectangleF. */
        void setSizeProperty(const SizeF& value)
        {
            Width = value.Width;
            Height = value.Height;
        }

        /** @brief Gets the Vector2 representing the center of this RectangleF. */
        [[nodiscard]] Vector2 getCenterProperty() const { return Vector2(X + Width * 0.5f, Y + Height * 0.5f); }

        /** @brief Gets the Vector2 representing the top-left of this RectangleF. */
        [[nodiscard]] Vector2 getTopLeftProperty() const { return Vector2(X, Y); }

        /** @brief Gets the Vector2 representing the top-right of this RectangleF. */
        [[nodiscard]] Vector2 getTopRightProperty() const { return Vector2(X + Width, Y); }

        /** @brief Gets the Vector2 representing the bottom-left of this RectangleF. */
        [[nodiscard]] Vector2 getBottomLeftProperty() const { return Vector2(X, Y + Height); }

        /** @brief Gets the Vector2 representing the bottom-right of this RectangleF. */
        [[nodiscard]] Vector2 getBottomRightProperty() const { return Vector2(X + Width, Y + Height); }

        RectangleF() = default;

        /**
         * @brief Initializes a new RectangleF from the specified top-left xy-coordinates, width,
         * and height.
         */
        RectangleF(float x, float y, float width, float height);

        /** @brief Initializes a new RectangleF from the specified top-left position and extents. */
        RectangleF(const Vector2& position, const SizeF& size);

        /**
         * @brief Computes the RectangleF from a minimum Vector2 and maximum Vector2.
         * @param result Receives the resulting rectangle.
         */
        static void CreateFrom(const Vector2& minimum, const Vector2& maximum, RectangleF& result);

        /** @brief Computes the RectangleF from a minimum Vector2 and maximum Vector2. */
        [[nodiscard]] static RectangleF CreateFrom(const Vector2& minimum, const Vector2& maximum);

        /**
         * @brief Computes the smallest RectangleF that contains the specified points.
         * @param result Receives the resulting rectangle.
         */
        static void CreateFrom(const std::vector<Vector2>& points, RectangleF& result);

        /** @brief Computes the smallest RectangleF that contains the specified points. */
        [[nodiscard]] static RectangleF CreateFrom(const std::vector<Vector2>& points);

        /**
         * @brief Transforms a RectangleF by the specified Matrix3x2.
         * @param result Receives the resulting rectangle.
         */
        static void Transform(RectangleF& rectangle, Matrix3x2& transformMatrix, RectangleF& result);

        /** @brief Transforms a RectangleF by the specified Matrix3x2. */
        [[nodiscard]] static RectangleF Transform(RectangleF rectangle, Matrix3x2& transformMatrix);

        /** @brief Updates X, Y, Width and Height from the smallest rectangle that contains the specified points. */
        void UpdateFromPoints(const std::vector<Vector2>& points);

        /** @brief Computes the squared distance from the specified Vector2 to the closest edge/corner of this RectangleF. */
        [[nodiscard]] float SquaredDistanceTo(const Vector2& point) const;

        /** @brief Computes the distance from the specified Vector2 to the closest edge/corner of this RectangleF. */
        [[nodiscard]] float DistanceTo(const Vector2& point) const;

        /** @brief Computes the point on this RectangleF closest to the specified Vector2. */
        [[nodiscard]] Vector2 ClosestPointTo(const Vector2& point) const;

        /**
         * @brief Computes the RectangleF that contains the two specified RectangleF structures.
         * @param result Receives the resulting rectangle.
         */
        static void Union(const RectangleF& first, const RectangleF& second, RectangleF& result);

        /** @brief Computes the RectangleF that contains the two specified RectangleF structures. */
        [[nodiscard]] static RectangleF Union(const RectangleF& first, const RectangleF& second);

        /** @brief Computes the RectangleF that contains both @p rectangle and this RectangleF. */
        [[nodiscard]] RectangleF Union(const RectangleF& rectangle) const;

        /**
         * @brief Computes the RectangleF that represents the intersection of two RectangleF
         * structures.
         * @param result Receives the intersection, or RectangleF::Empty if there is none.
         */
        static void Intersect(const RectangleF& value1, const RectangleF& value2, RectangleF& result);

        /**
         * @brief Computes the RectangleF that represents the intersection of two RectangleF
         * structures, or RectangleF::Empty if there is none.
         */
        [[nodiscard]] static RectangleF Intersect(const RectangleF& value1, const RectangleF& value2);

        /**
         * @brief Computes the RectangleF that represents the intersection of this RectangleF and
         * another, or RectangleF::Empty if there is none.
         */
        [[nodiscard]] RectangleF Intersect(const RectangleF& rectangle) const;

        /** @brief Determines whether the two specified RectangleF structures intersect. */
        [[nodiscard]] static bool Intersects(const RectangleF& first, const RectangleF& second);

        /** @brief Determines whether the specified RectangleF intersects with this RectangleF. */
        [[nodiscard]] bool Intersects(const RectangleF& rectangle) const;

        /**
         * @brief Normalizes this RectangleF so that Width and Height are positive without
         * changing the location of the rectangle.
         */
        void Normalize();

        /**
         * @brief Normalizes the specified RectangleF so that Width and Height are positive
         * without changing the location of the rectangle.
         */
        [[nodiscard]] static RectangleF Normalize(RectangleF rectangle);

        /**
         * @brief Normalizes a RectangleF so that Width and Height are positive without changing
         * the location of the rectangle.
         * @param result Receives the normalized rectangle.
         */
        static void Normalize(const RectangleF& rectangle, RectangleF& result);

        /** @brief Determines whether the specified RectangleF contains the specified Vector2. */
        [[nodiscard]] static bool Contains(const RectangleF& rectangle, const Vector2& point);

        /** @brief Determines whether this RectangleF contains the specified Vector2. */
        [[nodiscard]] bool Contains(const Vector2& point) const;

        //TODO: Document this.
        void Inflate(float horizontalAmount, float verticalAmount);

        //TODO: Document this.
        void Offset(float offsetX, float offsetY);

        //TODO: Document this.
        void Offset(const Vector2& amount);

        [[nodiscard]] bool Equals(const RectangleF& rectangle) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const RectangleF& first, const RectangleF& second) { return first.Equals(second); }
        friend bool operator!=(const RectangleF& first, const RectangleF& second) { return !first.Equals(second); }

        /** @brief Implicitly converts a Rectangle to a RectangleF (matches upstream's implicit operator). */
        RectangleF(const Rectangle& rectangle); // NOLINT(*-explicit-constructor)

        /**
         * @brief Explicitly converts a RectangleF to a Rectangle. A loss of precision may occur
         * due to the truncation from float to int.
         */
        [[nodiscard]] explicit operator Rectangle() const;
    };
}
