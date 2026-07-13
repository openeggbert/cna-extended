// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Collision2DTest.cs, covering
// the "Helper Methods" region and the full "Containment" region (AABB/Circle/OBB/Capsule/Convex
// Polygon Methods sub-regions) -- matching the scope of Collision2D.hpp/.cpp in this chunk. Tests
// from "Projection Methods" onward are out of scope for this chunk (deferred to the chunk that
// ports TryGetCollision*/Parametric Solvers/Ray Intervals).
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
}
