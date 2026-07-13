// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/CircleF.cs. Upstream cites "Real-Time Collision
// Detection, Christer Ericson, 2005" (Chapter 4.3, pg 88) as the algorithmic source for several
// methods -- an academic citation in a comment, not a third-party code/license dependency like
// the SlimMath case in Angle.cs (this file's code itself is still 100% Craftwork Games MIT).
//
// The 4 Intersects(CircleF, BoundingRectangle) overloads (static ref, static value, instance
// ref, instance value) are deferred: they call BoundingRectangle::SquaredDistanceTo, which is
// itself deferred pending PrimitivesHelper (see BoundingRectangle.hpp). CircleF-vs-CircleF
// Intersects, Contains, ClosestPointTo, BoundaryPointAt, and the Rectangle/RectangleF
// conversions are all self-contained and fully ported.
//
// IEquatable<CircleF>/IEquatableByRef<CircleF> are not implemented as C++ interfaces (matching
// the precedent set by the bounding-volume types) -- just a plain Equals(const CircleF&).
#pragma once

#include "CNA/Extended/RectangleF.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>
#include <vector>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;

    class BoundingRectangle;

    /** @brief A two dimensional circle defined by a centre Vector2 and a radius float. */
    struct CircleF
    {
        /** @brief The centre position of this CircleF. */
        Vector2 Center;

        /** @brief The distance from the Center point to any point on the boundary of this CircleF. */
        float Radius = 0.0f;

        CircleF() = default;

        /**
         * @brief Initializes a new CircleF from the specified centre and radius.
         * @param center The centre point.
         * @param radius The radius.
         */
        CircleF(const Vector2& center, float radius);

        /** @brief Explicitly converts a Rectangle to a CircleF (matches upstream's explicit operator). */
        explicit CircleF(const Rectangle& rectangle);

        /** @brief Explicitly converts a RectangleF to a CircleF (matches upstream's explicit operator). */
        explicit CircleF(const RectangleF& rectangle);

        /** @brief Gets or sets the position of the circle (an alias for Center). */
        [[nodiscard]] Vector2 getPositionProperty() const { return Center; }
        void setPositionProperty(const Vector2& value) { Center = value; }

        /** @brief Gets the axis-aligned bounding rectangle of this circle. */
        [[nodiscard]] RectangleF getBoundingRectangleProperty() const;

        /** @brief Gets the distance from a point to the opposite point, both on the boundary of this circle. */
        [[nodiscard]] float getDiameterProperty() const { return 2.0f * Radius; }

        /** @brief Gets the distance around the boundary of this circle. */
        [[nodiscard]] float getCircumferenceProperty() const;

        /**
         * @brief Computes the bounding CircleF from a minimum and maximum Vector2.
         * @param result Receives the resulting circle.
         */
        static void CreateFrom(const Vector2& minimum, const Vector2& maximum, CircleF& result);

        /** @brief Computes the bounding CircleF from a minimum and maximum Vector2. */
        [[nodiscard]] static CircleF CreateFrom(const Vector2& minimum, const Vector2& maximum);

        /**
         * @brief Computes the bounding CircleF from a list of points.
         * @param result Receives the resulting circle.
         */
        static void CreateFrom(const std::vector<Vector2>& points, CircleF& result);

        /** @brief Computes the bounding CircleF from a list of points. */
        [[nodiscard]] static CircleF CreateFrom(const std::vector<Vector2>& points);

        /** @brief Determines whether the two specified CircleF structures intersect. */
        [[nodiscard]] static bool Intersects(const CircleF& first, const CircleF& second);

        /** @brief Determines whether the specified CircleF intersects with this one. */
        [[nodiscard]] bool Intersects(const CircleF& circle) const;

        /** @brief Determines whether the specified CircleF contains the specified Vector2. */
        [[nodiscard]] static bool Contains(const CircleF& circle, const Vector2& point);

        /** @brief Determines whether this CircleF contains the specified Vector2. */
        [[nodiscard]] bool Contains(const Vector2& point) const;

        /** @brief Computes the closest Vector2 on this CircleF to a specified Vector2. */
        [[nodiscard]] Vector2 ClosestPointTo(const Vector2& point) const;

        /**
         * @brief Computes the Vector2 on the boundary of this CircleF at the specified angle.
         * @param angle The angle in radians.
         */
        [[nodiscard]] Vector2 BoundaryPointAt(float angle) const;

        [[nodiscard]] bool Equals(const CircleF& circle) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const CircleF& first, const CircleF& second) { return first.Equals(second); }
        friend bool operator!=(const CircleF& first, const CircleF& second) { return !first.Equals(second); }

        /** @brief Explicitly converts this CircleF to a Rectangle. */
        [[nodiscard]] explicit operator Rectangle() const;
        /** @brief Converts this CircleF to a Rectangle. */
        [[nodiscard]] Rectangle ToRectangle() const { return static_cast<Rectangle>(*this); }

        /** @brief Explicitly converts this CircleF to a RectangleF. */
        [[nodiscard]] explicit operator RectangleF() const;
        /** @brief Converts this CircleF to a RectangleF. */
        [[nodiscard]] RectangleF ToRectangleF() const { return static_cast<RectangleF>(*this); }
    };
}
