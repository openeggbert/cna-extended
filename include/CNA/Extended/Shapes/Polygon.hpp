// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Shapes/Polygon.cs. Upstream lives in the
// MonoGame.Extended.Shapes sub-namespace (unlike the Math/ folder types ported so far, which are
// all in the root MonoGame.Extended namespace) -- mirrored here as CNA::Extended::Shapes, the
// first sub-namespace used in this phase. Newly discovered while scoping "PrimitivesHelper,
// ShapeExtensions" (Phase 1 task 13): this Shapes/ folder was never tracked in plan.md's task
// list at all; added here since Polygon/Polyline turned out to be small and self-contained.
#pragma once

#include "CNA/Extended/RectangleF.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <vector>

namespace CNA::Extended::Shapes
{
    using Microsoft::Xna::Framework::Vector2;
    using CNA::Extended::RectangleF;

    /**
     * @brief Represents a polygon defined by a set of vertices, supporting deferred offset/
     * rotation/scale transforms.
     */
    class Polygon
    {
    public:
        /**
         * @brief Initializes a new Polygon with the specified local-space vertices.
         * @param vertices The vertices of the polygon, in local space.
         */
        explicit Polygon(std::vector<Vector2> vertices);

        /**
         * @brief Gets the (possibly transformed) vertices of this polygon. Recomputes the
         * transformed vertices lazily if Offset/Rotate/Scale have been called since the last
         * access.
         */
        [[nodiscard]] const std::vector<Vector2>& getVerticesProperty() const;

        /** @brief Gets the minimum X coordinate among this polygon's vertices. */
        [[nodiscard]] float getLeftProperty() const;

        /** @brief Gets the maximum X coordinate among this polygon's vertices. */
        [[nodiscard]] float getRightProperty() const;

        /** @brief Gets the minimum Y coordinate among this polygon's vertices. */
        [[nodiscard]] float getTopProperty() const;

        /** @brief Gets the maximum Y coordinate among this polygon's vertices. */
        [[nodiscard]] float getBottomProperty() const;

        /** @brief Gets the axis-aligned bounding rectangle of this polygon. */
        [[nodiscard]] RectangleF getBoundingRectangleProperty() const;

        /**
         * @brief Offsets this polygon's vertices by the specified amount (accumulates with any
         * prior offset).
         */
        void Offset(const Vector2& amount);

        /**
         * @brief Rotates this polygon's vertices by the specified amount, in radians (accumulates
         * with any prior rotation).
         */
        void Rotate(float amount);

        /**
         * @brief Scales this polygon's vertices by the specified amount (accumulates with any
         * prior scale, matching upstream's `_scale += amount` accumulation -- not a multiplicative
         * scale).
         */
        void Scale(const Vector2& amount);

        /**
         * @brief Creates a new Polygon from this polygon's local-space vertices, transformed by
         * the specified offset, rotation, and scale.
         */
        [[nodiscard]] Polygon TransformedCopy(const Vector2& offset, float rotation, const Vector2& scale) const;

        /** @brief Determines whether this polygon contains the specified point, using the even-odd rule. */
        [[nodiscard]] bool Contains(const Vector2& point) const;

        /** @brief Determines whether this polygon contains the specified point, using the even-odd rule. */
        [[nodiscard]] bool Contains(float x, float y) const;

        [[nodiscard]] bool Equals(const Polygon& other) const;
        [[nodiscard]] int GetHashCode() const;

        friend bool operator==(const Polygon& a, const Polygon& b) { return a.Equals(b); }
        friend bool operator!=(const Polygon& a, const Polygon& b) { return !a.Equals(b); }

    private:
        [[nodiscard]] std::vector<Vector2> GetTransformedVertices() const;

        std::vector<Vector2> localVertices_;
        mutable std::vector<Vector2> transformedVertices_;
        Vector2 offset_ = Vector2::Zero;
        float rotation_ = 0.0f;
        Vector2 scale_ = Vector2::One;
        mutable bool isDirty_ = false;
    };
}
