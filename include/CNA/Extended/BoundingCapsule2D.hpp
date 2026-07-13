// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's BoundingCapsule2D.cs. Upstream's Contains(...)/Intersects(...)/
// TryGetCollision(...) overloads all delegate to static query functions on MonoGame.Extended's
// Collision2D type, and CreateFromSegment/CreateMerged delegate to LineSegment2D's
// DistanceToPoint/DistanceSquaredToPoint -- now that Collision2D and LineSegment2D are both fully
// ported (Phase 2), all of them are landed below: 6 Contains (point, aabb, circle, obb, self,
// polygon), 5 Intersects (self, circle, aabb, obb, polygon), and CreateFromSegment/CreateMerged.
//
// Asymmetric coverage (matching Collision2D exactly, not an omission): Collision2D provides only
// one TryGetCollision* function involving a capsule -- TryGetCollisionCircleCapsule -- so
// TryGetCollision(BoundingCircle2D, CollisionResult2D&) is the only TryGetCollision overload landed
// here. Upstream has no TryGetCollision(BoundingBox2D)/TryGetCollision(OrientedBoundingBox2D)/
// TryGetCollision(BoundingCapsule2D)/TryGetCollision(BoundingPolygon2D) either, and Collision2D
// itself has no matching TryGetCollisionAabbCapsule/TryGetCollisionObbCapsule/
// TryGetCollisionCapsuleCapsule/TryGetCollisionCapsuleConvexPolygon functions to wrap -- confirmed
// by reading Collision2D.hpp -- so there is genuinely nothing more to wrap.
//
// TryGetCollision(BoundingCircle2D) calls Collision2D::TryGetCollisionCircleCapsule (Collision2D
// has no TryGetCollisionCapsuleCircle) and then calls .Invert() on the result, since
// TryGetCollisionCircleCapsule's MTV convention moves the circle out of the capsule, but this
// method's contract moves the capsule out of the circle -- matching upstream exactly.
#pragma once

