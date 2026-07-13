// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collision2D.cs: a static library of 2D collision-detection
// primitives (containment, projection, distance, clipping, and intersection tests across
// AABB/Circle/OBB/Capsule/ConvexPolygon shape pairs). This is the largest single file in this
// port (3,809 lines upstream) and is being ported in several sequential chunks, each appending
// to this same file -- see plan.md/NEXT.md for the chunk breakdown.
//
// Chunk 1 covered: Constants, Helpers, the full Containment region (all 5 shape-pair
// sub-regions), and every dependency those methods transitively require to be genuinely
// self-sufficient and correct -- which, in this tightly-coupled file, turned out to mean nearly
// everything except the Parametric Solvers region, the Ray Interval methods, and the
// `TryGetCollision*` overloads. Specifically it included: Constants/Helpers/Containment; the
// full Projection region; the full Distance/ClosestPoint region; `ClipLineToAabb`/
// `ClipLineToConvexPolygon` from the Clipping & Ray Intervals region (needed by the Distance
// region, not the Ray Interval methods in that same region); and every plain-bool `Intersects*`
// method from the Intersections region. See NEXT.md for the full account of why the
// originally-planned narrower scope wasn't achievable in isolation.
//
// Chunk 2 (this one) completes the file: the Parametric Solvers region
// (`SolveParametricIntersectionWithImplicitLine`, `SolveParametricIntersection2D`);
// `ClosestPointRaySegment` (a leaf dependency of `RayCapsuleIntersectionInterval` below -- its
// only caller, confirmed by reading the whole file) and the two Ray Interval methods
// (`RayCircleIntersectionInterval`, `RayCapsuleIntersectionInterval`); and all 10
// `CollisionResult2D`-producing `TryGetCollision*` overloads, each pairing with an
// already-ported plain-`bool` `Intersects*` sibling from chunk 1. `Collision2D.cs` is now
// 100% ported.
//
// `Vector2[]` array parameters -> `const std::vector<Vector2>&`, matching every other C#
// array/`IList<T>` translation in this project. C#'s `out` parameters -> C++ reference
// out-params. C#'s `ref` parameters (one `ClipInterval` overload) -> ordinary non-const C++
// references.
#pragma once

