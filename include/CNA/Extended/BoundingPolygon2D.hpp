// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's BoundingPolygon2D.cs. Contains(...)/Intersects(...)/
// TryGetCollision(...) overloads are deferred until Collision2D is ported (Phase 2) -- see the
// header comment in BoundingBox2D.hpp for the full rationale, which applies identically here.
//
// The private ComputeSignedArea helper calls Vector2Extensions.PerpDot upstream (Math/
// Vector2Extensions.cs, ported later in this phase). PerpDot is a single-line, permanently-fixed
// formula (a.X*b.Y - a.Y*b.X); it is inlined directly in BoundingPolygon2D.cpp rather than
// deferring ComputeSignedArea's callers (the constructor's winding-order check and CreateMerged),
// matching the same "duplicate the trivial, defer the substantial" line drawn for
// Collision2D::Epsilon elsewhere in this task -- unlike BoundingCapsule2D's LineSegment2D::
// DistanceToPoint dependency, PerpDot is not a multi-step algorithm.
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

    /** @brief Represents a convex polygon bounding volume in 2D space. */
    struct BoundingPolygon2D
    {
        /**
         * @brief The vertices of this polygon in counter-clockwise order in 2D space. The
         * counter-clockwise winding order is used to derive outward-facing edge normals for
         * collision detection.
         */
        std::vector<Vector2> Vertices;

        /**
         * @brief The outward-facing unit normals for each edge of this polygon, precomputed for
         * collision detection. Each normal at index i corresponds to the edge from Vertices[i]
         * to Vertices[(i + 1) % Vertices.size()].
         */
        std::vector<Vector2> Normals;

        BoundingPolygon2D() = default;

        /**
         * @brief Creates a new BoundingPolygon2D with the specified vertices. Edge normals are
         * computed automatically from the vertices and cached for collision detection.
         * @param vertices The vertices of the polygon in counter-clockwise order. Must contain
         * at least three vertices.
         */
        explicit BoundingPolygon2D(std::vector<Vector2> vertices);

        /**
         * @brief Creates a new BoundingPolygon2D with the specified vertices and precomputed
         * edge normals, avoiding recomputation when the data is already available.
         * @param vertices The vertices of the polygon in counter-clockwise order. Must contain
         * at least three vertices.
         * @param normals The outward-facing unit normals for each polygon edge. Must have the
         * same length as vertices.
         */
        BoundingPolygon2D(std::vector<Vector2> vertices, std::vector<Vector2> normals);

        /** @brief Gets the number of vertices in this polygon. */
        [[nodiscard]] int getVertexCountProperty() const { return static_cast<int>(Vertices.size()); }

        /**
         * @brief Gets the geometric centroid of this polygon in 2D space, computed as the
         * arithmetic mean of all vertex positions. Does not account for mass distribution.
         */
        [[nodiscard]] Vector2 getCentroidProperty() const;

        /** @brief Gets the area enclosed by this polygon. */
        [[nodiscard]] float getAreaProperty() const;

        /**
         * @brief Creates a new BoundingPolygon2D from the specified vertices, with edge normals
         * computed automatically.
         * @param vertices The vertices of the polygon in counter-clockwise order. Must contain
         * at least three vertices.
         */
        [[nodiscard]] static BoundingPolygon2D CreateFromVertices(std::vector<Vector2> vertices);

        /**
         * @brief Creates a new BoundingPolygon2D representing a regular polygon with equal side
         * lengths and angles, with vertices in counter-clockwise order and edge normals computed
         * automatically.
         * @param center The center position of the polygon in 2D space.
         * @param radius The distance from the center to each vertex.
         * @param sides The number of sides of the polygon. Must be at least three.
         * @param rotation The rotation angle of the polygon in radians, measured
         * counter-clockwise from the positive world X-axis.
         */
        [[nodiscard]] static BoundingPolygon2D CreateRegular(const Vector2& center, float radius, int sides, float rotation = 0.0f);

        /**
         * @brief Creates a new BoundingPolygon2D from a bounding box, with four vertices in
         * counter-clockwise order representing the bounding box as a rectangular polygon.
         * @param box The bounding box to convert to a polygon.
         */
        [[nodiscard]] static BoundingPolygon2D CreateFromBoundingBox2D(const BoundingBox2D& box);

        /**
         * @brief Creates a BoundingPolygon2D that encloses two polygons, by computing the convex
         * hull of the combined vertex sets from both polygons.
         * @param original The first polygon to enclose.
         * @param additional The second polygon to enclose.
         */
        [[nodiscard]] static BoundingPolygon2D CreateMerged(const BoundingPolygon2D& original, const BoundingPolygon2D& additional);

        /**
         * @brief Applies a matrix transformation to this polygon and creates a new transformed
         * polygon, with edge normals recomputed from the transformed geometry.
         * @param matrix The transformation matrix to apply.
         */
        [[nodiscard]] BoundingPolygon2D Transform(const Matrix& matrix) const;

        /**
         * @brief Creates a new BoundingPolygon2D by translating this polygon by the specified
         * offset. Normals don't change with translation.
         * @param translation The offset to translate the polygon by in 2D space.
         */
        [[nodiscard]] BoundingPolygon2D Translate(const Vector2& translation) const;

        /**
         * @brief Deconstructs this polygon into its component arrays.
         * @param vertices Receives the vertices.
         * @param normals Receives the edge normals.
         */
        void Deconstruct(std::vector<Vector2>& vertices, std::vector<Vector2>& normals) const;

        [[nodiscard]] bool Equals(const BoundingPolygon2D& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const BoundingPolygon2D& left, const BoundingPolygon2D& right) { return left.Equals(right); }
        friend bool operator!=(const BoundingPolygon2D& left, const BoundingPolygon2D& right) { return !left.Equals(right); }

    private:
        static float ComputeSignedArea(const std::vector<Vector2>& vertices);
        static void EnsureCounterClockwise(std::vector<Vector2>& vertices);
        static std::vector<Vector2> ComputeConvexHull(std::vector<Vector2> points);
        static float Orientation(const Vector2& a, const Vector2& b, const Vector2& c);
        static std::vector<Vector2> ComputeNormals(const std::vector<Vector2>& vertices);
    };
}
