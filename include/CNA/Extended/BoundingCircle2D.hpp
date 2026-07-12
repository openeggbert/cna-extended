// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's BoundingCircle2D.cs. Contains(...)/Intersects(...)/
// TryGetCollision(...) overloads are deferred until Collision2D is ported (Phase 2) -- see the
// header comment in BoundingBox2D.hpp for the full rationale, which applies identically here.
#pragma once

#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>
#include <vector>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector2;

    class BoundingBox2D;
    class BoundingCapsule2D;

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