#include "CNA/Extended/CollisionResult2D.hpp"
#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <vector>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::ContainmentType;
    using Microsoft::Xna::Framework::Vector2;

    /**
     * @brief Provides low-level, allocation-free geometric queries and helper routines for 2D
     * collision detection.
     *
     * Stateless, static methods for intersection tests, containment classification, distance
     * queries, interval clipping, and projection operations between common 2D primitives (points,
     * line segments, circles, AABBs, OBBs, capsules, and convex polygons). Intended as shared
     * building blocks for higher-level geometric types (bounding volumes and shapes) rather than
     * for direct use in most application code.
     */
    class Collision2D
    {
    public:
        Collision2D() = delete;

        // ---- Constants ----

        /** @brief A small tolerance value used to account for floating-point imprecision in geometric computations. */
        static constexpr float Epsilon = 1e-6f;

        /** @brief The square of Epsilon, provided for efficiency when comparing squared distances. */
        static constexpr float EpsilonSq = Epsilon * Epsilon;

        // ---- Helpers ----

        /**
         * @brief Determines whether the specified convex polygon data is structurally valid for
         * use by collision routines (at least 3 vertices, and @p normals the same length).
         */
        [[nodiscard]] static bool IsValidPolygon(const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals);

        /**
         * @brief Intersects a parametric interval with a clipping interval and updates the result
         * in place.
         * @return true if the two intervals overlap after clipping.
         */
        static bool ClipInterval(float& tMin, float& tMax, float clipMin, float clipMax);

        /**
         * @brief Computes the intersection of two parametric intervals and returns the clipped
         * result without modifying the input parameters.
         * @return true if the intervals overlap and the resulting interval is non-empty.
         */
        static bool ClipInterval(float tEnter, float tExit, float tLower, float tUpper, float& clippedEnter, float& clippedExit);

        /** @brief Determines whether two scalar intervals overlap or touch within Epsilon (touching counts as overlap). */
        [[nodiscard]] static bool IntervalsOverlap(float minA, float maxA, float minB, float maxB);

        /**
         * @brief Computes the scalar overlap between two projected intervals for separating-axis
         * tests. Touching intervals return true with an overlap of 0.
         */
        static bool TryGetProjectionOverlap(float minA, float maxA, float minB, float maxB, float& overlap);

        /**
         * @brief Updates the smallest overlap tracked during a separating-axis test.
         * @return true if the tracked overlap was updated.
         */
        static bool UpdateMinimumOverlap(float overlap, const Vector2& axis, float& minimumOverlap, Vector2& minimumOverlapAxis);

        /** @brief Orients a collision normal so it points from the second shape toward the first shape. */
        [[nodiscard]] static Vector2 OrientNormal(const Vector2& normal, const Vector2& centerA, const Vector2& centerB);

        // ---- Containment (AABB Contains) ----

        [[nodiscard]] static ContainmentType ContainsAabbPoint(const Vector2& point, const Vector2& min, const Vector2& max);
        [[nodiscard]] static ContainmentType ContainsAabbAabb(const Vector2& aMin, const Vector2& aMax, const Vector2& bMin, const Vector2& bMax);
        [[nodiscard]] static ContainmentType ContainsAabbCircle(
            const Vector2& boxMin, const Vector2& boxMax, const Vector2& circleCenter, float circleRadius);
        [[nodiscard]] static ContainmentType ContainsAabbObb(const Vector2& aabbMin, const Vector2& aabbMax, const Vector2& obbCenter,
            const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalfExtents);
        [[nodiscard]] static ContainmentType ContainsAabbCapsule(
            const Vector2& boxMin, const Vector2& boxMax, const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius);
        [[nodiscard]] static ContainmentType ContainsAabbConvexPolygon(
            const Vector2& boxMin, const Vector2& boxMax, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals);

        // ---- Containment (Circle Contains) ----

        [[nodiscard]] static ContainmentType ContainsCirclePoint(const Vector2& point, const Vector2& center, float radius);
        [[nodiscard]] static ContainmentType ContainsCircleAabb(
            const Vector2& circleCenter, float circleRadius, const Vector2& aabbMin, const Vector2& aabbMax);
        [[nodiscard]] static ContainmentType ContainsCircleCircle(const Vector2& aCenter, float aRadius, const Vector2& bCenter, float bRadius);
        [[nodiscard]] static ContainmentType ContainsCircleObb(const Vector2& circleCenter, float circleRadius, const Vector2& obbCenter,
            const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalfExtents);
        [[nodiscard]] static ContainmentType ContainsCircleCapsule(
            const Vector2& circleCenter, float circleRadius, const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius);
        [[nodiscard]] static ContainmentType ContainsCircleConvexPolygon(
            const Vector2& circleCenter, float circleRadius, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals);

        // ---- Containment (OBB Contains) ----

        [[nodiscard]] static ContainmentType ContainsObbPoint(
            const Vector2& point, const Vector2& center, const Vector2& axisX, const Vector2& axisY, const Vector2& halfExtents);
        [[nodiscard]] static ContainmentType ContainsObbAabb(const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY,
            const Vector2& obbHalfExtents, const Vector2& aabbMin, const Vector2& aabbMax);
        [[nodiscard]] static ContainmentType ContainsObbCircle(const Vector2& boxCenter, const Vector2& axisX, const Vector2& axisY,
            const Vector2& halfExtents, const Vector2& circleCenter, float circleRadius);
        [[nodiscard]] static ContainmentType ContainsObbObb(const Vector2& aCenter, const Vector2& aAxisX, const Vector2& aAxisY,
            const Vector2& aHalf, const Vector2& bCenter, const Vector2& bAxisX, const Vector2& bAxisY, const Vector2& bHalf);
        [[nodiscard]] static ContainmentType ContainsObbCapsule(const Vector2& boxCenter, const Vector2& axisX, const Vector2& axisY,
            const Vector2& halfExtents, const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius);
        [[nodiscard]] static ContainmentType ContainsObbConvexPolygon(const Vector2& boxCenter, const Vector2& axisX, const Vector2& axisY,
            const Vector2& halfExtents, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals);

        // ---- Containment (Capsule Contains) ----

        [[nodiscard]] static ContainmentType ContainsCapsulePoint(const Vector2& point, const Vector2& a, const Vector2& b, float radius);
        [[nodiscard]] static ContainmentType ContainsCapsuleAabb(
            const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius, const Vector2& aabbMin, const Vector2& aabbMax);
        [[nodiscard]] static ContainmentType ContainsCapsuleCircle(
            const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius, const Vector2& circleCenter, float circleRadius);
        [[nodiscard]] static ContainmentType ContainsCapsuleObb(const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius,
            const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalf);
        [[nodiscard]] static ContainmentType ContainsCapsuleCapsule(
            const Vector2& a0, const Vector2& a1, float aRadius, const Vector2& b0, const Vector2& b1, float bRadius);
        [[nodiscard]] static ContainmentType ContainsCapsuleConvexPolygon(const Vector2& capsuleA, const Vector2& capsuleB,
            float capsuleRadius, const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals);

        // ---- Containment (Convex Polygon Contains) ----

        [[nodiscard]] static ContainmentType ContainsConvexPolygonPoint(
            const Vector2& point, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals);
        [[nodiscard]] static ContainmentType ContainsConvexPolygonAabb(
            const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals, const Vector2& aabbMin, const Vector2& aabbMax);
        [[nodiscard]] static ContainmentType ContainsConvexPolygonCircle(
            const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals, const Vector2& circleCenter, float circleRadius);
        [[nodiscard]] static ContainmentType ContainsConvexPolygonCapsule(const std::vector<Vector2>& pVertices,
            const std::vector<Vector2>& pNormals, const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius);
        [[nodiscard]] static ContainmentType ContainsConvexPolygonObb(const std::vector<Vector2>& pVertices,
            const std::vector<Vector2>& pNormals, const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY,
            const Vector2& obbHalf);
        [[nodiscard]] static ContainmentType ContainsConvexPolygonConvexPolygon(const std::vector<Vector2>& aVertices,
            const std::vector<Vector2>& aNormals, const std::vector<Vector2>& bVertices, const std::vector<Vector2>& bNormals);

        // ---- Projection ----

        /** @brief Projects a set of points onto an axis and returns the resulting scalar interval. */
        static void ProjectOntoAxis(const std::vector<Vector2>& vertices, const Vector2& axis, float& min, float& max);

        /** @brief Projects an AABB (given as center/half-extents) onto an axis and returns the resulting scalar interval. */
        static void ProjectAabbOntoAxis(const Vector2& center, const Vector2& halfExtents, const Vector2& axis, float& min, float& max);

        /** @brief Projects an OBB onto an axis and returns the resulting scalar interval. */
        static void ProjectObbOntoAxis(
            const Vector2& center, const Vector2& axisX, const Vector2& axisY, const Vector2& halfExtents, const Vector2& axis, float& min, float& max);

        /** @brief Determines whether the projections of two point sets overlap on an axis. */
        [[nodiscard]] static bool OverlapOnAxis(const std::vector<Vector2>& aVerts, const std::vector<Vector2>& bVerts, const Vector2& axis);

        /** @brief Determines whether the projections of an AABB (center/half-extents) and a point set overlap on an axis. */
        [[nodiscard]] static bool OverlapOnAxis(
            const Vector2& aabbCenter, const Vector2& aabbHalfExtents, const std::vector<Vector2>& polygonVertices, const Vector2& axis);

        // ---- Distance / Closest Point ----

        /** @brief Computes the squared distance from a point to a line segment, and the closest point on the segment. */
        [[nodiscard]] static float DistanceSquaredPointSegment(const Vector2& point, const Vector2& a, const Vector2& b, float& t, Vector2& closestPoint);

        /** @brief Computes the squared distance between two line segments, and the closest points on each. */
        [[nodiscard]] static float DistanceSquaredSegmentSegment(
            const Vector2& p1, const Vector2& q1, const Vector2& p2, const Vector2& q2, float& s, float& t, Vector2& c1, Vector2& c2);

        /** @brief Computes the squared distance from a point to an AABB. */
        [[nodiscard]] static float DistanceSquaredPointAabb(const Vector2& point, const Vector2& min, const Vector2& max);

        /** @brief Computes the squared distance from a point to an OBB. */
        [[nodiscard]] static float DistanceSquaredPointObb(
            const Vector2& point, const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalfExtents);

        /** @brief Computes the squared distance from a point to a convex polygon (0 if the point is inside). */
        [[nodiscard]] static float DistanceSquaredPointConvexPolygon(
            const Vector2& p, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals);

        /** @brief Computes the squared distance between a line segment and an AABB (0 if they intersect). */
        [[nodiscard]] static float DistanceSquaredSegmentAabb(const Vector2& a, const Vector2& b, const Vector2& min, const Vector2& max);

        /** @brief Computes the squared distance between a line segment and a convex polygon (0 if they intersect). */
        [[nodiscard]] static float DistanceSquaredSegmentConvexPolygon(
            const Vector2& a, const Vector2& b, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals);

        // ---- Clipping (subset needed by the Distance region; Ray Interval methods and the remaining
        // Clipping-region members are deferred to a later chunk) ----

        /**
         * @brief Clips a parametric line against an AABB, restricting the result to
         * [tLower, tUpper]. The 2D slab test (Ericson-style).
         */
        static bool ClipLineToAabb(const Vector2& origin, const Vector2& direction, const Vector2& min, const Vector2& max, float tLower,
            float tUpper, float& tEnter, float& tExit);

        /** @brief Clips a parametric line against a convex polygon expressed as an intersection of half-spaces (Cyrus-Beck clipping). */
        static bool ClipLineToConvexPolygon(const Vector2& origin, const Vector2& direction, const std::vector<Vector2>& vertices,
            const std::vector<Vector2>& normals, float tLower, float tUpper, float& tEnter, float& tExit);

        // ---- Intersections (bool-only overloads; the CollisionResult2D-producing TryGetCollision*
        // overloads are deferred to a later chunk -- nothing ported so far depends on them) ----

        [[nodiscard]] static bool IntersectsAabbAabb(const Vector2& aMin, const Vector2& aMax, const Vector2& bMin, const Vector2& bMax);
        [[nodiscard]] static bool IntersectsAabbCapsule(
            const Vector2& boxMin, const Vector2& boxMax, const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius);
        [[nodiscard]] static bool IntersectsAabbConvexPolygon(
            const Vector2& aabbCenter, const Vector2& aabbHalfExtents, const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals);
        [[nodiscard]] static bool IntersectsAabbObb(const Vector2& aabbCenter, const Vector2& aabbHalfExtents, const Vector2& obbCenter,
            const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalfExtents);
        [[nodiscard]] static bool IntersectsObbObb(const Vector2& aCenter, const Vector2& aAxisX, const Vector2& aAxisY, const Vector2& aHalf,
            const Vector2& bCenter, const Vector2& bAxisX, const Vector2& bAxisY, const Vector2& bHalf);
        [[nodiscard]] static bool IntersectsObbCapsule(const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY,
            const Vector2& obbHalfExtents, const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius);
        [[nodiscard]] static bool IntersectsObbConvexPolygon(const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY,
            const Vector2& obbHalfExtents, const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals);
        [[nodiscard]] static bool IntersectsCapsuleConvexPolygon(const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius,
            const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals);
        [[nodiscard]] static bool IntersectsCircleCircle(const Vector2& aCenter, float aRadius, const Vector2& bCenter, float bRadius);
        [[nodiscard]] static bool IntersectsCircleAabb(const Vector2& cCenter, float cRadius, const Vector2& boxMin, const Vector2& boxMax);
        [[nodiscard]] static bool IntersectsCircleObb(const Vector2& cCenter, float cRadius, const Vector2& obbCenter, const Vector2& obbAxisX,
            const Vector2& obbAxisY, const Vector2& obbHalfExtents);
        [[nodiscard]] static bool IntersectsCircleConvexPolygon(
            const Vector2& cCenter, float cRadius, const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals);
        [[nodiscard]] static bool IntersectsCircleCapsule(
            const Vector2& circleCenter, float circleRadius, const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius);
        [[nodiscard]] static bool IntersectsCapsuleCapsule(
            const Vector2& a0, const Vector2& a1, float aRadius, const Vector2& b0, const Vector2& b1, float bRadius);
        [[nodiscard]] static bool IntersectsConvexPolygonConvexPolygon(const std::vector<Vector2>& aVertices, const std::vector<Vector2>& aNormals,
            const std::vector<Vector2>& bVertices, const std::vector<Vector2>& bNormals);

        // ---- Parametric Solvers ----

        /**
         * @brief Solves the parametric intersection between an implicit line (dot(lineNormal, x) = lineDistance) and a
         * parametric line/ray/segment (origin + t * direction).
         * @return true if a unique solution exists; false if the parametric line is parallel to the implicit line.
         */
        static bool SolveParametricIntersectionWithImplicitLine(
            const Vector2& lineNormal, float lineDistance, const Vector2& origin, const Vector2& direction, float& t);

        /**
         * @brief Solves the parametric intersection of two 2D lines in point-direction form.
         * @return true if the directions are not parallel; false for parallel/collinear lines (no single-point intersection).
         */
        static bool SolveParametricIntersection2D(
            const Vector2& origin1, const Vector2& direction1, const Vector2& origin2, const Vector2& direction2, float& t1, float& t2);

        // ---- Ray Interval methods (ClosestPointRaySegment is a leaf dependency used only by RayCapsuleIntersectionInterval) ----

        /** @brief Computes the closest points between a ray and a line segment, and the squared distance between them. */
        static void ClosestPointRaySegment(
            const Vector2& rayOrigin, const Vector2& rayDirection, const Vector2& segA, const Vector2& segB, float& sRay, float& tSeg, float& distanceSquared);

        /** @brief Computes the parametric intersection interval between a ray and a circle. */
        static bool RayCircleIntersectionInterval(const Vector2& origin, const Vector2& direction, const Vector2& center, float radius, float& tMin, float& tMax);

        /** @brief Computes the parametric intersection interval between a ray and a capsule. */
        static bool RayCapsuleIntersectionInterval(
            const Vector2& rayOrigin, const Vector2& rayDirection, const Vector2& segA, const Vector2& segB, float radius, float& tMin, float& tMax);

        // ---- Intersections (CollisionResult2D-producing TryGetCollision* overloads) ----

        /** @brief Determines whether two AABBs intersect, and computes collision resolution data (normal moves A out of B). */
        static bool TryGetCollisionAabbAabb(const Vector2& aMin, const Vector2& aMax, const Vector2& bMin, const Vector2& bMax, CollisionResult2D& result);

        /** @brief Determines whether an AABB intersects a convex polygon, and computes collision resolution data (normal moves the AABB out of the polygon). */
        static bool TryGetCollisionAabbConvexPolygon(const Vector2& aabbCenter, const Vector2& aabbHalfExtents, const std::vector<Vector2>& pVertices,
            const std::vector<Vector2>& pNormals, CollisionResult2D& result);

        /** @brief Determines whether an AABB intersects an OBB, and computes collision resolution data (normal moves the AABB out of the OBB). */
        static bool TryGetCollisionAabbObb(const Vector2& aabbCenter, const Vector2& aabbHalfExtents, const Vector2& obbCenter, const Vector2& obbAxisX,
            const Vector2& obbAxisY, const Vector2& obbHalfExtents, CollisionResult2D& result);

        /** @brief Determines whether two OBBs intersect, and computes collision resolution data (normal moves the first OBB out of the second). */
        static bool TryGetCollisionObbObb(const Vector2& aCenter, const Vector2& aAxisX, const Vector2& aAxisY, const Vector2& aHalf, const Vector2& bCenter,
            const Vector2& bAxisX, const Vector2& bAxisY, const Vector2& bHalf, CollisionResult2D& result);

        /** @brief Determines whether an OBB intersects a convex polygon, and computes collision resolution data (normal moves the OBB out of the polygon). */
        static bool TryGetCollisionObbConvexPolygon(const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalfExtents,
            const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals, CollisionResult2D& result);

        /**
         * @brief Determines whether two circles intersect, and computes collision resolution data (normal points from B's
         * center toward A's center; Vector2::UnitX when centers coincide).
         */
        static bool TryGetCollisionCircleCircle(const Vector2& aCenter, float aRadius, const Vector2& bCenter, float bRadius, CollisionResult2D& result);

        /**
         * @brief Determines whether a circle intersects an AABB, and computes collision resolution data (normal points from
         * the closest AABB point toward the circle center, or through the nearest face when the center is inside the AABB).
         */
        static bool TryGetCollisionCircleAabb(const Vector2& cCenter, float cRadius, const Vector2& boxMin, const Vector2& boxMax, CollisionResult2D& result);

        /**
         * @brief Determines whether a circle intersects an OBB, and computes collision resolution data (normal points from
         * the closest OBB point toward the circle center, or through the nearest face when the center is inside the OBB).
         */
        static bool TryGetCollisionCircleObb(const Vector2& cCenter, float cRadius, const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY,
            const Vector2& obbHalfExtents, CollisionResult2D& result);

        /**
         * @brief Determines whether a circle intersects a capsule, and computes collision resolution data (normal points
         * from the capsule's medial segment toward the circle center; perpendicular to the segment when centered on it;
         * Vector2::UnitX for a degenerate capsule).
         */
        static bool TryGetCollisionCircleCapsule(
            const Vector2& circleCenter, float circleRadius, const Vector2& capsuleA, const Vector2& capsuleB, float capsuleRadius, CollisionResult2D& result);

        /** @brief Determines whether two convex polygons intersect, and computes collision resolution data (normal moves the first polygon out of the second). */
        static bool TryGetCollisionConvexPolygonConvexPolygon(const std::vector<Vector2>& aVertices, const std::vector<Vector2>& aNormals,
            const std::vector<Vector2>& bVertices, const std::vector<Vector2>& bNormals, CollisionResult2D& result);
    };
}
