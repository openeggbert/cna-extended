// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's BoundingCircle2D.cs. Now that Collision2D has landed (Phase 2),
// all upstream Contains(...)/Intersects(...)/TryGetCollision(...) overloads are ported below as
// thin wrappers delegating to the matching Collision2D::ContainsCircleXxx/IntersectsCircleXxx/
// TryGetCollisionCircleXxx static method, using this circle's Center/Radius plus the other
// shape's own public fields -- except TryGetCollision(BoundingPolygon2D), which upstream itself
// never defines (there is no Collision2D.TryGetCollisionCircleConvexPolygon in the C# source
// either): Contains/Intersects both have full Circle/Aabb/Circle/Obb/Capsule/ConvexPolygon
// coverage (6 and 5 overloads respectively), but TryGetCollision only covers Circle/Aabb/Obb/
// Capsule (4 overloads) -- a genuine upstream asymmetry, preserved faithfully rather than
// "completed" with an invented overload.
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
    class BoundingCapsule2D;
    class OrientedBoundingBox2D;
    class BoundingPolygon2D;

    /** @brief Represents a circular bounding volume in 2D space, defined by a center point and radius. */
    struct BoundingCircle2D
    {
        /** @brief The center position of this circle in 2D space. */
        Vector2 Center;

        /** @brief The radius of this circle, representing the distance from the center to any point on the edge. */
        float Radius = 0.0f;

        BoundingCircle2D() = default;

        /**
         * @brief Creates a new BoundingCircle2D with the specified center and radius.
         * @param center The center position of the circle in 2D space.
         * @param radius The radius of the circle. Should be non-negative.
         */
        BoundingCircle2D(const Vector2& center, float radius);

        /**
         * @brief Gets the squared radius of this circle, useful for distance comparisons without
         * square root calculations.
         */
        [[nodiscard]] float getRadiusSquaredProperty() const { return Radius * Radius; }

        /** @brief Gets the diameter of this circle, the distance across through the center. */
        [[nodiscard]] float getDiameterProperty() const { return Radius * 2.0f; }

        /** @brief Gets the area enclosed by this circle. */
        [[nodiscard]] float getAreaProperty() const;

        /**
         * @brief Creates a BoundingCircle2D that encloses all specified points, using Ritter's
         * algorithm for efficient approximate bounding circle computation. May not be the
         * absolute minimal bounding circle. For a single point, returns a circle with radius 0
         * centered at that point.
         * @param points The points to enclose within the circle. Must not be empty.
         */
        [[nodiscard]] static BoundingCircle2D CreateFromPoints(const std::vector<Vector2>& points);

        /**
         * @brief Creates a BoundingCircle2D that completely encloses the specified bounding box.
         * @param box The bounding box to enclose within a circle.
         */
        [[nodiscard]] static BoundingCircle2D CreateFromBoundingBox2D(const BoundingBox2D& box);

        /**
         * @brief Creates a BoundingCircle2D that completely encloses the specified capsule. The
         * radius is computed as half the capsule's length plus the capsule's radius, ensuring
         * that the circle reaches the farthest points on both circular caps.
         * @param capsule The capsule to enclose within a circle.
         */
        [[nodiscard]] static BoundingCircle2D CreateFromBoundingCapsule2D(const BoundingCapsule2D& capsule);

        /**
         * @brief Creates a BoundingCircle2D with the smallest radius that completely contains
         * both input circles. If one circle completely contains the other, the larger circle is
         * returned.
         * @param original The first bounding circle to enclose.
         * @param additional The second bounding circle to enclose.
         */
        [[nodiscard]] static BoundingCircle2D CreateMerged(const BoundingCircle2D& original, const BoundingCircle2D& additional);

        /** @brief Determines whether this circle contains the specified point. */
        [[nodiscard]] ContainmentType Contains(const Vector2& point) const;

        /** @brief Determines whether this circle contains the specified bounding box. */
        [[nodiscard]] ContainmentType Contains(const BoundingBox2D& box) const;

        /** @brief Determines whether this circle contains the specified other circle. */
        [[nodiscard]] ContainmentType Contains(const BoundingCircle2D& other) const;

        /** @brief Determines whether this circle contains the specified oriented bounding box. */
        [[nodiscard]] ContainmentType Contains(const OrientedBoundingBox2D& obb) const;

        /** @brief Determines whether this circle contains the specified capsule. */
        [[nodiscard]] ContainmentType Contains(const BoundingCapsule2D& capsule) const;

        /** @brief Determines whether this circle contains the specified convex polygon. */
        [[nodiscard]] ContainmentType Contains(const BoundingPolygon2D& polygon) const;

        /** @brief Determines whether this circle intersects the specified other circle. */
        [[nodiscard]] bool Intersects(const BoundingCircle2D& other) const;

        /** @brief Determines whether this circle intersects the specified bounding box. */
        [[nodiscard]] bool Intersects(const BoundingBox2D& box) const;

        /** @brief Determines whether this circle intersects the specified capsule. */
        [[nodiscard]] bool Intersects(const BoundingCapsule2D& capsule) const;

        /** @brief Determines whether this circle intersects the specified oriented bounding box. */
        [[nodiscard]] bool Intersects(const OrientedBoundingBox2D& obb) const;

        /** @brief Determines whether this circle intersects the specified convex polygon. */
        [[nodiscard]] bool Intersects(const BoundingPolygon2D& polygon) const;

        /**
         * @brief Determines whether this circle intersects the specified other circle, and
         * computes collision resolution data.
         * @param other The other circle to test against.
         * @param result Receives the collision resolution data when an intersection is found.
         */
        bool TryGetCollision(const BoundingCircle2D& other, CollisionResult2D& result) const;

        /**
         * @brief Determines whether this circle intersects the specified bounding box, and
         * computes collision resolution data.
         * @param box The bounding box to test against.
         * @param result Receives the collision resolution data when an intersection is found.
         */
        bool TryGetCollision(const BoundingBox2D& box, CollisionResult2D& result) const;

        /**
         * @brief Determines whether this circle intersects the specified capsule, and computes
         * collision resolution data.
         * @param capsule The capsule to test against.
         * @param result Receives the collision resolution data when an intersection is found.
         */
        bool TryGetCollision(const BoundingCapsule2D& capsule, CollisionResult2D& result) const;

        /**
         * @brief Determines whether this circle intersects the specified oriented bounding box,
         * and computes collision resolution data.
         * @param obb The oriented bounding box to test against.
         * @param result Receives the collision resolution data when an intersection is found.
         */
        bool TryGetCollision(const OrientedBoundingBox2D& obb, CollisionResult2D& result) const;

        /**
         * @brief Applies a matrix transformation to this circle and creates a new transformed
         * circle. The radius is scaled by the maximum of the X and Y scale components of the
         * transformation matrix.
         * @param matrix The transformation matrix to apply.
         */
        [[nodiscard]] BoundingCircle2D Transform(const Matrix& matrix) const;

        /**
         * @brief Creates a new BoundingCircle2D by translating this circle by the specified offset.
         * @param translation The offset to translate the circle by in 2D space.
         */
        [[nodiscard]] BoundingCircle2D Translate(const Vector2& translation) const;

        /**
         * @brief Deconstructs this circle into its component values.
         * @param center Receives the center position.
         * @param radius Receives the radius.
         */
        void Deconstruct(Vector2& center, float& radius) const;

        [[nodiscard]] bool Equals(const BoundingCircle2D& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const BoundingCircle2D& left, const BoundingCircle2D& right) { return left.Equals(right); }
        friend bool operator!=(const BoundingCircle2D& left, const BoundingCircle2D& right) { return !left.Equals(right); }
    };
}
