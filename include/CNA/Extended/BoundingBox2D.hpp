// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's BoundingBox2D.cs. Upstream's Contains(...)/Intersects(...)/
// TryGetCollision(...) overloads all delegate to static query functions on MonoGame.Extended's
// Collision2D type -- now that Collision2D is fully ported (Phase 2), all of them are landed
// below: 6 Contains (point, self, circle, obb, capsule, polygon), 5 Intersects (self, circle,
// capsule, obb, polygon), 4 TryGetCollision (self, circle, obb, polygon -- upstream has no
// TryGetCollision(BoundingCapsule2D) overload; Collision2D itself has no matching
// TryGetCollisionAabbCapsule function, confirmed by reading Collision2D.hpp, so there is
// genuinely nothing to wrap here -- this is not an omission). Everything else (fields,
// properties, factory methods, GetCorners, Transform, Translate, Deconstruct, equality,
// ToString) was already ported.
//
// Two asymmetric-argument-order quirks, both matching upstream exactly (not simplifications):
//   - Intersects(BoundingCircle2D)/TryGetCollision(BoundingCircle2D) call
//     Collision2D::IntersectsCircleAabb/TryGetCollisionCircleAabb (Collision2D has no
//     IntersectsAabbCircle/TryGetCollisionAabbCircle) -- TryGetCollision additionally calls
//     .Invert() on the result, since TryGetCollisionCircleAabb's MTV convention moves the
//     circle out of the box, but this method's contract moves the box out of the circle.
//   - Intersects(OrientedBoundingBox2D)/Intersects(BoundingPolygon2D)/their TryGetCollision
//     counterparts pass this box's Center/HalfExtents (not Min/Max) to the matching
//     Collision2D::*AabbObb/*AabbConvexPolygon functions, which take a center/half-extents AABB
//     representation -- unlike every other overload here, which passes Min/Max directly.
#pragma once

