// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's OrientedBoundingBox2D.cs. Contains(...)/Intersects(...)/
// TryGetCollision(...) now delegate to Collision2D, which is fully ported -- see
// CollisionResult2D.hpp/Collision2D.hpp. As with BoundingBox2D, not every shape pair has a
// TryGetCollision overload upstream: there is no TryGetCollision(BoundingCapsule2D) here because
// Collision2D has no TryGetCollisionObbCapsule (only IntersectsObbCapsule).
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

    class BoundingBox2D;
    struct BoundingCircle2D;
    struct BoundingCapsule2D;
    struct BoundingPolygon2D;

    /** @brief Represents an oriented bounding box in 2D space. */
    struct OrientedBoundingBox2D
    {
        /** @brief The number of corners in this oriented bounding box. */
        static constexpr int CornerCount = 4;

        /** @brief The center position of this oriented bounding box in 2D space. */
        Vector2 Center;

        /** @brief The unit vector defining this box's local X-axis direction. */
        Vector2 AxisX;

        /** @brief The unit vector defining this box's local Y-axis direction, perpendicular to the X-axis. */
        Vector2 AxisY;

        /** @brief The half extents of this box, representing the distance from the center to each edge along the local axes. */
        Vector2 HalfExtents;

        OrientedBoundingBox2D() = default;

        /**
         * @brief Creates a new OrientedBoundingBox2D with the specified center, orientation, and extents.
         * @param center The center position of the box in 2D space.
         * @param axisX The unit vector defining the local X-axis direction.
         * @param axisY The unit vector defining the local Y-axis direction, perpendicular to axisX.
         * @param halfExtents The half extents, representing the distance from the center to each edge along the local axes.
         */
        OrientedBoundingBox2D(const Vector2& center, const Vector2& axisX, const Vector2& axisY, const Vector2& halfExtents);

        /** @brief Gets the width of this box along its local X-axis. */
        [[nodiscard]] float getWidthProperty() const { return HalfExtents.X * 2.0f; }

        /** @brief Gets the height of this box along its local Y-axis. */
        [[nodiscard]] float getHeightProperty() const { return HalfExtents.Y * 2.0f; }

        /** @brief Gets the rotation angle of this box in radians, measured counter-clockwise from the positive world X-axis. */
        [[nodiscard]] float getRotationProperty() const;

        /** @brief Gets the area enclosed by this box. */
        [[nodiscard]] float getAreaProperty() const { return getWidthProperty() * getHeightProperty(); }

        /**
         * @brief Creates a new OrientedBoundingBox2D from a center, rotation angle, and extents.
         * @param center The center position of the box in 2D space.
         * @param rotation The rotation angle in radians, measured counter-clockwise from the positive world X-axis.
         * @param halfExtents The half extents, representing the distance from the center to each edge along the local axes.
         */
        [[nodiscard]] static OrientedBoundingBox2D CreateFromRotation(const Vector2& center, float rotation, const Vector2& halfExtents);

        /**
         * @brief Creates a new OrientedBoundingBox2D from an axis-aligned bounding box with zero rotation.
         * @param box The axis-aligned bounding box to convert.
         */
        [[nodiscard]] static OrientedBoundingBox2D CreateFromBoundingBox2D(const BoundingBox2D& box);

        /**
         * @brief Creates an OrientedBoundingBox2D that encloses two oriented bounding boxes,
         * using Principal Component Analysis (PCA) with power iteration to compute optimal axes
         * for the merged box. The resulting box may not be the absolute minimum volume, but
         * provides a good approximation that aligns with the principal direction of the combined
         * corners.
         * @param original The first oriented bounding box to enclose.
         * @param additional The second oriented bounding box to enclose.
         */
        [[nodiscard]] static OrientedBoundingBox2D CreateMerged(const OrientedBoundingBox2D& original, const OrientedBoundingBox2D& additional);

        /**
         * @brief Gets an array containing the four corner positions of this oriented bounding box,
         * in order: top-left, top-right, bottom-right, bottom-left, relative to the box's local orientation.
         */
        [[nodiscard]] std::vector<Vector2> GetCorners() const;

        /**
         * @brief Writes the four corner positions of this oriented bounding box into an existing array.
         * @param corners The array to write corner positions into. Must have at least 4 elements.
         */
        void GetCorners(Vector2 corners[4]) const;

        /**
         * @brief Applies a matrix transformation to this oriented bounding box and creates a new
         * transformed box. The center is transformed as a point, the local axes are rotated and
         * normalized, and the half extents are scaled by the corresponding scale factors
         * extracted from the transformation.
         * @param matrix The transformation matrix to apply.
         */
        [[nodiscard]] OrientedBoundingBox2D Transform(const Matrix& matrix) const;

        /**
         * @brief Creates a new OrientedBoundingBox2D by translating this box by the specified offset.
         * @param translation The offset to translate the box by in 2D space.
         */
        [[nodiscard]] OrientedBoundingBox2D Translate(const Vector2& translation) const;

        /**
         * @brief Deconstructs this oriented bounding box into its component values.
         * @param center Receives the center position.
         * @param axisX Receives the local X-axis direction.
         * @param axisY Receives the local Y-axis direction.
         * @param halfExtents Receives the half extents.
         */
        void Deconstruct(Vector2& center, Vector2& axisX, Vector2& axisY, Vector2& halfExtents) const;

        /**
         * @brief Tests whether a point lies inside this oriented bounding box or on its boundary.
         * @param point The point to test in 2D space.
         */
        [[nodiscard]] ContainmentType Contains(const Vector2& point) const;

        /**
         * @brief Tests whether this oriented bounding box contains, intersects, or is separate from a bounding box.
         * @param aabb The bounding box to test against.
         */
        [[nodiscard]] ContainmentType Contains(const BoundingBox2D& aabb) const;

        /**
         * @brief Tests whether this oriented bounding box contains, intersects, or is separate from a circle.
         * @param circle The circle to test against.
         */
        [[nodiscard]] ContainmentType Contains(const BoundingCircle2D& circle) const;

        /**
         * @brief Tests whether this oriented bounding box contains, intersects, or is separate from another oriented bounding box.
         * @param other The other oriented bounding box to test against.
         */
        [[nodiscard]] ContainmentType Contains(const OrientedBoundingBox2D& other) const;

        /**
         * @brief Tests whether this oriented bounding box contains, intersects, or is separate from a capsule.
         * @param capsule The capsule to test against.
         */
        [[nodiscard]] ContainmentType Contains(const BoundingCapsule2D& capsule) const;

        /**
         * @brief Tests whether this oriented bounding box contains, intersects, or is separate from a polygon.
         * @param polygon The polygon to test against.
         */
        [[nodiscard]] ContainmentType Contains(const BoundingPolygon2D& polygon) const;

        /** @brief Tests whether this oriented bounding box intersects with a circle. */
        [[nodiscard]] bool Intersects(const BoundingCircle2D& circle) const;

        /** @brief Tests whether this oriented bounding box intersects with another oriented bounding box. */
        [[nodiscard]] bool Intersects(const OrientedBoundingBox2D& other) const;

        /** @brief Tests whether this oriented bounding box intersects with an axis-aligned bounding box. */
        [[nodiscard]] bool Intersects(const BoundingBox2D& box) const;

        /** @brief Tests whether this oriented bounding box intersects with a capsule. */
        [[nodiscard]] bool Intersects(const BoundingCapsule2D& capsule) const;

        /** @brief Tests whether this oriented bounding box intersects with a polygon. */
        [[nodiscard]] bool Intersects(const BoundingPolygon2D& polygon) const;

        /**
         * @brief Tests whether this oriented bounding box intersects with a circle, and returns collision
         * resolution data when they intersect (normal moves this box out of @p circle).
         */
        [[nodiscard]] bool TryGetCollision(const BoundingCircle2D& circle, CollisionResult2D& result) const;

        /**
         * @brief Tests whether this oriented bounding box intersects with another oriented bounding box, and returns
         * collision resolution data when they intersect (normal moves this box out of @p other).
         */
        [[nodiscard]] bool TryGetCollision(const OrientedBoundingBox2D& other, CollisionResult2D& result) const;

        /**
         * @brief Tests whether this oriented bounding box intersects with an axis-aligned bounding box, and returns
         * collision resolution data when they intersect (normal moves this box out of @p box).
         */
        [[nodiscard]] bool TryGetCollision(const BoundingBox2D& box, CollisionResult2D& result) const;

        /**
         * @brief Tests whether this oriented bounding box intersects with a polygon, and returns collision
         * resolution data when they intersect (normal moves this box out of @p polygon).
         */
        [[nodiscard]] bool TryGetCollision(const BoundingPolygon2D& polygon, CollisionResult2D& result) const;

        [[nodiscard]] bool Equals(const OrientedBoundingBox2D& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const OrientedBoundingBox2D& left, const OrientedBoundingBox2D& right) { return left.Equals(right); }
        friend bool operator!=(const OrientedBoundingBox2D& left, const OrientedBoundingBox2D& right) { return !left.Equals(right); }
    };
}
