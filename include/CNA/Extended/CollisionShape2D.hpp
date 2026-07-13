// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's CollisionShape2D.cs: a tagged-union `readonly struct` wrapping
// one of 5 bounding-volume kinds (see CollisionShapeKind2D), dispatching Intersects/TryGetCollision
// calls via nested switch statements to the corresponding bounding-volume type's own instance
// methods -- all of which now exist (Collision2D and the full bounding-volume follow-up sweep are
// complete; see NEXT.md entries (19)-(23)).
//
// Upstream stores only BoundingBox2D directly as a field (`_boundingBox`, doubling as this shape's
// cached broadphase bounds) plus a small set of scalar/Vector2 "slots" (`_primary`/`_secondary`/
// `_tertiary`/`_scalar`) reused differently per CollisionShapeKind2D, plus `_polygonVertices`/
// `_polygonNormals` (reference-backed, not copied, per upstream's own documented design choice --
// ported as std::vector<Vector2> copies here, since C++ value semantics for std::vector already
// avoid the "copying arbitrary-size arrays into a value type" concern upstream's comment is about;
// unlike C#, there is no reference-type array to alias). The private computed `Circle`/`OrientedBox`/
// `Capsule`/`Polygon` C# properties (each reconstructing a fresh instance from the stored slots) are
// ported as private getXProperty() methods, matching this project's established C# property
// convention regardless of visibility.
//
// `internal bool TryGetLegacyPenetrationVector(...)` -- upstream's `internal` visibility has no C++
// equivalent; kept public, matching the precedent set by CollisionResult2D::Invert() and
// CollisionShapeKind2D elsewhere in this port. The `private static` legacy-penetration helper
// methods (GetLegacyCircleCirclePenetrationVector/GetLegacyCircleBoxPenetrationVector/
// GetLegacyBoxBoxPenetrationVector/Contains/GetClosestPoint) are ported as genuine C++ `private
// static` member functions.
//
// No upstream test file exists for this type (confirmed: no CollisionShape2DTest.cs anywhere under
// MonoGame.Extended.Tests) -- CollisionShape2DTests.cpp below is fresh test coverage, following the
// "spot-check pair per delegation branch" convention used throughout this session's Collision2D
// follow-up work, plus direct coverage of the legacy penetration vector helpers.
#pragma once

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/CollisionResult2D.hpp"
#include "CNA/Extended/CollisionShapeKind2D.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <optional>
#include <vector>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    struct BoundingCircle2D;
    struct OrientedBoundingBox2D;
    struct BoundingCapsule2D;
    struct BoundingPolygon2D;
    struct Ray2D;
    struct Line2D;
    struct LineSegment2D;

    /**
     * @brief Represents a collision shape for two-dimensional intersection and collision queries.
     * @remark The default value represents a None shape and does not intersect any other shape.
     * Convex polygon support is limited to polygons that satisfy the requirements of
     * BoundingPolygon2D, including convexity and consistent winding for vertices and normals.
     */
    class CollisionShape2D
    {
    public:
        CollisionShape2D() = default;

        /**
         * @brief Creates a collision shape from an axis-aligned bounding box.
         * @param box The axis-aligned bounding box to represent.
         * @remark The supplied box is stored directly and also serves as the broadphase bounds
         * for this shape.
         */
        explicit CollisionShape2D(const BoundingBox2D& box);

        /**
         * @brief Creates a collision shape from a bounding circle.
         * @param circle The bounding circle to represent.
         * @remark The broadphase bounds are cached from the circle center and radius at
         * construction time.
         */
        explicit CollisionShape2D(const BoundingCircle2D& circle);

        /**
         * @brief Creates a collision shape from an oriented bounding box.
         * @param box The oriented bounding box to represent.
         * @remark The broadphase bounds are cached from the oriented box corners at construction
         * time. The stored orientation assumes a valid orthonormal basis and derives the Y axis
         * from the X axis.
         */
        explicit CollisionShape2D(const OrientedBoundingBox2D& box);

        /**
         * @brief Creates a collision shape from a bounding capsule.
         * @param capsule The bounding capsule to represent.
         * @remark The broadphase bounds are cached from the segment endpoints expanded by the
         * capsule radius.
         */
        explicit CollisionShape2D(const BoundingCapsule2D& capsule);

        /**
         * @brief Creates a collision shape from a convex bounding polygon.
         * @param polygon The convex bounding polygon to represent.
         * @remark The polygon is expected to satisfy the requirements of BoundingPolygon2D,
         * including convexity and counter-clockwise winding.
         */
        explicit CollisionShape2D(const BoundingPolygon2D& polygon);

        /** @brief Gets the axis-aligned broadphase bounds for this collision shape. */
        [[nodiscard]] BoundingBox2D getBoundingBoxProperty() const { return boundingBox_; }

        /**
         * @brief Determines whether this collision shape intersects with another collision shape.
         * @param other The other collision shape to test against.
         * @return true if the shapes overlap or touch; otherwise, false.
         * @remark This method delegates to the existing bounding volume intersection APIs. If
         * either shape is the default None shape, this method returns false.
         */
        [[nodiscard]] bool Intersects(const CollisionShape2D& other) const;

        /**
         * @brief Determines whether this collision shape is intersected by the specified ray.
         * @param ray The ray to test against.
         * @return true if the ray intersects the shape in its forward direction; otherwise, false.
         * @remark This method delegates to the existing ray-shape intersection APIs. If this
         * shape is None, it returns false.
         */
        [[nodiscard]] bool Intersects(const Ray2D& ray) const;

        /**
         * @brief Determines whether this collision shape is intersected by the specified ray and
         * computes the parametric distances to the entry and exit intersection points.
         * @param ray The ray to test against.
         * @param tMin Receives the parametric distance along the ray to the entry intersection
         * point. If the ray origin lies inside the shape, this value is 0. When this method
         * returns false, contains 0.0f.
         * @param tMax Receives the parametric distance along the ray to the exit intersection
         * point (always >= tMin). When this method returns false, contains 0.0f.
         * @return true if the ray intersects the shape in its forward direction; otherwise, false.
         * @remark This method delegates to the existing ray-shape intersection APIs. If this
         * shape is None, it returns false and the out parameters are left at their initial value
         * of 0.0f. Note: unlike Intersects(CollisionShape2D)/Intersects(Line2D)/
         * Intersects(LineSegment2D), this overload does not support the Polygon kind -- matching
         * upstream, which has no Ray2D::Intersects(BoundingPolygon2D, out, out) overload at all.
         */
        [[nodiscard]] bool Intersects(const Ray2D& ray, std::optional<float>& tMin, std::optional<float>& tMax) const;

        /**
         * @brief Determines whether this collision shape is intersected by the specified infinite
         * line.
         * @param line The line to test against.
         * @return true if the line passes through or touches the shape; otherwise, false.
         * @remark This method delegates to the existing line-shape intersection APIs. If this
         * shape is None, it returns false.
         */
        [[nodiscard]] bool Intersects(const Line2D& line) const;

        /**
         * @brief Determines whether this collision shape is intersected by the specified line
         * segment.
         * @param lineSegment The line segment to test against.
         * @return true if the segment intersects the shape; otherwise, false.
         * @remark This method delegates to the existing segment-shape intersection APIs. If this
         * shape is None, it returns false.
         */
        [[nodiscard]] bool Intersects(const LineSegment2D& lineSegment) const;

        /**
         * @brief Determines whether this collision shape is intersected by the specified line
         * segment and computes the parametric distances to the entry and exit intersection points.
         * @param lineSegment The line segment to test against.
         * @param tMin Receives the parametric distance along the segment to the entry
         * intersection point, in [0, 1]. When this method returns false, contains 0.0f.
         * @param tMax Receives the parametric distance along the segment to the exit intersection
         * point, in [0, 1]. When this method returns false, contains 0.0f.
         * @return true if the segment intersects the shape; otherwise, false.
         * @remark This method delegates to the existing segment-shape intersection APIs. If this
         * shape is None, it returns false and the out parameters are left at their initial value
         * of 0.0f. For degenerate (zero-length) segments, returns true with tMin = tMax = 0 if the
         * start point lies inside the shape. Note: like Intersects(Ray2D, out, out), this overload
         * does not support the Polygon kind, matching upstream.
         */
        [[nodiscard]] bool Intersects(const LineSegment2D& lineSegment, std::optional<float>& tMin, std::optional<float>& tMax) const;

        /**
         * @brief Tests whether this collision shape intersects with another collision shape, and
         * returns collision resolution data when the represented shape pair supports it.
         * @param other The other collision shape to test against.
         * @param result Receives the collision result whose minimum translation vector moves this
         * shape out of other when this method returns true; otherwise CollisionResult2D::None.
         * @return true if the shapes overlap or touch and the represented pair supports
         * collision-result queries; otherwise, false.
         * @remark This method does not add new collision-resolution algorithms; it delegates only
         * to existing shape-pair APIs that already return CollisionResult2D. If either shape is
         * the default None shape, or if the represented pair does not support collision-result
         * queries, this method returns false and sets result to CollisionResult2D::None.
         */
        [[nodiscard]] bool TryGetCollision(const CollisionShape2D& other, CollisionResult2D& result) const;

        /**
         * @brief Computes a legacy-style, non-normalized penetration vector for a Circle/Box
         * shape pair, matching MonoGame.Extended's older (pre-CollisionResult2D) collision API.
         * @param other The other collision shape to test against.
         * @param penetrationVector Receives the penetration vector that separates this shape from
         * other when this method returns true; otherwise Vector2::Zero.
         * @return true if the represented pair (Circle/Circle, Circle/Box, Box/Circle, or
         * Box/Box) supports this legacy query; otherwise, false.
         * @remark Upstream declares this `internal`; C++ has no assembly-level visibility
         * equivalent, so it is public here, documented as an implementation detail not part of
         * this type's stable public API.
         */
        [[nodiscard]] bool TryGetLegacyPenetrationVector(const CollisionShape2D& other, Vector2& penetrationVector) const;

    private:
        [[nodiscard]] BoundingCircle2D getCircleProperty() const;
        [[nodiscard]] OrientedBoundingBox2D getOrientedBoxProperty() const;
        [[nodiscard]] BoundingCapsule2D getCapsuleProperty() const;
        [[nodiscard]] BoundingPolygon2D getPolygonProperty() const;

        [[nodiscard]] static Vector2 GetLegacyCircleCirclePenetrationVector(const BoundingCircle2D& first, const BoundingCircle2D& second);
        [[nodiscard]] static Vector2 GetLegacyCircleBoxPenetrationVector(const BoundingCircle2D& circle, const BoundingBox2D& box);
        [[nodiscard]] static Vector2 GetLegacyBoxBoxPenetrationVector(const BoundingBox2D& first, const BoundingBox2D& second);
        [[nodiscard]] static bool Contains(const BoundingBox2D& box, const Vector2& point);
        [[nodiscard]] static Vector2 GetClosestPoint(const BoundingBox2D& box, const Vector2& point);

        CollisionShapeKind2D kind_ = CollisionShapeKind2D::None;
        BoundingBox2D boundingBox_;
        Vector2 primary_ = Vector2::Zero;
        Vector2 secondary_ = Vector2::Zero;
        Vector2 tertiary_ = Vector2::Zero;
        float scalar_ = 0.0f;
        std::vector<Vector2> polygonVertices_;
        std::vector<Vector2> polygonNormals_;
    };
}
