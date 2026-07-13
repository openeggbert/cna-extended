// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Collision2DTest.cs.
//
// Ported so far: the "Helper Methods" region, the full "Containment" region
// (AABB/Circle/OBB/Capsule/Convex Polygon Methods sub-regions), the "Parametric Solvers" region
// (SolveParametricIntersection2D / SolveParametricIntersectionWithImplicitLine), the
// "RayCircleIntersectionInterval Tests" / "RayCapsuleIntersectionInterval Tests" sub-regions, every
// "TryGetCollision*" sub-region under "Intersection Methods" (AabbAabb, AabbConvexPolygon, AabbObb,
// CircleCircle, CircleAabb, CircleObb, CircleCapsule, ObbObb, ObbConvexPolygon,
// ConvexPolygonConvexPolygon), and the "CollisionResult2D MTV Separation Tests" / "CollisionResult2D
// Reversed Input Tests" sub-regions.
//
// KNOWN GAP (flagged, not fixed here): upstream's "Projection Methods", "Distance Calculations",
// "ClosestPointRaySegment Tests", "Clipping Methods" (ClipLineToAabb/ClipLineToConvexPolygon),
// "Overlap Methods" (OverlapOnAxis/OverlapOnAxisAabbPolygon), and every plain-bool "Intersects*"
// sub-region (IntersectsAabbAabb, IntersectsAabbCapsule, IntersectsAabbConvexPolygon,
// IntersectsAabbObb, IntersectsCircleCircle, IntersectsCircleAabb, IntersectsCircleObb,
// IntersectsCircleConvexPolygon, IntersectsCircleCapsule, IntersectsObbObb, IntersectsObbCapsule,
// IntersectsObbConvexPolygon, IntersectsCapsuleCapsule, IntersectsCapsuleConvexPolygon,
// IntersectsConvexPolygonConvexPolygon) are STILL NOT PORTED even though their implementations
// already exist in Collision2D.cpp (from the first porting pass). This gap pre-dates this pass and
// was discovered, not introduced, here -- see NEXT.md for the follow-up task.
//
// C#'s `IsValidPolygon(null, normals)` / `IsValidPolygon(vertices, null)` have no direct C++
// equivalent since `const std::vector<Vector2>&` cannot be null; an empty vector is the faithful
// substitute (both yield the same `vertices.size() < 3` / length-mismatch outcome as the null
// case). C#'s `ref`/`out` float and Vector2 parameters map to ordinary non-const C++ references.
// Upstream does not have dedicated tests for ContainsObbAabb, ContainsObbCircle,
// ContainsCapsuleAabb, ContainsCapsuleCircle, ContainsConvexPolygonAabb, or
// ContainsConvexPolygonCircle -- so none are ported here either, matching upstream's coverage.
#include "CNA/Extended/Collision2D.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace CNA::Extended
{
    // ---- Helper Methods: IsValidPolygon ----

    TEST(Collision2DTests, IsValidPolygonValidPolygonReturnsTrue)
    {
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2::Normalize(Vector2(1.0f, 1.0f)), Vector2(-1.0f, 0.0f)};

        const bool result = Collision2D::IsValidPolygon(vertices, normals);

        EXPECT_TRUE(result);
    }

    TEST(Collision2DTests, IsValidPolygonNullVerticesReturnsFalse)
    {
        const std::vector<Vector2> normals{Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        const bool result = Collision2D::IsValidPolygon({}, normals);

        EXPECT_FALSE(result);
    }

    TEST(Collision2DTests, IsValidPolygonNullNormalsReturnsFalse)
    {
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f)};

        const bool result = Collision2D::IsValidPolygon(vertices, {});

        EXPECT_FALSE(result);
    }

    TEST(Collision2DTests, IsValidPolygonMismatchedArrayLengthsReturnsFalse)
    {
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f)};
        const std::vector<Vector2> normals{Vector2::UnitX, Vector2::UnitY};

        const bool result = Collision2D::IsValidPolygon(vertices, normals);

        EXPECT_FALSE(result);
    }

    TEST(Collision2DTests, IsValidPolygonLessThanThreeVerticesReturnsFalse)
    {
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f)};
        const std::vector<Vector2> normals{Vector2::UnitX, Vector2::UnitY};

        const bool result = Collision2D::IsValidPolygon(vertices, normals);

        EXPECT_FALSE(result);
    }

    // ---- Helper Methods: TryGetProjectionOverlap ----

    TEST(Collision2DTests, TryGetProjectionOverlapOverlappingIntervalsReturnsTrueAndOverlap)
    {
        float overlap = 0.0f;
        const bool result = Collision2D::TryGetProjectionOverlap(0.0f, 10.0f, 4.0f, 12.0f, overlap);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(6.0f, overlap);
    }

    TEST(Collision2DTests, TryGetProjectionOverlapTouchingIntervalsReturnsTrueAndZeroOverlap)
    {
        float overlap = 0.0f;
        const bool result = Collision2D::TryGetProjectionOverlap(0.0f, 10.0f, 10.0f, 12.0f, overlap);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(0.0f, overlap);
    }

    TEST(Collision2DTests, TryGetProjectionOverlapSeparatedIntervalsReturnsFalseAndZeroOverlap)
    {
        float overlap = 0.0f;
        const bool result = Collision2D::TryGetProjectionOverlap(0.0f, 10.0f, 12.0f, 14.0f, overlap);

        EXPECT_FALSE(result);
        EXPECT_FLOAT_EQ(0.0f, overlap);
    }

    TEST(Collision2DTests, TryGetProjectionOverlapReversedIntervalOrderReturnsSameOverlap)
    {
        float overlap = 0.0f;
        const bool result = Collision2D::TryGetProjectionOverlap(4.0f, 12.0f, 0.0f, 10.0f, overlap);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(6.0f, overlap);
    }

    // ---- Helper Methods: UpdateMinimumOverlap ----

    TEST(Collision2DTests, UpdateMinimumOverlapWithSmallerOverlapUpdatesTrackedValues)
    {
        float minimumOverlap = 10.0f;
        Vector2 minimumOverlapAxis = Vector2::UnitX;
        const Vector2 candidateAxis = Vector2::UnitY;

        const bool result = Collision2D::UpdateMinimumOverlap(5.0f, candidateAxis, minimumOverlap, minimumOverlapAxis);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(5.0f, minimumOverlap);
        EXPECT_EQ(candidateAxis, minimumOverlapAxis);
    }

    TEST(Collision2DTests, UpdateMinimumOverlapWithEqualOverlapDoesNotUpdateTrackedValues)
    {
        float minimumOverlap = 5.0f;
        Vector2 minimumOverlapAxis = Vector2::UnitX;

        const bool result = Collision2D::UpdateMinimumOverlap(5.0f, Vector2::UnitY, minimumOverlap, minimumOverlapAxis);

        EXPECT_FALSE(result);
        EXPECT_FLOAT_EQ(5.0f, minimumOverlap);
        EXPECT_EQ(Vector2::UnitX, minimumOverlapAxis);
    }

    TEST(Collision2DTests, UpdateMinimumOverlapWithLargerOverlapDoesNotUpdateTrackedValues)
    {
        float minimumOverlap = 5.0f;
        Vector2 minimumOverlapAxis = Vector2::UnitX;

        const bool result = Collision2D::UpdateMinimumOverlap(6.0f, Vector2::UnitY, minimumOverlap, minimumOverlapAxis);

        EXPECT_FALSE(result);
        EXPECT_FLOAT_EQ(5.0f, minimumOverlap);
        EXPECT_EQ(Vector2::UnitX, minimumOverlapAxis);
    }

    TEST(Collision2DTests, UpdateMinimumOverlapWithZeroOverlapUpdatesTrackedValues)
    {
        float minimumOverlap = 5.0f;
        Vector2 minimumOverlapAxis = Vector2::UnitX;
        const Vector2 candidateAxis = Vector2::UnitY;

        const bool result = Collision2D::UpdateMinimumOverlap(0.0f, candidateAxis, minimumOverlap, minimumOverlapAxis);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(0.0f, minimumOverlap);
        EXPECT_EQ(candidateAxis, minimumOverlapAxis);
    }

    // ---- Helper Methods: OrientNormal ----

    TEST(Collision2DTests, OrientNormalWhenNormalPointsFromBToAReturnsNormal)
    {
        const Vector2 normal = Vector2::UnitX;
        const Vector2 centerA(10.0f, 0.0f);
        const Vector2 centerB = Vector2::Zero;

        const Vector2 result = Collision2D::OrientNormal(normal, centerA, centerB);

        EXPECT_EQ(normal, result);
    }

    TEST(Collision2DTests, OrientNormalWhenNormalPointsFromAToBReturnsInvertedNormal)
    {
        const Vector2 normal = -Vector2::UnitX;
        const Vector2 centerA(10.0f, 0.0f);
        const Vector2 centerB = Vector2::Zero;

        const Vector2 result = Collision2D::OrientNormal(normal, centerA, centerB);

        EXPECT_EQ(Vector2::UnitX, result);
    }

    TEST(Collision2DTests, OrientNormalWhenCentersAreEqualReturnsNormal)
    {
        const Vector2 normal = Vector2::UnitY;
        const Vector2 center(5.0f, 5.0f);

        const Vector2 result = Collision2D::OrientNormal(normal, center, center);

        EXPECT_EQ(normal, result);
    }

    TEST(Collision2DTests, OrientNormalWhenNormalIsPerpendicularToCenterDeltaReturnsNormal)
    {
        const Vector2 normal = Vector2::UnitY;
        const Vector2 centerA(10.0f, 0.0f);
        const Vector2 centerB = Vector2::Zero;

        const Vector2 result = Collision2D::OrientNormal(normal, centerA, centerB);

        EXPECT_EQ(normal, result);
    }

    // ---- Helper Methods: ClipInterval (ref overload) ----

    TEST(Collision2DTests, ClipIntervalOverlappingIntervalsReturnsTrue)
    {
        float tMin = 0.0f;
        float tMax = 10.0f;
        const float clipMin = 3.0f;
        const float clipMax = 8.0f;

        const bool result = Collision2D::ClipInterval(tMin, tMax, clipMin, clipMax);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(3.0f, tMin);
        EXPECT_FLOAT_EQ(8.0f, tMax);
    }

    TEST(Collision2DTests, ClipIntervalNonOverlappingIntervalsReturnsFalse)
    {
        float tMin = 0.0f;
        float tMax = 5.0f;
        const float clipMin = 10.0f;
        const float clipMax = 15.0f;

        const bool result = Collision2D::ClipInterval(tMin, tMax, clipMin, clipMax);

        EXPECT_FALSE(result);
    }

    TEST(Collision2DTests, ClipIntervalTouchingAtBoundaryReturnsTrue)
    {
        float tMin = 0.0f;
        float tMax = 5.0f;
        const float clipMin = 5.0f;
        const float clipMax = 10.0f;

        const bool result = Collision2D::ClipInterval(tMin, tMax, clipMin, clipMax);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(5.0f, tMin);
        EXPECT_FLOAT_EQ(5.0f, tMax);
    }

    TEST(Collision2DTests, ClipIntervalClippingWiderThanOriginalKeepsOriginalBounds)
    {
        float tMin = 3.0f;
        float tMax = 7.0f;
        const float clipMin = 0.0f;
        const float clipMax = 10.0f;

        const bool result = Collision2D::ClipInterval(tMin, tMax, clipMin, clipMax);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(3.0f, tMin);
        EXPECT_FLOAT_EQ(7.0f, tMax);
    }

    TEST(Collision2DTests, ClipIntervalNegativeIntervalsWorksCorrectly)
    {
        float tMin = -10.0f;
        float tMax = -5.0f;
        const float clipMin = -8.0f;
        const float clipMax = -3.0f;

        const bool result = Collision2D::ClipInterval(tMin, tMax, clipMin, clipMax);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(-8.0f, tMin);
        EXPECT_FLOAT_EQ(-5.0f, tMax);
    }

    // ---- Helper Methods: ClipInterval (out overload) ----

    TEST(Collision2DTests, ClipIntervalOutOverlappingIntervalsReturnsClippedValues)
    {
        float clippedEnter = 0.0f;
        float clippedExit = 0.0f;

        const bool result = Collision2D::ClipInterval(0.0f, 10.0f, 3.0f, 8.0f, clippedEnter, clippedExit);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(3.0f, clippedEnter);
        EXPECT_FLOAT_EQ(8.0f, clippedExit);
    }

    TEST(Collision2DTests, ClipIntervalOutNonOverlappingIntervalsReturnsFalse)
    {
        float clippedEnter = 0.0f;
        float clippedExit = 0.0f;

        const bool result = Collision2D::ClipInterval(0.0f, 5.0f, 10.0f, 15.0f, clippedEnter, clippedExit);

        EXPECT_FALSE(result);
    }

    TEST(Collision2DTests, ClipIntervalOutTouchingAtBoundaryReturnsTrue)
    {
        float clippedEnter = 0.0f;
        float clippedExit = 0.0f;

        const bool result = Collision2D::ClipInterval(0.0f, 5.0f, 5.0f, 10.0f, clippedEnter, clippedExit);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(5.0f, clippedEnter);
        EXPECT_FLOAT_EQ(5.0f, clippedExit);
    }

    TEST(Collision2DTests, ClipIntervalOutClippingWiderThanOriginalReturnsOriginal)
    {
        float clippedEnter = 0.0f;
        float clippedExit = 0.0f;

        const bool result = Collision2D::ClipInterval(3.0f, 7.0f, 0.0f, 10.0f, clippedEnter, clippedExit);

        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(3.0f, clippedEnter);
        EXPECT_FLOAT_EQ(7.0f, clippedExit);
    }

    // ---- Helper Methods: IntervalsOverlap ----

    TEST(Collision2DTests, IntervalsOverlapOverlappingReturnsTrue)
    {
        const bool result = Collision2D::IntervalsOverlap(0.0f, 10.0f, 5.0f, 15.0f);

        EXPECT_TRUE(result);
    }

    TEST(Collision2DTests, IntervalsOverlapSeparatedReturnsFalse)
    {
        const bool result = Collision2D::IntervalsOverlap(0.0f, 5.0f, 10.0f, 15.0f);

        EXPECT_FALSE(result);
    }

    TEST(Collision2DTests, IntervalsOverlapTouchingAtPointReturnsTrue)
    {
        const bool result = Collision2D::IntervalsOverlap(0.0f, 5.0f, 5.0f, 10.0f);

        EXPECT_TRUE(result);
    }

    TEST(Collision2DTests, IntervalsOverlapNegativeIntervalsWorksCorrectly)
    {
        const bool result = Collision2D::IntervalsOverlap(-10.0f, -5.0f, -8.0f, -3.0f);

        EXPECT_TRUE(result);
    }

    TEST(Collision2DTests, IntervalsOverlapNearlyTouchingReturnsFalse)
    {
        const float minB = 5.0f + Collision2D::Epsilon * 2.0f;

        const bool result = Collision2D::IntervalsOverlap(0.0f, 5.0f, minB, 10.0f);

        EXPECT_FALSE(result);
    }

    // ---- Containment - AABB Methods: ContainsAabbPoint ----

    TEST(Collision2DTests, ContainsAabbPointPointInsideReturnsContains)
    {
        const Vector2 point(5.0f, 5.0f);
        const Vector2 min(0.0f, 0.0f);
        const Vector2 max(10.0f, 10.0f);

        const ContainmentType result = Collision2D::ContainsAabbPoint(point, min, max);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsAabbPointPointOutsideReturnsDisjoint)
    {
        const Vector2 point(15.0f, 5.0f);
        const Vector2 min(0.0f, 0.0f);
        const Vector2 max(10.0f, 10.0f);

        const ContainmentType result = Collision2D::ContainsAabbPoint(point, min, max);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsAabbPointPointOnBoundaryReturnsContains)
    {
        const Vector2 point(10.0f, 5.0f);
        const Vector2 min(0.0f, 0.0f);
        const Vector2 max(10.0f, 10.0f);

        const ContainmentType result = Collision2D::ContainsAabbPoint(point, min, max);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    // ---- Containment - AABB Methods: ContainsAabbAabb ----

    TEST(Collision2DTests, ContainsAabbAabbCompletelyContainsReturnsContains)
    {
        const Vector2 aMin(0.0f, 0.0f);
        const Vector2 aMax(10.0f, 10.0f);
        const Vector2 bMin(2.0f, 2.0f);
        const Vector2 bMax(8.0f, 8.0f);

        const ContainmentType result = Collision2D::ContainsAabbAabb(aMin, aMax, bMin, bMax);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsAabbAabbNoOverlapReturnsDisjoint)
    {
        const Vector2 aMin(0.0f, 0.0f);
        const Vector2 aMax(10.0f, 10.0f);
        const Vector2 bMin(15.0f, 15.0f);
        const Vector2 bMax(20.0f, 20.0f);

        const ContainmentType result = Collision2D::ContainsAabbAabb(aMin, aMax, bMin, bMax);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsAabbAabbPartialOverlapReturnsIntersects)
    {
        const Vector2 aMin(0.0f, 0.0f);
        const Vector2 aMax(10.0f, 10.0f);
        const Vector2 bMin(5.0f, 5.0f);
        const Vector2 bMax(15.0f, 15.0f);

        const ContainmentType result = Collision2D::ContainsAabbAabb(aMin, aMax, bMin, bMax);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - AABB Methods: ContainsAabbCircle ----

    TEST(Collision2DTests, ContainsAabbCircleCircleCompletelyInsideReturnsContains)
    {
        const Vector2 boxMin(0.0f, 0.0f);
        const Vector2 boxMax(10.0f, 10.0f);
        const Vector2 circleCenter(5.0f, 5.0f);
        const float circleRadius = 2.0f;

        const ContainmentType result = Collision2D::ContainsAabbCircle(boxMin, boxMax, circleCenter, circleRadius);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsAabbCircleCircleCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 boxMin(0.0f, 0.0f);
        const Vector2 boxMax(10.0f, 10.0f);
        const Vector2 circleCenter(20.0f, 20.0f);
        const float circleRadius = 3.0f;

        const ContainmentType result = Collision2D::ContainsAabbCircle(boxMin, boxMax, circleCenter, circleRadius);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsAabbCircleCirclePartiallyInsideReturnsIntersects)
    {
        const Vector2 boxMin(0.0f, 0.0f);
        const Vector2 boxMax(10.0f, 10.0f);
        const Vector2 circleCenter(5.0f, 5.0f);
        const float circleRadius = 7.0f;

        const ContainmentType result = Collision2D::ContainsAabbCircle(boxMin, boxMax, circleCenter, circleRadius);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - AABB Methods: ContainsAabbObb ----

    TEST(Collision2DTests, ContainsAabbObbObbCompletelyInsideReturnsContains)
    {
        const Vector2 aabbMin(0.0f, 0.0f);
        const Vector2 aabbMax(10.0f, 10.0f);
        const Vector2 obbCenter(5.0f, 5.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(2.0f, 2.0f);

        const ContainmentType result = Collision2D::ContainsAabbObb(aabbMin, aabbMax, obbCenter, obbAxisX, obbAxisY, obbHalfExtents);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsAabbObbObbCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 aabbMin(0.0f, 0.0f);
        const Vector2 aabbMax(10.0f, 10.0f);
        const Vector2 obbCenter(20.0f, 20.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(2.0f, 2.0f);

        const ContainmentType result = Collision2D::ContainsAabbObb(aabbMin, aabbMax, obbCenter, obbAxisX, obbAxisY, obbHalfExtents);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsAabbObbObbPartiallyInsideReturnsIntersects)
    {
        const Vector2 aabbMin(0.0f, 0.0f);
        const Vector2 aabbMax(10.0f, 10.0f);
        const Vector2 obbCenter(8.0f, 8.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(5.0f, 5.0f);

        const ContainmentType result = Collision2D::ContainsAabbObb(aabbMin, aabbMax, obbCenter, obbAxisX, obbAxisY, obbHalfExtents);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - AABB Methods: ContainsAabbCapsule ----

    TEST(Collision2DTests, ContainsAabbCapsuleCapsuleCompletelyInsideReturnsContains)
    {
        const Vector2 boxMin(0.0f, 0.0f);
        const Vector2 boxMax(10.0f, 10.0f);
        const Vector2 capsuleA(3.0f, 5.0f);
        const Vector2 capsuleB(7.0f, 5.0f);
        const float capsuleRadius = 1.0f;

        const ContainmentType result = Collision2D::ContainsAabbCapsule(boxMin, boxMax, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsAabbCapsuleCapsuleCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 boxMin(0.0f, 0.0f);
        const Vector2 boxMax(10.0f, 10.0f);
        const Vector2 capsuleA(15.0f, 15.0f);
        const Vector2 capsuleB(20.0f, 20.0f);
        const float capsuleRadius = 1.0f;

        const ContainmentType result = Collision2D::ContainsAabbCapsule(boxMin, boxMax, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsAabbCapsuleCapsulePartiallyInsideReturnsIntersects)
    {
        const Vector2 boxMin(0.0f, 0.0f);
        const Vector2 boxMax(10.0f, 10.0f);
        const Vector2 capsuleA(5.0f, 5.0f);
        const Vector2 capsuleB(15.0f, 5.0f);
        const float capsuleRadius = 1.0f;

        const ContainmentType result = Collision2D::ContainsAabbCapsule(boxMin, boxMax, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - AABB Methods: ContainsAabbConvexPolygon ----

    TEST(Collision2DTests, ContainsAabbConvexPolygonPolygonCompletelyInsideReturnsContains)
    {
        const Vector2 boxMin(0.0f, 0.0f);
        const Vector2 boxMax(10.0f, 10.0f);
        const std::vector<Vector2> vertices{Vector2(3.0f, 3.0f), Vector2(7.0f, 3.0f), Vector2(7.0f, 7.0f), Vector2(3.0f, 7.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsAabbConvexPolygon(boxMin, boxMax, vertices, normals);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsAabbConvexPolygonPolygonCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 boxMin(0.0f, 0.0f);
        const Vector2 boxMax(10.0f, 10.0f);
        const std::vector<Vector2> vertices{Vector2(15.0f, 15.0f), Vector2(20.0f, 15.0f), Vector2(20.0f, 20.0f), Vector2(15.0f, 20.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsAabbConvexPolygon(boxMin, boxMax, vertices, normals);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsAabbConvexPolygonPolygonPartiallyInsideReturnsIntersects)
    {
        const Vector2 boxMin(0.0f, 0.0f);
        const Vector2 boxMax(10.0f, 10.0f);
        const std::vector<Vector2> vertices{Vector2(5.0f, 5.0f), Vector2(15.0f, 5.0f), Vector2(15.0f, 15.0f), Vector2(5.0f, 15.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsAabbConvexPolygon(boxMin, boxMax, vertices, normals);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - Circle Methods: ContainsCirclePoint ----

    TEST(Collision2DTests, ContainsCirclePointPointInsideReturnsContains)
    {
        const Vector2 point(3.0f, 0.0f);
        const Vector2 center(0.0f, 0.0f);
        const float radius = 5.0f;

        const ContainmentType result = Collision2D::ContainsCirclePoint(point, center, radius);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsCirclePointPointOutsideReturnsDisjoint)
    {
        const Vector2 point(10.0f, 0.0f);
        const Vector2 center(0.0f, 0.0f);
        const float radius = 5.0f;

        const ContainmentType result = Collision2D::ContainsCirclePoint(point, center, radius);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsCirclePointPointOnCircumferenceReturnsContains)
    {
        const Vector2 point(5.0f, 0.0f);
        const Vector2 center(0.0f, 0.0f);
        const float radius = 5.0f;

        const ContainmentType result = Collision2D::ContainsCirclePoint(point, center, radius);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    // ---- Containment - Circle Methods: ContainsCircleAabb ----

    TEST(Collision2DTests, ContainsCircleAabbAabbCompletelyInsideReturnsContains)
    {
        const Vector2 circleCenter(10.0f, 10.0f);
        const float circleRadius = 10.0f;
        const Vector2 aabbMin(8.0f, 8.0f);
        const Vector2 aabbMax(12.0f, 12.0f);

        const ContainmentType result = Collision2D::ContainsCircleAabb(circleCenter, circleRadius, aabbMin, aabbMax);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsCircleAabbAabbCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 circleCenter(0.0f, 0.0f);
        const float circleRadius = 5.0f;
        const Vector2 aabbMin(20.0f, 20.0f);
        const Vector2 aabbMax(25.0f, 25.0f);

        const ContainmentType result = Collision2D::ContainsCircleAabb(circleCenter, circleRadius, aabbMin, aabbMax);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsCircleAabbAabbPartiallyInsideReturnsIntersects)
    {
        const Vector2 circleCenter(0.0f, 0.0f);
        const float circleRadius = 5.0f;
        const Vector2 aabbMin(3.0f, 3.0f);
        const Vector2 aabbMax(8.0f, 8.0f);

        const ContainmentType result = Collision2D::ContainsCircleAabb(circleCenter, circleRadius, aabbMin, aabbMax);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - Circle Methods: ContainsCircleCircle ----

    TEST(Collision2DTests, ContainsCircleCircleCompletelyContainsReturnsContains)
    {
        const Vector2 aCenter(0.0f, 0.0f);
        const float aRadius = 10.0f;
        const Vector2 bCenter(2.0f, 0.0f);
        const float bRadius = 3.0f;

        const ContainmentType result = Collision2D::ContainsCircleCircle(aCenter, aRadius, bCenter, bRadius);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsCircleCircleSeparatedReturnsDisjoint)
    {
        const Vector2 aCenter(0.0f, 0.0f);
        const float aRadius = 5.0f;
        const Vector2 bCenter(20.0f, 0.0f);
        const float bRadius = 3.0f;

        const ContainmentType result = Collision2D::ContainsCircleCircle(aCenter, aRadius, bCenter, bRadius);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsCircleCirclePartialOverlapReturnsIntersects)
    {
        const Vector2 aCenter(0.0f, 0.0f);
        const float aRadius = 5.0f;
        const Vector2 bCenter(7.0f, 0.0f);
        const float bRadius = 4.0f;

        const ContainmentType result = Collision2D::ContainsCircleCircle(aCenter, aRadius, bCenter, bRadius);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - Circle Methods: ContainsCircleObb ----

    TEST(Collision2DTests, ContainsCircleObbObbCompletelyInsideReturnsContains)
    {
        const Vector2 circleCenter(10.0f, 10.0f);
        const float circleRadius = 10.0f;
        const Vector2 obbCenter(10.0f, 10.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(2.0f, 2.0f);

        const ContainmentType result = Collision2D::ContainsCircleObb(circleCenter, circleRadius, obbCenter, obbAxisX, obbAxisY, obbHalfExtents);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsCircleObbObbCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 circleCenter(0.0f, 0.0f);
        const float circleRadius = 5.0f;
        const Vector2 obbCenter(20.0f, 20.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(2.0f, 2.0f);

        const ContainmentType result = Collision2D::ContainsCircleObb(circleCenter, circleRadius, obbCenter, obbAxisX, obbAxisY, obbHalfExtents);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsCircleObbObbPartiallyInsideReturnsIntersects)
    {
        const Vector2 circleCenter(0.0f, 0.0f);
        const float circleRadius = 5.0f;
        const Vector2 obbCenter(4.0f, 4.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(3.0f, 3.0f);

        const ContainmentType result = Collision2D::ContainsCircleObb(circleCenter, circleRadius, obbCenter, obbAxisX, obbAxisY, obbHalfExtents);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - Circle Methods: ContainsCircleCapsule ----

    TEST(Collision2DTests, ContainsCircleCapsuleCapsuleCompletelyInsideReturnsContains)
    {
        const Vector2 circleCenter(10.0f, 10.0f);
        const float circleRadius = 10.0f;
        const Vector2 capsuleA(8.0f, 10.0f);
        const Vector2 capsuleB(12.0f, 10.0f);
        const float capsuleRadius = 1.0f;

        const ContainmentType result = Collision2D::ContainsCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsCircleCapsuleCapsuleCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 circleCenter(0.0f, 0.0f);
        const float circleRadius = 5.0f;
        const Vector2 capsuleA(20.0f, 20.0f);
        const Vector2 capsuleB(25.0f, 25.0f);
        const float capsuleRadius = 1.0f;

        const ContainmentType result = Collision2D::ContainsCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsCircleCapsuleCapsulePartiallyInsideReturnsIntersects)
    {
        const Vector2 circleCenter(0.0f, 0.0f);
        const float circleRadius = 5.0f;
        const Vector2 capsuleA(3.0f, 0.0f);
        const Vector2 capsuleB(10.0f, 0.0f);
        const float capsuleRadius = 1.0f;

        const ContainmentType result = Collision2D::ContainsCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - Circle Methods: ContainsCircleConvexPolygon ----

    TEST(Collision2DTests, ContainsCircleConvexPolygonPolygonCompletelyInsideReturnsContains)
    {
        const Vector2 circleCenter(10.0f, 10.0f);
        const float circleRadius = 10.0f;
        const std::vector<Vector2> vertices{Vector2(9.0f, 9.0f), Vector2(11.0f, 9.0f), Vector2(11.0f, 11.0f), Vector2(9.0f, 11.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsCircleConvexPolygon(circleCenter, circleRadius, vertices, normals);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsCircleConvexPolygonPolygonCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 circleCenter(0.0f, 0.0f);
        const float circleRadius = 5.0f;
        const std::vector<Vector2> vertices{Vector2(20.0f, 20.0f), Vector2(25.0f, 20.0f), Vector2(25.0f, 25.0f), Vector2(20.0f, 25.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsCircleConvexPolygon(circleCenter, circleRadius, vertices, normals);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsCircleConvexPolygonPolygonPartiallyInsideReturnsIntersects)
    {
        const Vector2 circleCenter(0.0f, 0.0f);
        const float circleRadius = 5.0f;
        const std::vector<Vector2> vertices{Vector2(3.0f, 3.0f), Vector2(8.0f, 3.0f), Vector2(8.0f, 8.0f), Vector2(3.0f, 8.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsCircleConvexPolygon(circleCenter, circleRadius, vertices, normals);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - OBB Methods: ContainsObbPoint ----

    TEST(Collision2DTests, ContainsObbPointPointInsideReturnsContains)
    {
        const Vector2 point(5.0f, 5.0f);
        const Vector2 center(5.0f, 5.0f);
        const Vector2 axisX = Vector2::UnitX;
        const Vector2 axisY = Vector2::UnitY;
        const Vector2 halfExtents(3.0f, 3.0f);

        const ContainmentType result = Collision2D::ContainsObbPoint(point, center, axisX, axisY, halfExtents);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsObbPointPointOutsideReturnsDisjoint)
    {
        const Vector2 point(15.0f, 15.0f);
        const Vector2 center(5.0f, 5.0f);
        const Vector2 axisX = Vector2::UnitX;
        const Vector2 axisY = Vector2::UnitY;
        const Vector2 halfExtents(3.0f, 3.0f);

        const ContainmentType result = Collision2D::ContainsObbPoint(point, center, axisX, axisY, halfExtents);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsObbPointPointOnEdgeReturnsContains)
    {
        const Vector2 point(8.0f, 5.0f);
        const Vector2 center(5.0f, 5.0f);
        const Vector2 axisX = Vector2::UnitX;
        const Vector2 axisY = Vector2::UnitY;
        const Vector2 halfExtents(3.0f, 3.0f);

        const ContainmentType result = Collision2D::ContainsObbPoint(point, center, axisX, axisY, halfExtents);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    // ---- Containment - OBB Methods: ContainsObbObb ----

    TEST(Collision2DTests, ContainsObbObbCompletelyContainsReturnsContains)
    {
        const Vector2 aCenter(5.0f, 5.0f);
        const Vector2 aAxisX = Vector2::UnitX;
        const Vector2 aAxisY = Vector2::UnitY;
        const Vector2 aHalf(5.0f, 5.0f);
        const Vector2 bCenter(5.0f, 5.0f);
        const Vector2 bAxisX = Vector2::UnitX;
        const Vector2 bAxisY = Vector2::UnitY;
        const Vector2 bHalf(2.0f, 2.0f);

        const ContainmentType result = Collision2D::ContainsObbObb(aCenter, aAxisX, aAxisY, aHalf, bCenter, bAxisX, bAxisY, bHalf);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsObbObbSeparatedReturnsDisjoint)
    {
        const Vector2 aCenter(0.0f, 0.0f);
        const Vector2 aAxisX = Vector2::UnitX;
        const Vector2 aAxisY = Vector2::UnitY;
        const Vector2 aHalf(2.0f, 2.0f);
        const Vector2 bCenter(20.0f, 20.0f);
        const Vector2 bAxisX = Vector2::UnitX;
        const Vector2 bAxisY = Vector2::UnitY;
        const Vector2 bHalf(2.0f, 2.0f);

        const ContainmentType result = Collision2D::ContainsObbObb(aCenter, aAxisX, aAxisY, aHalf, bCenter, bAxisX, bAxisY, bHalf);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsObbObbPartialOverlapReturnsIntersects)
    {
        const Vector2 aCenter(5.0f, 5.0f);
        const Vector2 aAxisX = Vector2::UnitX;
        const Vector2 aAxisY = Vector2::UnitY;
        const Vector2 aHalf(5.0f, 5.0f);
        const Vector2 bCenter(8.0f, 8.0f);
        const Vector2 bAxisX = Vector2::UnitX;
        const Vector2 bAxisY = Vector2::UnitY;
        const Vector2 bHalf(5.0f, 5.0f);

        const ContainmentType result = Collision2D::ContainsObbObb(aCenter, aAxisX, aAxisY, aHalf, bCenter, bAxisX, bAxisY, bHalf);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - OBB Methods: ContainsObbCapsule ----

    TEST(Collision2DTests, ContainsObbCapsuleCapsuleCompletelyInsideReturnsContains)
    {
        const Vector2 boxCenter(10.0f, 10.0f);
        const Vector2 axisX = Vector2::UnitX;
        const Vector2 axisY = Vector2::UnitY;
        const Vector2 halfExtents(5.0f, 5.0f);
        const Vector2 capsuleA(8.0f, 10.0f);
        const Vector2 capsuleB(12.0f, 10.0f);
        const float capsuleRadius = 1.0f;

        const ContainmentType result = Collision2D::ContainsObbCapsule(boxCenter, axisX, axisY, halfExtents, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsObbCapsuleCapsuleCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 boxCenter(0.0f, 0.0f);
        const Vector2 axisX = Vector2::UnitX;
        const Vector2 axisY = Vector2::UnitY;
        const Vector2 halfExtents(5.0f, 5.0f);
        const Vector2 capsuleA(20.0f, 20.0f);
        const Vector2 capsuleB(25.0f, 25.0f);
        const float capsuleRadius = 1.0f;

        const ContainmentType result = Collision2D::ContainsObbCapsule(boxCenter, axisX, axisY, halfExtents, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsObbCapsuleCapsulePartiallyInsideReturnsIntersects)
    {
        const Vector2 boxCenter(5.0f, 5.0f);
        const Vector2 axisX = Vector2::UnitX;
        const Vector2 axisY = Vector2::UnitY;
        const Vector2 halfExtents(5.0f, 5.0f);
        const Vector2 capsuleA(5.0f, 5.0f);
        const Vector2 capsuleB(15.0f, 5.0f);
        const float capsuleRadius = 1.0f;

        const ContainmentType result = Collision2D::ContainsObbCapsule(boxCenter, axisX, axisY, halfExtents, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - OBB Methods: ContainsObbConvexPolygon ----

    TEST(Collision2DTests, ContainsObbConvexPolygonPolygonCompletelyInsideReturnsContains)
    {
        const Vector2 boxCenter(10.0f, 10.0f);
        const Vector2 axisX = Vector2::UnitX;
        const Vector2 axisY = Vector2::UnitY;
        const Vector2 halfExtents(5.0f, 5.0f);
        const std::vector<Vector2> vertices{Vector2(8.0f, 8.0f), Vector2(12.0f, 8.0f), Vector2(12.0f, 12.0f), Vector2(8.0f, 12.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsObbConvexPolygon(boxCenter, axisX, axisY, halfExtents, vertices, normals);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsObbConvexPolygonPolygonCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 boxCenter(0.0f, 0.0f);
        const Vector2 axisX = Vector2::UnitX;
        const Vector2 axisY = Vector2::UnitY;
        const Vector2 halfExtents(5.0f, 5.0f);
        const std::vector<Vector2> vertices{Vector2(20.0f, 20.0f), Vector2(25.0f, 20.0f), Vector2(25.0f, 25.0f), Vector2(20.0f, 25.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsObbConvexPolygon(boxCenter, axisX, axisY, halfExtents, vertices, normals);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsObbConvexPolygonPolygonPartiallyInsideReturnsIntersects)
    {
        const Vector2 boxCenter(5.0f, 5.0f);
        const Vector2 axisX = Vector2::UnitX;
        const Vector2 axisY = Vector2::UnitY;
        const Vector2 halfExtents(5.0f, 5.0f);
        const std::vector<Vector2> vertices{Vector2(5.0f, 5.0f), Vector2(15.0f, 5.0f), Vector2(15.0f, 15.0f), Vector2(5.0f, 15.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsObbConvexPolygon(boxCenter, axisX, axisY, halfExtents, vertices, normals);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - Capsule Methods: ContainsCapsulePoint ----

    TEST(Collision2DTests, ContainsCapsulePointPointInsideReturnsContains)
    {
        const Vector2 point(5.0f, 1.0f);
        const Vector2 a(0.0f, 0.0f);
        const Vector2 b(10.0f, 0.0f);
        const float radius = 2.0f;

        const ContainmentType result = Collision2D::ContainsCapsulePoint(point, a, b, radius);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsCapsulePointPointOutsideReturnsDisjoint)
    {
        const Vector2 point(5.0f, 10.0f);
        const Vector2 a(0.0f, 0.0f);
        const Vector2 b(10.0f, 0.0f);
        const float radius = 2.0f;

        const ContainmentType result = Collision2D::ContainsCapsulePoint(point, a, b, radius);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsCapsulePointPointOnBoundaryReturnsContains)
    {
        const Vector2 point(5.0f, 2.0f);
        const Vector2 a(0.0f, 0.0f);
        const Vector2 b(10.0f, 0.0f);
        const float radius = 2.0f;

        const ContainmentType result = Collision2D::ContainsCapsulePoint(point, a, b, radius);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    // ---- Containment - Capsule Methods: ContainsCapsuleObb ----

    TEST(Collision2DTests, ContainsCapsuleObbObbCompletelyInsideReturnsContains)
    {
        const Vector2 capsuleA(0.0f, 5.0f);
        const Vector2 capsuleB(20.0f, 5.0f);
        const float capsuleRadius = 5.0f;
        const Vector2 obbCenter(10.0f, 5.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalf(2.0f, 2.0f);

        const ContainmentType result = Collision2D::ContainsCapsuleObb(capsuleA, capsuleB, capsuleRadius, obbCenter, obbAxisX, obbAxisY, obbHalf);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsCapsuleObbObbCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 capsuleA(0.0f, 0.0f);
        const Vector2 capsuleB(10.0f, 0.0f);
        const float capsuleRadius = 2.0f;
        const Vector2 obbCenter(20.0f, 20.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalf(2.0f, 2.0f);

        const ContainmentType result = Collision2D::ContainsCapsuleObb(capsuleA, capsuleB, capsuleRadius, obbCenter, obbAxisX, obbAxisY, obbHalf);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsCapsuleObbObbPartiallyInsideReturnsIntersects)
    {
        const Vector2 capsuleA(0.0f, 5.0f);
        const Vector2 capsuleB(10.0f, 5.0f);
        const float capsuleRadius = 3.0f;
        const Vector2 obbCenter(8.0f, 5.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalf(5.0f, 5.0f);

        const ContainmentType result = Collision2D::ContainsCapsuleObb(capsuleA, capsuleB, capsuleRadius, obbCenter, obbAxisX, obbAxisY, obbHalf);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - Capsule Methods: ContainsCapsuleCapsule ----

    TEST(Collision2DTests, ContainsCapsuleCapsuleCompletelyContainsReturnsContains)
    {
        const Vector2 a0(0.0f, 0.0f);
        const Vector2 a1(20.0f, 0.0f);
        const float aRadius = 5.0f;
        const Vector2 b0(8.0f, 0.0f);
        const Vector2 b1(12.0f, 0.0f);
        const float bRadius = 2.0f;

        const ContainmentType result = Collision2D::ContainsCapsuleCapsule(a0, a1, aRadius, b0, b1, bRadius);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsCapsuleCapsuleSeparatedReturnsDisjoint)
    {
        const Vector2 a0(0.0f, 0.0f);
        const Vector2 a1(10.0f, 0.0f);
        const float aRadius = 2.0f;
        const Vector2 b0(20.0f, 20.0f);
        const Vector2 b1(30.0f, 20.0f);
        const float bRadius = 2.0f;

        const ContainmentType result = Collision2D::ContainsCapsuleCapsule(a0, a1, aRadius, b0, b1, bRadius);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsCapsuleCapsulePartialOverlapReturnsIntersects)
    {
        const Vector2 a0(0.0f, 0.0f);
        const Vector2 a1(10.0f, 0.0f);
        const float aRadius = 3.0f;
        const Vector2 b0(8.0f, 0.0f);
        const Vector2 b1(15.0f, 0.0f);
        const float bRadius = 2.0f;

        const ContainmentType result = Collision2D::ContainsCapsuleCapsule(a0, a1, aRadius, b0, b1, bRadius);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - Capsule Methods: ContainsCapsuleConvexPolygon ----

    TEST(Collision2DTests, ContainsCapsuleConvexPolygonPolygonCompletelyInsideReturnsContains)
    {
        const Vector2 capsuleA(0.0f, 10.0f);
        const Vector2 capsuleB(20.0f, 10.0f);
        const float capsuleRadius = 8.0f;
        const std::vector<Vector2> vertices{Vector2(8.0f, 8.0f), Vector2(12.0f, 8.0f), Vector2(12.0f, 12.0f), Vector2(8.0f, 12.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsCapsuleConvexPolygon(capsuleA, capsuleB, capsuleRadius, vertices, normals);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsCapsuleConvexPolygonPolygonCompletelyOutsideReturnsDisjoint)
    {
        const Vector2 capsuleA(0.0f, 0.0f);
        const Vector2 capsuleB(10.0f, 0.0f);
        const float capsuleRadius = 2.0f;
        const std::vector<Vector2> vertices{Vector2(20.0f, 20.0f), Vector2(25.0f, 20.0f), Vector2(25.0f, 25.0f), Vector2(20.0f, 25.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsCapsuleConvexPolygon(capsuleA, capsuleB, capsuleRadius, vertices, normals);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsCapsuleConvexPolygonPolygonPartiallyInsideReturnsIntersects)
    {
        const Vector2 capsuleA(0.0f, 5.0f);
        const Vector2 capsuleB(10.0f, 5.0f);
        const float capsuleRadius = 3.0f;
        const std::vector<Vector2> vertices{Vector2(5.0f, 5.0f), Vector2(15.0f, 5.0f), Vector2(15.0f, 15.0f), Vector2(5.0f, 15.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsCapsuleConvexPolygon(capsuleA, capsuleB, capsuleRadius, vertices, normals);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - Convex Polygon Methods: ContainsConvexPolygonPoint ----

    TEST(Collision2DTests, ContainsConvexPolygonPointPointInsideReturnsContains)
    {
        const Vector2 point(5.0f, 5.0f);
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f), Vector2(10.0f, 10.0f), Vector2(0.0f, 10.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsConvexPolygonPoint(point, vertices, normals);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsConvexPolygonPointPointOutsideReturnsDisjoint)
    {
        const Vector2 point(15.0f, 15.0f);
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f), Vector2(10.0f, 10.0f), Vector2(0.0f, 10.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsConvexPolygonPoint(point, vertices, normals);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsConvexPolygonPointPointOnEdgeReturnsContains)
    {
        const Vector2 point(10.0f, 5.0f);
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f), Vector2(10.0f, 10.0f), Vector2(0.0f, 10.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsConvexPolygonPoint(point, vertices, normals);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    // ---- Containment - Convex Polygon Methods: ContainsConvexPolygonCapsule ----

    TEST(Collision2DTests, ContainsConvexPolygonCapsuleCapsuleCompletelyInsideReturnsContains)
    {
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(20.0f, 0.0f), Vector2(20.0f, 20.0f), Vector2(0.0f, 20.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};
        const Vector2 capsuleA(8.0f, 10.0f);
        const Vector2 capsuleB(12.0f, 10.0f);
        const float capsuleRadius = 2.0f;

        const ContainmentType result = Collision2D::ContainsConvexPolygonCapsule(vertices, normals, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsConvexPolygonCapsuleCapsuleCompletelyOutsideReturnsDisjoint)
    {
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f), Vector2(10.0f, 10.0f), Vector2(0.0f, 10.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};
        const Vector2 capsuleA(20.0f, 20.0f);
        const Vector2 capsuleB(25.0f, 25.0f);
        const float capsuleRadius = 1.0f;

        const ContainmentType result = Collision2D::ContainsConvexPolygonCapsule(vertices, normals, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsConvexPolygonCapsuleCapsulePartiallyInsideReturnsIntersects)
    {
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f), Vector2(10.0f, 10.0f), Vector2(0.0f, 10.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};
        const Vector2 capsuleA(5.0f, 5.0f);
        const Vector2 capsuleB(15.0f, 5.0f);
        const float capsuleRadius = 1.0f;

        const ContainmentType result = Collision2D::ContainsConvexPolygonCapsule(vertices, normals, capsuleA, capsuleB, capsuleRadius);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - Convex Polygon Methods: ContainsConvexPolygonObb ----

    TEST(Collision2DTests, ContainsConvexPolygonObbObbCompletelyInsideReturnsContains)
    {
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(20.0f, 0.0f), Vector2(20.0f, 20.0f), Vector2(0.0f, 20.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};
        const Vector2 obbCenter(10.0f, 10.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalf(3.0f, 3.0f);

        const ContainmentType result = Collision2D::ContainsConvexPolygonObb(vertices, normals, obbCenter, obbAxisX, obbAxisY, obbHalf);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsConvexPolygonObbObbCompletelyOutsideReturnsDisjoint)
    {
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f), Vector2(10.0f, 10.0f), Vector2(0.0f, 10.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};
        const Vector2 obbCenter(20.0f, 20.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalf(2.0f, 2.0f);

        const ContainmentType result = Collision2D::ContainsConvexPolygonObb(vertices, normals, obbCenter, obbAxisX, obbAxisY, obbHalf);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsConvexPolygonObbObbPartiallyInsideReturnsIntersects)
    {
        const std::vector<Vector2> vertices{Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f), Vector2(10.0f, 10.0f), Vector2(0.0f, 10.0f)};
        const std::vector<Vector2> normals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};
        const Vector2 obbCenter(8.0f, 8.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalf(5.0f, 5.0f);

        const ContainmentType result = Collision2D::ContainsConvexPolygonObb(vertices, normals, obbCenter, obbAxisX, obbAxisY, obbHalf);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Containment - Convex Polygon Methods: ContainsConvexPolygonConvexPolygon ----

    TEST(Collision2DTests, ContainsConvexPolygonConvexPolygonCompletelyContainsReturnsContains)
    {
        const std::vector<Vector2> aVertices{Vector2(0.0f, 0.0f), Vector2(20.0f, 0.0f), Vector2(20.0f, 20.0f), Vector2(0.0f, 20.0f)};
        const std::vector<Vector2> aNormals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};
        const std::vector<Vector2> bVertices{Vector2(5.0f, 5.0f), Vector2(15.0f, 5.0f), Vector2(15.0f, 15.0f), Vector2(5.0f, 15.0f)};
        const std::vector<Vector2> bNormals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsConvexPolygonConvexPolygon(aVertices, aNormals, bVertices, bNormals);

        EXPECT_EQ(ContainmentType::Contains, result);
    }

    TEST(Collision2DTests, ContainsConvexPolygonConvexPolygonSeparatedReturnsDisjoint)
    {
        const std::vector<Vector2> aVertices{Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f), Vector2(10.0f, 10.0f), Vector2(0.0f, 10.0f)};
        const std::vector<Vector2> aNormals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};
        const std::vector<Vector2> bVertices{Vector2(20.0f, 20.0f), Vector2(30.0f, 20.0f), Vector2(30.0f, 30.0f), Vector2(20.0f, 30.0f)};
        const std::vector<Vector2> bNormals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsConvexPolygonConvexPolygon(aVertices, aNormals, bVertices, bNormals);

        EXPECT_EQ(ContainmentType::Disjoint, result);
    }

    TEST(Collision2DTests, ContainsConvexPolygonConvexPolygonPartialOverlapReturnsIntersects)
    {
        const std::vector<Vector2> aVertices{Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f), Vector2(10.0f, 10.0f), Vector2(0.0f, 10.0f)};
        const std::vector<Vector2> aNormals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};
        const std::vector<Vector2> bVertices{Vector2(5.0f, 5.0f), Vector2(15.0f, 5.0f), Vector2(15.0f, 15.0f), Vector2(5.0f, 15.0f)};
        const std::vector<Vector2> bNormals{Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)};

        const ContainmentType result = Collision2D::ContainsConvexPolygonConvexPolygon(aVertices, aNormals, bVertices, bNormals);

        EXPECT_EQ(ContainmentType::Intersects, result);
    }

    // ---- Parametric Solvers: SolveParametricIntersection2D ----

    TEST(Collision2DTests, SolveParametricIntersection2DPerpendicularLinesFindsIntersection)
    {
        const Vector2 origin1(0.0f, 5.0f);
        const Vector2 direction1 = Vector2::UnitX;
        const Vector2 origin2(5.0f, 0.0f);
        const Vector2 direction2 = Vector2::UnitY;

        float t1;
        float t2;
        const bool result = Collision2D::SolveParametricIntersection2D(origin1, direction1, origin2, direction2, t1, t2);

        EXPECT_TRUE(result);
        EXPECT_NEAR(5.0f, t1, Collision2D::Epsilon);
        EXPECT_NEAR(5.0f, t2, Collision2D::Epsilon);
    }

    TEST(Collision2DTests, SolveParametricIntersection2DParallelLinesReturnsFalse)
    {
        const Vector2 origin1(0.0f, 0.0f);
        const Vector2 direction1 = Vector2::UnitX;
        const Vector2 origin2(0.0f, 5.0f);
        const Vector2 direction2 = Vector2::UnitX;

        float t1;
        float t2;
        const bool result = Collision2D::SolveParametricIntersection2D(origin1, direction1, origin2, direction2, t1, t2);

        EXPECT_FALSE(result);
    }

    TEST(Collision2DTests, SolveParametricIntersection2DCollinearLinesReturnsFalse)
    {
        const Vector2 origin1(0.0f, 0.0f);
        const Vector2 direction1 = Vector2::UnitX;
        const Vector2 origin2(5.0f, 0.0f);
        const Vector2 direction2 = Vector2::UnitX;

        float t1;
        float t2;
        const bool result = Collision2D::SolveParametricIntersection2D(origin1, direction1, origin2, direction2, t1, t2);

        EXPECT_FALSE(result);
    }

    TEST(Collision2DTests, SolveParametricIntersection2DLinesAtAnAngleFindsIntersection)
    {
        const Vector2 origin1(0.0f, 0.0f);
        const Vector2 direction1(1.0f, 0.0f);
        const Vector2 origin2(0.0f, 0.0f);
        const Vector2 direction2(0.0f, 1.0f);

        float t1;
        float t2;
        const bool result = Collision2D::SolveParametricIntersection2D(origin1, direction1, origin2, direction2, t1, t2);

        EXPECT_TRUE(result);
        EXPECT_NEAR(0.0f, t1, Collision2D::Epsilon);
        EXPECT_NEAR(0.0f, t2, Collision2D::Epsilon);
    }

    // ---- Parametric Solvers: SolveParametricIntersectionWithImplicitLine ----

    TEST(Collision2DTests, SolveParametricIntersectionWithImplicitLineIntersectingFindsParameter)
    {
        const Vector2 lineNormal = Vector2::UnitY;
        const float lineDistance = 5.0f;
        const Vector2 origin(0.0f, 0.0f);
        const Vector2 direction = Vector2::UnitY;

        float t;
        const bool result = Collision2D::SolveParametricIntersectionWithImplicitLine(lineNormal, lineDistance, origin, direction, t);

        EXPECT_TRUE(result);
        EXPECT_NEAR(5.0f, t, Collision2D::Epsilon);
    }

    TEST(Collision2DTests, SolveParametricIntersectionWithImplicitLineParallelReturnsFalse)
    {
        const Vector2 lineNormal = Vector2::UnitY;
        const float lineDistance = 5.0f;
        const Vector2 origin(0.0f, 0.0f);
        const Vector2 direction = Vector2::UnitX;

        float t;
        const bool result = Collision2D::SolveParametricIntersectionWithImplicitLine(lineNormal, lineDistance, origin, direction, t);

        EXPECT_FALSE(result);
    }

    TEST(Collision2DTests, SolveParametricIntersectionWithImplicitLinePerpendicularFindsParameter)
    {
        const Vector2 lineNormal = Vector2::UnitX;
        const float lineDistance = 10.0f;
        const Vector2 origin(0.0f, 0.0f);
        const Vector2 direction = Vector2::UnitX;

        float t;
        const bool result = Collision2D::SolveParametricIntersectionWithImplicitLine(lineNormal, lineDistance, origin, direction, t);

        EXPECT_TRUE(result);
        EXPECT_NEAR(10.0f, t, Collision2D::Epsilon);
    }

    // ---- Ray Interval Methods: RayCircleIntersectionInterval ----

    TEST(Collision2DTests, RayCircleIntersectionIntervalTwoIntersectionsReturnsBothParameters)
    {
        const Vector2 origin(-10.0f, 0.0f);
        const Vector2 direction = Vector2::UnitX;
        const Vector2 center(0.0f, 0.0f);
        const float radius = 5.0f;

        float tMin;
        float tMax;
        const bool result = Collision2D::RayCircleIntersectionInterval(origin, direction, center, radius, tMin, tMax);

        EXPECT_TRUE(result);
        EXPECT_NEAR(5.0f, tMin, Collision2D::Epsilon);
        EXPECT_NEAR(15.0f, tMax, Collision2D::Epsilon);
    }

    TEST(Collision2DTests, RayCircleIntersectionIntervalTangentReturnsSinglePoint)
    {
        const Vector2 origin(-10.0f, 5.0f);
        const Vector2 direction = Vector2::UnitX;
        const Vector2 center(0.0f, 0.0f);
        const float radius = 5.0f;

        float tMin;
        float tMax;
        const bool result = Collision2D::RayCircleIntersectionInterval(origin, direction, center, radius, tMin, tMax);

        EXPECT_TRUE(result);
        EXPECT_NEAR(tMin, tMax, Collision2D::Epsilon);
    }

    TEST(Collision2DTests, RayCircleIntersectionIntervalMissReturnsFalse)
    {
        const Vector2 origin(-10.0f, 10.0f);
        const Vector2 direction = Vector2::UnitX;
        const Vector2 center(0.0f, 0.0f);
        const float radius = 5.0f;

        float tMin;
        float tMax;
        const bool result = Collision2D::RayCircleIntersectionInterval(origin, direction, center, radius, tMin, tMax);

        EXPECT_FALSE(result);
    }

    TEST(Collision2DTests, RayCircleIntersectionIntervalOriginInsideReturnsIntervalFromZero)
    {
        const Vector2 origin(0.0f, 0.0f);
        const Vector2 direction = Vector2::UnitX;
        const Vector2 center(0.0f, 0.0f);
        const float radius = 5.0f;

        float tMin;
        float tMax;
        const bool result = Collision2D::RayCircleIntersectionInterval(origin, direction, center, radius, tMin, tMax);

        EXPECT_TRUE(result);
        EXPECT_NEAR(0.0f, tMin, Collision2D::Epsilon);
        EXPECT_NEAR(5.0f, tMax, Collision2D::Epsilon);
    }

    // ---- Ray Interval Methods: RayCapsuleIntersectionInterval ----

    TEST(Collision2DTests, RayCapsuleIntersectionIntervalHitsCylinderReturnsInterval)
    {
        const Vector2 rayOrigin(-10.0f, 0.0f);
        const Vector2 rayDirection = Vector2::UnitX;
        const Vector2 segA(0.0f, -5.0f);
        const Vector2 segB(0.0f, 5.0f);
        const float radius = 3.0f;

        float tMin;
        float tMax;
        const bool result = Collision2D::RayCapsuleIntersectionInterval(rayOrigin, rayDirection, segA, segB, radius, tMin, tMax);

        EXPECT_TRUE(result);
        EXPECT_TRUE(tMax > tMin);
    }

    TEST(Collision2DTests, RayCapsuleIntersectionIntervalHitsEndCapReturnsInterval)
    {
        const Vector2 rayOrigin(-10.0f, 10.0f);
        const Vector2 rayDirection = Vector2::UnitX;
        const Vector2 segA(0.0f, 10.0f);
        const Vector2 segB(10.0f, 10.0f);
        const float radius = 3.0f;

        float tMin;
        float tMax;
        const bool result = Collision2D::RayCapsuleIntersectionInterval(rayOrigin, rayDirection, segA, segB, radius, tMin, tMax);

        EXPECT_TRUE(result);
    }

    TEST(Collision2DTests, RayCapsuleIntersectionIntervalMissReturnsFalse)
    {
        const Vector2 rayOrigin(-10.0f, 20.0f);
        const Vector2 rayDirection = Vector2::UnitX;
        const Vector2 segA(0.0f, 0.0f);
        const Vector2 segB(10.0f, 0.0f);
        const float radius = 3.0f;

        float tMin;
        float tMax;
        const bool result = Collision2D::RayCapsuleIntersectionInterval(rayOrigin, rayDirection, segA, segB, radius, tMin, tMax);

        EXPECT_FALSE(result);
    }

    TEST(Collision2DTests, RayCapsuleIntersectionIntervalOriginInsideReturnsIntervalFromZero)
    {
        const Vector2 rayOrigin(5.0f, 5.0f);
        const Vector2 rayDirection = Vector2::UnitX;
        const Vector2 segA(0.0f, 5.0f);
        const Vector2 segB(10.0f, 5.0f);
        const float radius = 3.0f;

        float tMin;
        float tMax;
        const bool result = Collision2D::RayCapsuleIntersectionInterval(rayOrigin, rayDirection, segA, segB, radius, tMin, tMax);

        EXPECT_TRUE(result);
        EXPECT_NEAR(0.0f, tMin, Collision2D::Epsilon);
    }

    TEST(Collision2DTests, RayCapsuleIntersectionIntervalDegenerateCapsuleBehavesLikeCircle)
    {
        const Vector2 rayOrigin(-10.0f, 0.0f);
        const Vector2 rayDirection = Vector2::UnitX;
        const Vector2 segA(0.0f, 0.0f);
        const Vector2 segB(0.0f, 0.0f);
        const float radius = 5.0f;

        float tMin;
        float tMax;
        const bool result = Collision2D::RayCapsuleIntersectionInterval(rayOrigin, rayDirection, segA, segB, radius, tMin, tMax);

        EXPECT_TRUE(result);
        EXPECT_NEAR(5.0f, tMin, Collision2D::Epsilon);
        EXPECT_NEAR(15.0f, tMax, Collision2D::Epsilon);
    }

    // ---- Intersection Methods: TryGetCollisionAabbAabb ----

    TEST(Collision2DTests, TryGetCollisionAabbAabbOverlappingReturnsTrueAndCollisionResult)
    {
        const Vector2 aMin(-2.0f, -2.0f);
        const Vector2 aMax(2.0f, 2.0f);
        const Vector2 bMin(1.0f, -2.0f);
        const Vector2 bMax(5.0f, 2.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbAabb(aMin, aMax, bMin, bMax, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionAabbAabbSeparatedReturnsFalseAndNone)
    {
        const Vector2 aMin(-1.0f, -1.0f);
        const Vector2 aMax(1.0f, 1.0f);
        const Vector2 bMin(4.0f, -1.0f);
        const Vector2 bMax(6.0f, 1.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbAabb(aMin, aMax, bMin, bMax, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(Vector2::Zero, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionAabbAabbTouchingReturnsTrueAndZeroDepth)
    {
        const Vector2 aMin(-1.0f, -1.0f);
        const Vector2 aMax(1.0f, 1.0f);
        const Vector2 bMin(1.0f, -1.0f);
        const Vector2 bMax(3.0f, 1.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbAabb(aMin, aMax, bMin, bMax, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionAabbAabbWhenFirstBoxIsContainedReturnsSeparatingMinimumTranslationVector)
    {
        const Vector2 aMin(-1.0f, -1.0f);
        const Vector2 aMax(1.0f, 1.0f);
        const Vector2 bMin(-10.0f, -10.0f);
        const Vector2 bMax(10.0f, 10.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbAabb(aMin, aMax, bMin, bMax, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(11.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-11.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionAabbAabbWhenInputsAreReversedThenMinimumTranslationVectorIsOpposite)
    {
        const Vector2 aMin(-2.0f, -2.0f);
        const Vector2 aMax(2.0f, 2.0f);
        const Vector2 bMin(1.0f, -2.0f);
        const Vector2 bMax(5.0f, 2.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbAabb(aMin, aMax, bMin, bMax, result);
        CollisionResult2D reversedResult;
        const bool reversedIntersects = Collision2D::TryGetCollisionAabbAabb(bMin, bMax, aMin, aMax, reversedResult);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(reversedIntersects);
        EXPECT_EQ(-result.MinimumTranslationVector, reversedResult.MinimumTranslationVector);
    }

    // ---- Intersection Methods: TryGetCollisionAabbConvexPolygon ----

    TEST(Collision2DTests, TryGetCollisionAabbConvexPolygonOverlappingReturnsTrueAndCollisionResult)
    {
        const Vector2 aabbCenter = Vector2::Zero;
        const Vector2 aabbHalfExtents(2.0f, 2.0f);
        const std::vector<Vector2> vertices{Vector2(1.0f, -2.0f), Vector2(5.0f, -2.0f), Vector2(5.0f, 2.0f), Vector2(1.0f, 2.0f)};
        const std::vector<Vector2> normals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbConvexPolygon(aabbCenter, aabbHalfExtents, vertices, normals, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionAabbConvexPolygonSeparatedReturnsFalseAndNone)
    {
        const Vector2 aabbCenter = Vector2::Zero;
        const Vector2 aabbHalfExtents(1.0f, 1.0f);
        const std::vector<Vector2> vertices{Vector2(4.0f, -1.0f), Vector2(6.0f, -1.0f), Vector2(6.0f, 1.0f), Vector2(4.0f, 1.0f)};
        const std::vector<Vector2> normals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbConvexPolygon(aabbCenter, aabbHalfExtents, vertices, normals, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(Vector2::Zero, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionAabbConvexPolygonTouchingReturnsTrueAndZeroDepth)
    {
        const Vector2 aabbCenter = Vector2::Zero;
        const Vector2 aabbHalfExtents(1.0f, 1.0f);
        const std::vector<Vector2> vertices{Vector2(1.0f, -1.0f), Vector2(3.0f, -1.0f), Vector2(3.0f, 1.0f), Vector2(1.0f, 1.0f)};
        const std::vector<Vector2> normals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbConvexPolygon(aabbCenter, aabbHalfExtents, vertices, normals, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionAabbConvexPolygonPolygonOnLeftMovesAabbRight)
    {
        const Vector2 aabbCenter = Vector2::Zero;
        const Vector2 aabbHalfExtents(2.0f, 2.0f);
        const std::vector<Vector2> vertices{Vector2(-5.0f, -2.0f), Vector2(-1.0f, -2.0f), Vector2(-1.0f, 2.0f), Vector2(-5.0f, 2.0f)};
        const std::vector<Vector2> normals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbConvexPolygon(aabbCenter, aabbHalfExtents, vertices, normals, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(1.0f, 0.0f), result.MinimumTranslationVector);
    }

    // ---- Intersection Methods: TryGetCollisionAabbObb ----

    TEST(Collision2DTests, TryGetCollisionAabbObbOverlappingReturnsTrueAndCollisionResult)
    {
        const Vector2 aabbCenter = Vector2::Zero;
        const Vector2 aabbHalfExtents(2.0f, 2.0f);
        const Vector2 obbCenter(3.0f, 0.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(2.0f, 2.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbObb(aabbCenter, aabbHalfExtents, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionAabbObbSeparatedReturnsFalseAndNone)
    {
        const Vector2 aabbCenter = Vector2::Zero;
        const Vector2 aabbHalfExtents(1.0f, 1.0f);
        const Vector2 obbCenter(5.0f, 0.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(1.0f, 1.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbObb(aabbCenter, aabbHalfExtents, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(Vector2::Zero, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionAabbObbTouchingReturnsTrueAndZeroDepth)
    {
        const Vector2 aabbCenter = Vector2::Zero;
        const Vector2 aabbHalfExtents(1.0f, 1.0f);
        const Vector2 obbCenter(2.0f, 0.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(1.0f, 1.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbObb(aabbCenter, aabbHalfExtents, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionAabbObbObbOnLeftMovesAabbRight)
    {
        const Vector2 aabbCenter = Vector2::Zero;
        const Vector2 aabbHalfExtents(2.0f, 2.0f);
        const Vector2 obbCenter(-3.0f, 0.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(2.0f, 2.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbObb(aabbCenter, aabbHalfExtents, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(1.0f, 0.0f), result.MinimumTranslationVector);
    }

    // ---- Intersection Methods: TryGetCollisionCircleCircle ----

    TEST(Collision2DTests, TryGetCollisionCircleCircleOverlappingReturnsTrueAndCollisionResult)
    {
        const Vector2 aCenter = Vector2::Zero;
        const float aRadius = 5.0f;
        const Vector2 bCenter(8.0f, 0.0f);
        const float bRadius = 5.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCircle(aCenter, aRadius, bCenter, bRadius, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(2.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-2.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleCircleSeparatedReturnsFalseAndNone)
    {
        const Vector2 aCenter = Vector2::Zero;
        const float aRadius = 5.0f;
        const Vector2 bCenter(20.0f, 0.0f);
        const float bRadius = 5.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCircle(aCenter, aRadius, bCenter, bRadius, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(Vector2::Zero, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleCircleTouchingReturnsTrueAndZeroDepth)
    {
        const Vector2 aCenter = Vector2::Zero;
        const float aRadius = 5.0f;
        const Vector2 bCenter(10.0f, 0.0f);
        const float bRadius = 5.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCircle(aCenter, aRadius, bCenter, bRadius, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleCircleSecondCircleOnLeftMovesFirstCircleRight)
    {
        const Vector2 aCenter = Vector2::Zero;
        const float aRadius = 5.0f;
        const Vector2 bCenter(-8.0f, 0.0f);
        const float bRadius = 5.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCircle(aCenter, aRadius, bCenter, bRadius, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(2.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(2.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleCircleCoincidentCentersUsesStableNormal)
    {
        const Vector2 aCenter = Vector2::Zero;
        const float aRadius = 3.0f;
        const Vector2 bCenter = Vector2::Zero;
        const float bRadius = 2.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCircle(aCenter, aRadius, bCenter, bRadius, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(5.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(5.0f, 0.0f), result.MinimumTranslationVector);
    }

    // ---- Intersection Methods: TryGetCollisionCircleAabb ----

    TEST(Collision2DTests, TryGetCollisionCircleAabbOverlappingReturnsTrueAndCollisionResult)
    {
        const Vector2 circleCenter(14.0f, 5.0f);
        const float circleRadius = 5.0f;
        const Vector2 boxMin = Vector2::Zero;
        const Vector2 boxMax(10.0f, 10.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleAabb(circleCenter, circleRadius, boxMin, boxMax, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleAabbSeparatedReturnsFalseAndNone)
    {
        const Vector2 circleCenter(16.0f, 5.0f);
        const float circleRadius = 5.0f;
        const Vector2 boxMin = Vector2::Zero;
        const Vector2 boxMax(10.0f, 10.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleAabb(circleCenter, circleRadius, boxMin, boxMax, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(Vector2::Zero, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleAabbTouchingReturnsTrueAndZeroDepth)
    {
        const Vector2 circleCenter(15.0f, 5.0f);
        const float circleRadius = 5.0f;
        const Vector2 boxMin = Vector2::Zero;
        const Vector2 boxMax(10.0f, 10.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleAabb(circleCenter, circleRadius, boxMin, boxMax, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleAabbCircleOnLeftMovesCircleLeft)
    {
        const Vector2 circleCenter(-4.0f, 5.0f);
        const float circleRadius = 5.0f;
        const Vector2 boxMin = Vector2::Zero;
        const Vector2 boxMax(10.0f, 10.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleAabb(circleCenter, circleRadius, boxMin, boxMax, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleAabbCircleCenterInsideBoxMovesCircleThroughNearestFace)
    {
        const Vector2 circleCenter(8.0f, 5.0f);
        const float circleRadius = 2.0f;
        const Vector2 boxMin = Vector2::Zero;
        const Vector2 boxMax(10.0f, 10.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleAabb(circleCenter, circleRadius, boxMin, boxMax, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(4.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(4.0f, 0.0f), result.MinimumTranslationVector);
    }

    // ---- Intersection Methods: TryGetCollisionCircleObb ----

    TEST(Collision2DTests, TryGetCollisionCircleObbOverlappingReturnsTrueAndCollisionResult)
    {
        const Vector2 circleCenter(9.0f, 0.0f);
        const float circleRadius = 5.0f;
        const Vector2 obbCenter = Vector2::Zero;
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(5.0f, 5.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleObb(circleCenter, circleRadius, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleObbSeparatedReturnsFalseAndNone)
    {
        const Vector2 circleCenter(11.0f, 0.0f);
        const float circleRadius = 5.0f;
        const Vector2 obbCenter = Vector2::Zero;
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(5.0f, 5.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleObb(circleCenter, circleRadius, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(Vector2::Zero, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleObbTouchingReturnsTrueAndZeroDepth)
    {
        const Vector2 circleCenter(10.0f, 0.0f);
        const float circleRadius = 5.0f;
        const Vector2 obbCenter = Vector2::Zero;
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(5.0f, 5.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleObb(circleCenter, circleRadius, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleObbCircleOnLeftMovesCircleLeft)
    {
        const Vector2 circleCenter(-9.0f, 0.0f);
        const float circleRadius = 5.0f;
        const Vector2 obbCenter = Vector2::Zero;
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(5.0f, 5.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleObb(circleCenter, circleRadius, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleObbCircleCenterInsideObbMovesCircleThroughNearestFace)
    {
        const Vector2 circleCenter(3.0f, 0.0f);
        const float circleRadius = 2.0f;
        const Vector2 obbCenter = Vector2::Zero;
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(5.0f, 5.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleObb(circleCenter, circleRadius, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(4.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(4.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleObbRotatedObbReturnsWorldSpaceCollisionResult)
    {
        const Vector2 circleCenter(0.0f, 9.0f);
        const float circleRadius = 5.0f;
        const Vector2 obbCenter = Vector2::Zero;
        const Vector2 obbAxisX = Vector2::UnitY;
        const Vector2 obbAxisY = -Vector2::UnitX;
        const Vector2 obbHalfExtents(5.0f, 5.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleObb(circleCenter, circleRadius, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitY, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(0.0f, 1.0f), result.MinimumTranslationVector);
    }

    // ---- Intersection Methods: TryGetCollisionCircleCapsule ----

    TEST(Collision2DTests, TryGetCollisionCircleCapsuleOverlappingReturnsTrueAndCollisionResult)
    {
        const Vector2 circleCenter = Vector2::Zero;
        const float circleRadius = 5.0f;
        const Vector2 capsuleA(6.0f, 0.0f);
        const Vector2 capsuleB(14.0f, 0.0f);
        const float capsuleRadius = 2.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleCapsuleSeparatedReturnsFalseAndNone)
    {
        const Vector2 circleCenter = Vector2::Zero;
        const float circleRadius = 5.0f;
        const Vector2 capsuleA(8.0f, 0.0f);
        const Vector2 capsuleB(14.0f, 0.0f);
        const float capsuleRadius = 2.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(Vector2::Zero, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleCapsuleTouchingReturnsTrueAndZeroDepth)
    {
        const Vector2 circleCenter = Vector2::Zero;
        const float circleRadius = 5.0f;
        const Vector2 capsuleA(7.0f, 0.0f);
        const Vector2 capsuleB(14.0f, 0.0f);
        const float capsuleRadius = 2.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleCapsuleCapsuleOnLeftMovesCircleRight)
    {
        const Vector2 circleCenter = Vector2::Zero;
        const float circleRadius = 5.0f;
        const Vector2 capsuleA(-14.0f, 0.0f);
        const Vector2 capsuleB(-6.0f, 0.0f);
        const float capsuleRadius = 2.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleCapsuleCircleCenterOnCapsuleSegmentUsesPerpendicularNormal)
    {
        const Vector2 circleCenter = Vector2::Zero;
        const float circleRadius = 1.0f;
        const Vector2 capsuleA(-5.0f, 0.0f);
        const Vector2 capsuleB(5.0f, 0.0f);
        const float capsuleRadius = 2.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitY, result.Normal);
        EXPECT_FLOAT_EQ(3.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(0.0f, 3.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleCapsuleDegenerateCapsuleUsesStableNormal)
    {
        const Vector2 circleCenter = Vector2::Zero;
        const float circleRadius = 5.0f;
        const Vector2 capsuleA = Vector2::Zero;
        const Vector2 capsuleB = Vector2::Zero;
        const float capsuleRadius = 2.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(7.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(7.0f, 0.0f), result.MinimumTranslationVector);
    }

    // ---- CollisionResult2D MTV Separation Tests ----

    TEST(Collision2DTests, TryGetCollisionAabbObbWhenMinimumTranslationVectorAppliedThenRemovesPenetration)
    {
        const Vector2 aabbCenter = Vector2::Zero;
        const Vector2 aabbHalfExtents(2.0f, 2.0f);
        const Vector2 obbCenter(3.0f, 0.0f);
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(2.0f, 2.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionAabbObb(aabbCenter, aabbHalfExtents, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, result);
        const Vector2 movedAabbCenter = aabbCenter + result.MinimumTranslationVector;
        CollisionResult2D resolvedResult;
        const bool resolvedIntersects =
            Collision2D::TryGetCollisionAabbObb(movedAabbCenter, aabbHalfExtents, obbCenter, obbAxisX, obbAxisY, obbHalfExtents, resolvedResult);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(resolvedIntersects);
        EXPECT_FLOAT_EQ(0.0f, resolvedResult.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, resolvedResult.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleCircleWhenMinimumTranslationVectorAppliedThenRemovesPenetration)
    {
        const Vector2 aCenter = Vector2::Zero;
        const float aRadius = 5.0f;
        const Vector2 bCenter(8.0f, 0.0f);
        const float bRadius = 5.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCircle(aCenter, aRadius, bCenter, bRadius, result);
        const Vector2 movedCenter = aCenter + result.MinimumTranslationVector;
        CollisionResult2D resolvedResult;
        const bool resolvedIntersects = Collision2D::TryGetCollisionCircleCircle(movedCenter, aRadius, bCenter, bRadius, resolvedResult);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(resolvedIntersects);
        EXPECT_FLOAT_EQ(0.0f, resolvedResult.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, resolvedResult.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleAabbWhenMinimumTranslationVectorAppliedThenRemovesPenetration)
    {
        const Vector2 circleCenter(14.0f, 5.0f);
        const float circleRadius = 5.0f;
        const Vector2 boxMin = Vector2::Zero;
        const Vector2 boxMax(10.0f, 10.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleAabb(circleCenter, circleRadius, boxMin, boxMax, result);
        const Vector2 movedCircleCenter = circleCenter + result.MinimumTranslationVector;
        CollisionResult2D resolvedResult;
        const bool resolvedIntersects = Collision2D::TryGetCollisionCircleAabb(movedCircleCenter, circleRadius, boxMin, boxMax, resolvedResult);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(resolvedIntersects);
        EXPECT_FLOAT_EQ(0.0f, resolvedResult.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, resolvedResult.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionCircleCapsuleWhenMinimumTranslationVectorAppliedThenRemovesPenetration)
    {
        const Vector2 circleCenter = Vector2::Zero;
        const float circleRadius = 5.0f;
        const Vector2 capsuleA(6.0f, 0.0f);
        const Vector2 capsuleB(14.0f, 0.0f);
        const float capsuleRadius = 2.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius, result);
        const Vector2 movedCircleCenter = circleCenter + result.MinimumTranslationVector;
        CollisionResult2D resolvedResult;
        const bool resolvedIntersects =
            Collision2D::TryGetCollisionCircleCapsule(movedCircleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius, resolvedResult);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(resolvedIntersects);
        EXPECT_FLOAT_EQ(0.0f, resolvedResult.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, resolvedResult.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionConvexPolygonConvexPolygonWhenMinimumTranslationVectorAppliedThenRemovesPenetration)
    {
        std::vector<Vector2> aVertices{Vector2(-2.0f, -2.0f), Vector2(2.0f, -2.0f), Vector2(2.0f, 2.0f), Vector2(-2.0f, 2.0f)};
        const std::vector<Vector2> aNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};
        const std::vector<Vector2> bVertices{Vector2(1.0f, -2.0f), Vector2(5.0f, -2.0f), Vector2(5.0f, 2.0f), Vector2(1.0f, 2.0f)};
        const std::vector<Vector2> bNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionConvexPolygonConvexPolygon(aVertices, aNormals, bVertices, bNormals, result);
        for (std::size_t i = 0; i < aVertices.size(); i++)
        {
            aVertices[i] = aVertices[i] + result.MinimumTranslationVector;
        }
        CollisionResult2D resolvedResult;
        const bool resolvedIntersects = Collision2D::TryGetCollisionConvexPolygonConvexPolygon(aVertices, aNormals, bVertices, bNormals, resolvedResult);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(resolvedIntersects);
        EXPECT_FLOAT_EQ(0.0f, resolvedResult.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, resolvedResult.MinimumTranslationVector);
    }

    // ---- CollisionResult2D Reversed Input Tests ----

    TEST(Collision2DTests, TryGetCollisionCircleCircleWhenInputsAreReversedThenMinimumTranslationVectorIsOpposite)
    {
        const Vector2 aCenter = Vector2::Zero;
        const float aRadius = 5.0f;
        const Vector2 bCenter(8.0f, 0.0f);
        const float bRadius = 5.0f;

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionCircleCircle(aCenter, aRadius, bCenter, bRadius, result);
        CollisionResult2D reversedResult;
        const bool reversedIntersects = Collision2D::TryGetCollisionCircleCircle(bCenter, bRadius, aCenter, aRadius, reversedResult);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(reversedIntersects);
        EXPECT_EQ(-result.Normal, reversedResult.Normal);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, reversedResult.PenetrationDepth);
        EXPECT_EQ(-result.MinimumTranslationVector, reversedResult.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionObbObbWhenInputsAreReversedThenMinimumTranslationVectorIsOpposite)
    {
        const Vector2 aCenter = Vector2::Zero;
        const Vector2 aAxisX = Vector2::UnitX;
        const Vector2 aAxisY = Vector2::UnitY;
        const Vector2 aHalf(2.0f, 2.0f);
        const Vector2 bCenter(3.0f, 0.0f);
        const Vector2 bAxisX = Vector2::UnitX;
        const Vector2 bAxisY = Vector2::UnitY;
        const Vector2 bHalf(2.0f, 2.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionObbObb(aCenter, aAxisX, aAxisY, aHalf, bCenter, bAxisX, bAxisY, bHalf, result);
        CollisionResult2D reversedResult;
        const bool reversedIntersects = Collision2D::TryGetCollisionObbObb(bCenter, bAxisX, bAxisY, bHalf, aCenter, aAxisX, aAxisY, aHalf, reversedResult);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(reversedIntersects);
        EXPECT_EQ(-result.Normal, reversedResult.Normal);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, reversedResult.PenetrationDepth);
        EXPECT_EQ(-result.MinimumTranslationVector, reversedResult.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionConvexPolygonConvexPolygonWhenInputsAreReversedThenMinimumTranslationVectorIsOpposite)
    {
        const std::vector<Vector2> aVertices{Vector2(-2.0f, -2.0f), Vector2(2.0f, -2.0f), Vector2(2.0f, 2.0f), Vector2(-2.0f, 2.0f)};
        const std::vector<Vector2> aNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};
        const std::vector<Vector2> bVertices{Vector2(1.0f, -2.0f), Vector2(5.0f, -2.0f), Vector2(5.0f, 2.0f), Vector2(1.0f, 2.0f)};
        const std::vector<Vector2> bNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionConvexPolygonConvexPolygon(aVertices, aNormals, bVertices, bNormals, result);
        CollisionResult2D reversedResult;
        const bool reversedIntersects = Collision2D::TryGetCollisionConvexPolygonConvexPolygon(bVertices, bNormals, aVertices, aNormals, reversedResult);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(reversedIntersects);
        EXPECT_EQ(-result.Normal, reversedResult.Normal);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, reversedResult.PenetrationDepth);
        EXPECT_EQ(-result.MinimumTranslationVector, reversedResult.MinimumTranslationVector);
    }

    // ---- Intersection Methods: TryGetCollisionObbObb ----

    TEST(Collision2DTests, TryGetCollisionObbObbOverlappingReturnsTrueAndCollisionResult)
    {
        const Vector2 aCenter = Vector2::Zero;
        const Vector2 aAxisX = Vector2::UnitX;
        const Vector2 aAxisY = Vector2::UnitY;
        const Vector2 aHalf(2.0f, 2.0f);
        const Vector2 bCenter(3.0f, 0.0f);
        const Vector2 bAxisX = Vector2::UnitX;
        const Vector2 bAxisY = Vector2::UnitY;
        const Vector2 bHalf(2.0f, 2.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionObbObb(aCenter, aAxisX, aAxisY, aHalf, bCenter, bAxisX, bAxisY, bHalf, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionObbObbSeparatedReturnsFalseAndNone)
    {
        const Vector2 aCenter = Vector2::Zero;
        const Vector2 aAxisX = Vector2::UnitX;
        const Vector2 aAxisY = Vector2::UnitY;
        const Vector2 aHalf(1.0f, 1.0f);
        const Vector2 bCenter(5.0f, 0.0f);
        const Vector2 bAxisX = Vector2::UnitX;
        const Vector2 bAxisY = Vector2::UnitY;
        const Vector2 bHalf(1.0f, 1.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionObbObb(aCenter, aAxisX, aAxisY, aHalf, bCenter, bAxisX, bAxisY, bHalf, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(Vector2::Zero, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionObbObbTouchingReturnsTrueAndZeroDepth)
    {
        const Vector2 aCenter = Vector2::Zero;
        const Vector2 aAxisX = Vector2::UnitX;
        const Vector2 aAxisY = Vector2::UnitY;
        const Vector2 aHalf(1.0f, 1.0f);
        const Vector2 bCenter(2.0f, 0.0f);
        const Vector2 bAxisX = Vector2::UnitX;
        const Vector2 bAxisY = Vector2::UnitY;
        const Vector2 bHalf(1.0f, 1.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionObbObb(aCenter, aAxisX, aAxisY, aHalf, bCenter, bAxisX, bAxisY, bHalf, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionObbObbSecondObbOnLeftMovesFirstObbRight)
    {
        const Vector2 aCenter = Vector2::Zero;
        const Vector2 aAxisX = Vector2::UnitX;
        const Vector2 aAxisY = Vector2::UnitY;
        const Vector2 aHalf(2.0f, 2.0f);
        const Vector2 bCenter(-3.0f, 0.0f);
        const Vector2 bAxisX = Vector2::UnitX;
        const Vector2 bAxisY = Vector2::UnitY;
        const Vector2 bHalf(2.0f, 2.0f);

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionObbObb(aCenter, aAxisX, aAxisY, aHalf, bCenter, bAxisX, bAxisY, bHalf, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(1.0f, 0.0f), result.MinimumTranslationVector);
    }

    // ---- Intersection Methods: TryGetCollisionObbConvexPolygon ----

    TEST(Collision2DTests, TryGetCollisionObbConvexPolygonOverlappingReturnsTrueAndCollisionResult)
    {
        const Vector2 obbCenter = Vector2::Zero;
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(2.0f, 2.0f);
        const std::vector<Vector2> vertices{Vector2(1.0f, -2.0f), Vector2(5.0f, -2.0f), Vector2(5.0f, 2.0f), Vector2(1.0f, 2.0f)};
        const std::vector<Vector2> normals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionObbConvexPolygon(obbCenter, obbAxisX, obbAxisY, obbHalfExtents, vertices, normals, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionObbConvexPolygonSeparatedReturnsFalseAndNone)
    {
        const Vector2 obbCenter = Vector2::Zero;
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(1.0f, 1.0f);
        const std::vector<Vector2> vertices{Vector2(4.0f, -1.0f), Vector2(6.0f, -1.0f), Vector2(6.0f, 1.0f), Vector2(4.0f, 1.0f)};
        const std::vector<Vector2> normals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionObbConvexPolygon(obbCenter, obbAxisX, obbAxisY, obbHalfExtents, vertices, normals, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(Vector2::Zero, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionObbConvexPolygonTouchingReturnsTrueAndZeroDepth)
    {
        const Vector2 obbCenter = Vector2::Zero;
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(1.0f, 1.0f);
        const std::vector<Vector2> vertices{Vector2(1.0f, -1.0f), Vector2(3.0f, -1.0f), Vector2(3.0f, 1.0f), Vector2(1.0f, 1.0f)};
        const std::vector<Vector2> normals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionObbConvexPolygon(obbCenter, obbAxisX, obbAxisY, obbHalfExtents, vertices, normals, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionObbConvexPolygonPolygonOnLeftMovesObbRight)
    {
        const Vector2 obbCenter = Vector2::Zero;
        const Vector2 obbAxisX = Vector2::UnitX;
        const Vector2 obbAxisY = Vector2::UnitY;
        const Vector2 obbHalfExtents(2.0f, 2.0f);
        const std::vector<Vector2> vertices{Vector2(-5.0f, -2.0f), Vector2(-1.0f, -2.0f), Vector2(-1.0f, 2.0f), Vector2(-5.0f, 2.0f)};
        const std::vector<Vector2> normals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionObbConvexPolygon(obbCenter, obbAxisX, obbAxisY, obbHalfExtents, vertices, normals, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(1.0f, 0.0f), result.MinimumTranslationVector);
    }

    // ---- Intersection Methods: TryGetCollisionConvexPolygonConvexPolygon ----

    TEST(Collision2DTests, TryGetCollisionConvexPolygonConvexPolygonOverlappingReturnsTrueAndCollisionResult)
    {
        const std::vector<Vector2> aVertices{Vector2(-2.0f, -2.0f), Vector2(2.0f, -2.0f), Vector2(2.0f, 2.0f), Vector2(-2.0f, 2.0f)};
        const std::vector<Vector2> aNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};
        const std::vector<Vector2> bVertices{Vector2(1.0f, -2.0f), Vector2(5.0f, -2.0f), Vector2(5.0f, 2.0f), Vector2(1.0f, 2.0f)};
        const std::vector<Vector2> bNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionConvexPolygonConvexPolygon(aVertices, aNormals, bVertices, bNormals, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionConvexPolygonConvexPolygonSeparatedReturnsFalseAndNone)
    {
        const std::vector<Vector2> aVertices{Vector2(-1.0f, -1.0f), Vector2(1.0f, -1.0f), Vector2(1.0f, 1.0f), Vector2(-1.0f, 1.0f)};
        const std::vector<Vector2> aNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};
        const std::vector<Vector2> bVertices{Vector2(4.0f, -1.0f), Vector2(6.0f, -1.0f), Vector2(6.0f, 1.0f), Vector2(4.0f, 1.0f)};
        const std::vector<Vector2> bNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionConvexPolygonConvexPolygon(aVertices, aNormals, bVertices, bNormals, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(Vector2::Zero, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionConvexPolygonConvexPolygonTouchingReturnsTrueAndZeroDepth)
    {
        const std::vector<Vector2> aVertices{Vector2(-1.0f, -1.0f), Vector2(1.0f, -1.0f), Vector2(1.0f, 1.0f), Vector2(-1.0f, 1.0f)};
        const std::vector<Vector2> aNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};
        const std::vector<Vector2> bVertices{Vector2(1.0f, -1.0f), Vector2(3.0f, -1.0f), Vector2(3.0f, 1.0f), Vector2(1.0f, 1.0f)};
        const std::vector<Vector2> bNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionConvexPolygonConvexPolygon(aVertices, aNormals, bVertices, bNormals, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(Collision2DTests, TryGetCollisionConvexPolygonConvexPolygonSecondPolygonOnLeftMovesFirstPolygonRight)
    {
        const std::vector<Vector2> aVertices{Vector2(-2.0f, -2.0f), Vector2(2.0f, -2.0f), Vector2(2.0f, 2.0f), Vector2(-2.0f, 2.0f)};
        const std::vector<Vector2> aNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};
        const std::vector<Vector2> bVertices{Vector2(-5.0f, -2.0f), Vector2(-1.0f, -2.0f), Vector2(-1.0f, 2.0f), Vector2(-5.0f, 2.0f)};
        const std::vector<Vector2> bNormals{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX};

        CollisionResult2D result;
        const bool intersects = Collision2D::TryGetCollisionConvexPolygonConvexPolygon(aVertices, aNormals, bVertices, bNormals, result);

        EXPECT_TRUE(intersects);
        EXPECT_EQ(Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(1.0f, 0.0f), result.MinimumTranslationVector);
    }
}
