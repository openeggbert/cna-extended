// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for CollisionShape3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here. Follows CollisionShape2DTests.cpp's
// own documented "spot-check pair per delegation branch" convention rather than exhaustive
// permutation coverage.
#include "CNA/Extended/World3DEXT/CollisionShape3DEXT.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    namespace
    {
        const BoundingBox kBoxA(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f));
        const BoundingBox kBoxOverlapping(Vector3(1.0f, 0.0f, 0.0f), Vector3(3.0f, 2.0f, 2.0f));
        const BoundingBox kBoxFar(Vector3(100.0f, 100.0f, 100.0f), Vector3(102.0f, 102.0f, 102.0f));
        const BoundingSphere kSphereA(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
        const BoundingSphere kSphereOverlapping(Vector3(1.5f, 0.0f, 0.0f), 1.0f);
        const BoundingSphere kSphereFar(Vector3(100.0f, 100.0f, 100.0f), 1.0f);
    }

    TEST(CollisionShape3DEXTTests, DefaultConstructed_IsNoneAndNeverIntersects)
    {
        CollisionShape3DEXT none;
        CollisionShape3DEXT box(kBoxA);

        EXPECT_EQ(none.getKindProperty(), CollisionShapeKind3DEXT::None);
        EXPECT_FALSE(none.Intersects(box));
        EXPECT_FALSE(box.Intersects(none));
    }

    TEST(CollisionShape3DEXTTests, BoxBox_Overlapping_Intersects)
    {
        CollisionShape3DEXT first(kBoxA);
        CollisionShape3DEXT second(kBoxOverlapping);
        EXPECT_TRUE(first.Intersects(second));
    }

    TEST(CollisionShape3DEXTTests, BoxBox_Separated_DoesNotIntersect)
    {
        CollisionShape3DEXT first(kBoxA);
        CollisionShape3DEXT second(kBoxFar);
        EXPECT_FALSE(first.Intersects(second));
    }

    TEST(CollisionShape3DEXTTests, BoxBox_TryGetCollision_ProducesMinimumTranslationVectorAlongSmallestOverlapAxis)
    {
        CollisionShape3DEXT first(kBoxA);
        CollisionShape3DEXT second(kBoxOverlapping);

        CollisionResult3DEXT result;
        ASSERT_TRUE(first.TryGetCollision(second, result));
        EXPECT_TRUE(result.Intersects);
        // Overlap along X is 1 unit (1..2); Y/Z fully overlap at 2 units -- X is the
        // separation axis. first (kBoxA) is centered left of second, so it should be
        // pushed further in -X.
        EXPECT_NEAR(result.PenetrationDepth, 1.0f, 1e-4f);
        EXPECT_NEAR(result.Normal.X, -1.0f, 1e-4f);
        EXPECT_NEAR(result.MinimumTranslationVector.X, -1.0f, 1e-4f);
    }

    TEST(CollisionShape3DEXTTests, SphereSphere_Overlapping_Intersects)
    {
        CollisionShape3DEXT first(kSphereA);
        CollisionShape3DEXT second(kSphereOverlapping);
        EXPECT_TRUE(first.Intersects(second));
    }

    TEST(CollisionShape3DEXTTests, SphereSphere_Separated_DoesNotIntersect)
    {
        CollisionShape3DEXT first(kSphereA);
        CollisionShape3DEXT second(kSphereFar);
        EXPECT_FALSE(first.Intersects(second));
    }

    TEST(CollisionShape3DEXTTests, SphereSphere_TryGetCollision_ProducesExpectedPenetrationDepth)
    {
        CollisionShape3DEXT first(kSphereA);
        CollisionShape3DEXT second(kSphereOverlapping);

        CollisionResult3DEXT result;
        ASSERT_TRUE(first.TryGetCollision(second, result));
        // Centers 1.5 apart, radii sum to 2.0 -> depth 0.5, normal along -X (first is left of second).
        EXPECT_NEAR(result.PenetrationDepth, 0.5f, 1e-4f);
        EXPECT_NEAR(result.Normal.X, -1.0f, 1e-4f);
    }

    TEST(CollisionShape3DEXTTests, BoxSphere_Overlapping_Intersects)
    {
        CollisionShape3DEXT box(kBoxA);
        CollisionShape3DEXT sphere(BoundingSphere(Vector3(2.5f, 1.0f, 1.0f), 1.0f));
        EXPECT_TRUE(box.Intersects(sphere));
        EXPECT_TRUE(sphere.Intersects(box));
    }

    TEST(CollisionShape3DEXTTests, BoxSphere_TryGetCollision_MovesBoxAwayFromSphere)
    {
        CollisionShape3DEXT box(kBoxA);
        CollisionShape3DEXT sphere(BoundingSphere(Vector3(2.5f, 1.0f, 1.0f), 1.0f));

        CollisionResult3DEXT result;
        ASSERT_TRUE(box.TryGetCollision(sphere, result));
        // Closest point on box to sphere center (2.5,1,1) is (2,1,1); distance 0.5, depth 0.5,
        // normal points from sphere toward the box (i.e. -X, away from the sphere).
        EXPECT_NEAR(result.PenetrationDepth, 0.5f, 1e-4f);
        EXPECT_NEAR(result.Normal.X, -1.0f, 1e-4f);
    }

    TEST(CollisionShape3DEXTTests, SphereBox_TryGetCollision_IsInverseOfBoxSphere)
    {
        CollisionShape3DEXT box(kBoxA);
        CollisionShape3DEXT sphere(BoundingSphere(Vector3(2.5f, 1.0f, 1.0f), 1.0f));

        CollisionResult3DEXT boxVsSphere;
        ASSERT_TRUE(box.TryGetCollision(sphere, boxVsSphere));

        CollisionResult3DEXT sphereVsBox;
        ASSERT_TRUE(sphere.TryGetCollision(box, sphereVsBox));

        EXPECT_NEAR(sphereVsBox.Normal.X, -boxVsSphere.Normal.X, 1e-4f);
        EXPECT_NEAR(sphereVsBox.PenetrationDepth, boxVsSphere.PenetrationDepth, 1e-4f);
    }

    TEST(CollisionShape3DEXTTests, CollisionResult3DEXT_Invert_NegatesNormalAndMinimumTranslationVector)
    {
        const CollisionResult3DEXT result(true, Vector3(1.0f, 0.0f, 0.0f), 0.5f, Vector3(0.5f, 0.0f, 0.0f));
        const CollisionResult3DEXT inverted = result.Invert();

        EXPECT_EQ(inverted.Intersects, result.Intersects);
        EXPECT_EQ(inverted.Normal, Vector3(-1.0f, 0.0f, 0.0f));
        EXPECT_EQ(inverted.PenetrationDepth, result.PenetrationDepth);
        EXPECT_EQ(inverted.MinimumTranslationVector, Vector3(-0.5f, 0.0f, 0.0f));
    }
}