#include "CNA/Extended/CollisionResult2D.hpp"
#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>
#include <vector>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::ContainmentType;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector2;

    struct BoundingCircle2D;
    struct OrientedBoundingBox2D;
    struct BoundingCapsule2D;
    struct BoundingPolygon2D;

    /**
     * @brief Represents an axis-aligned bounding box in 2D space, defined by minimum and maximum
     * corner points.
     */
    struct BoundingBox2D
    {
        /** @brief The number of corners in this bounding box. */
        static constexpr int CornerCount = 4;

        /** @brief The minimum corner position with the smallest X and Y coordinates. */
        Vector2 Min;

        /** @brief The maximum corner position with the largest X and Y coordinates. */
        Vector2 Max;

        BoundingBox2D() = default;

        /**
         * @brief Creates a new BoundingBox2D with the specified minimum and maximum corners.
         * @param min The minimum corner, containing the smallest X and Y coordinate values.
         * @param max The maximum corner, containing the largest X and Y coordinate values.
         */
        BoundingBox2D(const Vector2& min, const Vector2& max);

        /** @brief Gets the center position of this bounding box in 2D space. */
        [[nodiscard]] Vector2 getCenterProperty() const { return (Min + Max) * 0.5f; }

        /** @brief Gets the size of this bounding box as width and height. */
        [[nodiscard]] Vector2 getSizeProperty() const { return Max - Min; }

        /**
         * @brief Gets the half extents of this bounding box, representing the distance from the
         * center to each edge.
         */
        [[nodiscard]] Vector2 getHalfExtentsProperty() const { return (Max - Min) * 0.5f; }

        /** @brief Gets the width of this bounding box. */
        [[nodiscard]] float getWidthProperty() const { return Max.X - Min.X; }

        /** @brief Gets the height of this bounding box. */
        [[nodiscard]] float getHeightProperty() const { return Max.Y - Min.Y; }

        /** @brief Gets the area enclosed by this bounding box. */
        [[nodiscard]] float getAreaProperty() const { return getWidthProperty() * getHeightProperty(); }

        /**
         * @brief Creates a new BoundingBox2D from the specified minimum and maximum corners.
         * @param min The minimum corner, containing the smallest X and Y coordinate values.
         * @param max The maximum corner, containing the largest X and Y coordinate values.
         */
        [[nodiscard]] static BoundingBox2D CreateFromMinMax(const Vector2& min, const Vector2& max);

        /**
         * @brief Creates a new BoundingBox2D from a center position and half extents.
         * @param center The center position of the bounding box in 2D space.
         * @param halfExtents The half extents representing the distance from the center to each
         * edge (half-width and half-height).
         */
        [[nodiscard]] static BoundingBox2D CreateFromCenterAndExtents(const Vector2& center, const Vector2& halfExtents);

        /**
         * @brief Creates a new BoundingBox2D from a minimum corner position and size.
         * @param position The minimum corner position in 2D space.
         * @param size The size as width and height.
         */
        [[nodiscard]] static BoundingBox2D CreateFromPositionAndSize(const Vector2& position, const Vector2& size);

        /**
         * @brief Creates a new BoundingBox2D that encloses all specified points.
         * @param points The points to enclose within the bounding box. Must not be empty.
         */
        [[nodiscard]] static BoundingBox2D CreateFromPoints(const std::vector<Vector2>& points);

        /**
         * @brief Creates a new BoundingBox2D that encloses two bounding boxes.
         * @param original The first bounding box to enclose.
         * @param additional The second bounding box to enclose.
         */
        [[nodiscard]] static BoundingBox2D CreateMerged(const BoundingBox2D& original, const BoundingBox2D& additional);

        /**
         * @brief Gets an array containing the four corner positions of this bounding box, in
         * counter-clockwise order starting from the minimum corner: bottom-left, bottom-right,
         * top-right, top-left.
         */
        [[nodiscard]] std::vector<Vector2> GetCorners() const;

        /**
         * @brief Writes the four corner positions of this bounding box into an existing array.
         * @param corners The array to write corner positions into. Must have at least 4 elements.
         */
        void GetCorners(Vector2 corners[4]) const;

        /**
         * @brief Applies a matrix transformation to this bounding box and creates a new
         * axis-aligned bounding box that encloses the result.
         * @param matrix The transformation matrix to apply.
         */
        [[nodiscard]] BoundingBox2D Transform(const Matrix& matrix) const;

        /**
         * @brief Creates a new BoundingBox2D by translating this bounding box by the specified
         * offset.
         * @param translation The offset to translate the bounding box by in 2D space.
         */
        [[nodiscard]] BoundingBox2D Translate(const Vector2& translation) const;

        /**
         * @brief Deconstructs this bounding box into its component values.
         * @param min Receives the minimum corner.
         * @param max Receives the maximum corner.
         */
        void Deconstruct(Vector2& min, Vector2& max) const;

        /** @brief Determines whether this bounding box contains the specified point. */
        [[nodiscard]] ContainmentType Contains(const Vector2& point) const;
        /** @brief Determines whether this bounding box contains another bounding box. */
        [[nodiscard]] ContainmentType Contains(const BoundingBox2D& other) const;
        /** @brief Determines whether this bounding box contains a bounding circle. */
        [[nodiscard]] ContainmentType Contains(const BoundingCircle2D& circle) const;
        /** @brief Determines whether this bounding box contains an oriented bounding box. */
        [[nodiscard]] ContainmentType Contains(const OrientedBoundingBox2D& obb) const;
        /** @brief Determines whether this bounding box contains a bounding capsule. */
        [[nodiscard]] ContainmentType Contains(const BoundingCapsule2D& capsule) const;
        /** @brief Determines whether this bounding box contains a bounding polygon. */
        [[nodiscard]] ContainmentType Contains(const BoundingPolygon2D& polygon) const;

        /** @brief Determines whether this bounding box intersects another bounding box. */
        [[nodiscard]] bool Intersects(const BoundingBox2D& other) const;
        /** @brief Determines whether this bounding box intersects a bounding circle. */
        [[nodiscard]] bool Intersects(const BoundingCircle2D& circle) const;
        /** @brief Determines whether this bounding box intersects a bounding capsule. */
        [[nodiscard]] bool Intersects(const BoundingCapsule2D& capsule) const;
        /** @brief Determines whether this bounding box intersects an oriented bounding box. */
        [[nodiscard]] bool Intersects(const OrientedBoundingBox2D& obb) const;
        /** @brief Determines whether this bounding box intersects a bounding polygon. */
        [[nodiscard]] bool Intersects(const BoundingPolygon2D& polygon) const;

        /** @brief Attempts to compute collision information between this bounding box and another bounding box. */
        [[nodiscard]] bool TryGetCollision(const BoundingBox2D& other, CollisionResult2D& result) const;
        /** @brief Attempts to compute collision information between this bounding box and a bounding circle. */
        [[nodiscard]] bool TryGetCollision(const BoundingCircle2D& circle, CollisionResult2D& result) const;
        /** @brief Attempts to compute collision information between this bounding box and an oriented bounding box. */
        [[nodiscard]] bool TryGetCollision(const OrientedBoundingBox2D& obb, CollisionResult2D& result) const;
        /** @brief Attempts to compute collision information between this bounding box and a bounding polygon. */
        [[nodiscard]] bool TryGetCollision(const BoundingPolygon2D& polygon, CollisionResult2D& result) const;

        [[nodiscard]] bool Equals(const BoundingBox2D& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const BoundingBox2D& left, const BoundingBox2D& right) { return left.Equals(right); }
        friend bool operator!=(const BoundingBox2D& left, const BoundingBox2D& right) { return !left.Equals(right); }
    };
}
