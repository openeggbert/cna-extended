// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's BoundingCapsule2D.cs. Contains(...)/Intersects(...)/
// TryGetCollision(...) overloads are deferred until Collision2D is ported (Phase 2) -- see the
// header comment in BoundingBox2D.hpp for the full rationale, which applies identically here.
//
// Two additional members are deferred for a second reason: `CreateFromSegment(LineSegment2D, float)`
// takes a LineSegment2D parameter, and `CreateMerged` calls LineSegment2D::DistanceToPoint
// internally -- LineSegment2D is ported later in this same phase (plan.md Phase 1, "Line2D,
// LineSegment2D, Ray2D"). Unlike the Collision2D::Epsilon constant (trivial enough to duplicate
// locally, see BoundingCircle2D.cpp), point-to-segment distance is a real algorithm that should
// come from LineSegment2D itself once it exists, not be re-derived here. Add both back as a
// follow-up once LineSegment2D lands.
#pragma once

#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector2;

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

        [[nodiscard]] bool Equals(const BoundingCapsule2D& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const BoundingCapsule2D& left, const BoundingCapsule2D& right) { return left.Equals(right); }
        friend bool operator!=(const BoundingCapsule2D& left, const BoundingCapsule2D& right) { return !left.Equals(right); }
    };
}