#include "CNA/Extended/CollisionResult2D.hpp"
#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::ContainmentType;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector2;

    struct BoundingBox2D;
    struct BoundingCircle2D;
    struct OrientedBoundingBox2D;
    struct BoundingPolygon2D;
    struct LineSegment2D;

    /** @brief Represents a capsule bounding volume in 2D space, formed by sweeping a circle along a line segment. */
    struct BoundingCapsule2D
    {
        /** @brief The first endpoint of the capsule's central line segment in 2D space. */
        Vector2 PointA;

        /** @brief The second endpoint of the capsule's central line segment in 2D space. */
        Vector2 PointB;

        /** @brief The radius of this capsule, defining the circular caps and the width of the swept region. */
        float Radius = 0.0f;

        BoundingCapsule2D() = default;

        /**
         * @brief Creates a new BoundingCapsule2D with the specified endpoints and radius.
         * @param pointA The first endpoint of the capsule's central line segment in 2D space.
         * @param pointB The second endpoint of the capsule's central line segment in 2D space.
         * @param radius The radius of the capsule. Should be non-negative.
         */
        BoundingCapsule2D(const Vector2& pointA, const Vector2& pointB, float radius);

        /** @brief Gets the center of this capsule in 2D space, located at the midpoint between the two endpoints. */
        [[nodiscard]] Vector2 getCenterProperty() const { return (PointA + PointB) * 0.5f; }

        /** @brief Gets the length of this capsule's central line segment. */
        [[nodiscard]] float getLengthProperty() const;

        /**
         * @brief Gets the squared length of this capsule's central line segment, useful for
         * distance comparisons without square root calculations.
         */
        [[nodiscard]] float getLengthSquaredProperty() const;

        /**
         * @brief Gets the unit direction vector from PointA toward PointB, defining this
         * capsule's orientation. Returns Vector2::Zero when the endpoints are identical
         * (degenerate capsule).
         */
        [[nodiscard]] Vector2 getDirectionProperty() const;

        /**
         * @brief Gets the total area enclosed by this capsule: the area of the central rectangle
         * plus the area of the two semicircular caps, which together form a complete circle.
         */
        [[nodiscard]] float getAreaProperty() const;

        /**
         * @brief Creates a new BoundingCapsule2D from a center point, direction, length, and
         * radius. The direction is normalized automatically.
         * @param center The center point of the capsule in 2D space.
         * @param direction The direction vector defining the capsule's orientation.
         * @param length The length of the capsule's central line segment.
         * @param radius The radius of the capsule.
         */
        [[nodiscard]] static BoundingCapsule2D CreateFromCenterAndDirection(
            const Vector2& center, const Vector2& direction, float length, float radius);

        /**
         * @brief Creates a new BoundingCapsule2D from a line segment and radius, using the
         * segment's endpoints directly as the capsule's PointA/PointB.
         * @param segment The line segment defining the capsule's central axis.
         * @param radius The radius of the capsule.
         */
        [[nodiscard]] static BoundingCapsule2D CreateFromSegment(const LineSegment2D& segment, float radius);

        /**
         * @brief Creates a new BoundingCapsule2D that encloses two capsules, using Ericson's
         * "Sphere-Swept Volumes" merging approach: the new central segment runs between whichever
         * two of the four input endpoints are farthest apart, and the radius grows to cover all
         * four original endpoints plus their original radii.
         * @param original The first capsule to enclose.
         * @param additional The second capsule to enclose.
         */
        [[nodiscard]] static BoundingCapsule2D CreateMerged(const BoundingCapsule2D& original, const BoundingCapsule2D& additional);

        /**
         * @brief Applies a matrix transformation to this capsule and creates a new transformed
         * capsule. The radius is scaled by the maximum of the X and Y scale components of the
         * transformation matrix.
         * @param matrix The transformation matrix to apply.
         */
        [[nodiscard]] BoundingCapsule2D Transform(const Matrix& matrix) const;

        /**
         * @brief Creates a new BoundingCapsule2D by translating this capsule by the specified offset.
         * @param translation The offset to translate the capsule by in 2D space.
         */
        [[nodiscard]] BoundingCapsule2D Translate(const Vector2& translation) const;

        /**
         * @brief Deconstructs this capsule into its component values.
         * @param pointA Receives the first endpoint.
         * @param pointB Receives the second endpoint.
         * @param radius Receives the radius.
         */
        void Deconstruct(Vector2& pointA, Vector2& pointB, float& radius) const;

        /** @brief Determines whether this capsule contains the specified point. */
        [[nodiscard]] ContainmentType Contains(const Vector2& point) const;
        /** @brief Determines whether this capsule contains a bounding box. */
        [[nodiscard]] ContainmentType Contains(const BoundingBox2D& aabb) const;
        /** @brief Determines whether this capsule contains a bounding circle. */
        [[nodiscard]] ContainmentType Contains(const BoundingCircle2D& circle) const;
        /** @brief Determines whether this capsule contains an oriented bounding box. */
        [[nodiscard]] ContainmentType Contains(const OrientedBoundingBox2D& obb) const;
        /** @brief Determines whether this capsule contains another capsule. */
        [[nodiscard]] ContainmentType Contains(const BoundingCapsule2D& other) const;
        /** @brief Determines whether this capsule contains a bounding polygon. */
        [[nodiscard]] ContainmentType Contains(const BoundingPolygon2D& polygon) const;

        /** @brief Determines whether this capsule intersects another capsule. */
        [[nodiscard]] bool Intersects(const BoundingCapsule2D& other) const;
        /** @brief Determines whether this capsule intersects a bounding circle. */
        [[nodiscard]] bool Intersects(const BoundingCircle2D& circle) const;
        /** @brief Determines whether this capsule intersects a bounding box. */
        [[nodiscard]] bool Intersects(const BoundingBox2D& box) const;
        /** @brief Determines whether this capsule intersects an oriented bounding box. */
        [[nodiscard]] bool Intersects(const OrientedBoundingBox2D& obb) const;
        /** @brief Determines whether this capsule intersects a bounding polygon. */
        [[nodiscard]] bool Intersects(const BoundingPolygon2D& polygon) const;

        /** @brief Attempts to compute collision information between this capsule and a bounding circle. */
        [[nodiscard]] bool TryGetCollision(const BoundingCircle2D& circle, CollisionResult2D& result) const;

        [[nodiscard]] bool Equals(const BoundingCapsule2D& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const BoundingCapsule2D& left, const BoundingCapsule2D& right) { return left.Equals(right); }
        friend bool operator!=(const BoundingCapsule2D& left, const BoundingCapsule2D& right) { return !left.Equals(right); }
    };
}
