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

        // Non-uniform half-extents (2, 3, 4) on X/Y/Z so each "sphere center embedded in the
        // box" near-face case below has exactly one unambiguous nearest face.
        const BoundingBox kBoxAsymmetric(Vector3(-2.0f, -3.0f, -4.0f), Vector3(2.0f, 3.0f, 4.0f));
        const BoundingBox kUnitCube(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
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

    // A-01 regression tests: sphere center embedded inside the box (GetClosestPoint
    // degenerates to the center itself, dist == 0), previously mishandled -- see
    // CollisionShape3DEXT.cpp's file header for the bug this fixes.

    namespace
    {
        // Directly proves the MTV actually separates the shapes, rather than just checking
        // specific numeric fields (a round-1 version of these tests checked the fields but
        // not actual separation, which is exactly how round 1's direction bug slipped
        // through -- see CollisionShape3DEXT.cpp's file header). Translates the box's AABB
        // by the MTV and asserts the translated box's interval no longer overlaps the
        // sphere's own interval on every axis (a sphere's projection onto any axis is
        // exactly [center-radius, center+radius], so this is an exact check, not an
        // approximation).
        void ExpectMtvActuallySeparates(const BoundingBox& box, const BoundingSphere& sphere, const CollisionResult3DEXT& result)
        {
            const BoundingBox moved(box.Min + result.MinimumTranslationVector, box.Max + result.MinimumTranslationVector);
            const auto separatedOnAxis = [](float boxMin, float boxMax, float sphereCenter, float radius) {
                return boxMax <= sphereCenter - radius + 1e-4f || boxMin >= sphereCenter + radius - 1e-4f;
            };
            const bool separated = separatedOnAxis(moved.Min.X, moved.Max.X, sphere.Center.X, sphere.Radius) ||
                                    separatedOnAxis(moved.Min.Y, moved.Max.Y, sphere.Center.Y, sphere.Radius) ||
                                    separatedOnAxis(moved.Min.Z, moved.Max.Z, sphere.Center.Z, sphere.Radius);
            EXPECT_TRUE(separated) << "translated box [" << moved.Min.X << "," << moved.Max.X << "]x[" << moved.Min.Y << ","
                                    << moved.Max.Y << "]x[" << moved.Min.Z << "," << moved.Max.Z << "] still overlaps sphere center=("
                                    << sphere.Center.X << "," << sphere.Center.Y << "," << sphere.Center.Z << ") radius=" << sphere.Radius;
        }
    }

    TEST(CollisionShape3DEXTTests, BoxSphere_TryGetCollision_SphereCenterAtBoxCenter_PicksDeterministicFaceAndFullDepth)
    {
        CollisionShape3DEXT box(kUnitCube);
        const BoundingSphere sphereShape(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
        CollisionShape3DEXT sphere(sphereShape);

        CollisionResult3DEXT result;
        ASSERT_TRUE(box.TryGetCollision(sphere, result));
        // All six faces are equidistant (1.0) from a dead-center point; the nearest-face scan's
        // deterministic tie-breaking (first candidate wins ties) always selects +X as
        // *nearest*, so the escape direction (away from the nearest face) is -X.
        EXPECT_NEAR(result.Normal.X, -1.0f, 1e-4f);
        EXPECT_NEAR(result.Normal.Y, 0.0f, 1e-4f);
        EXPECT_NEAR(result.Normal.Z, 0.0f, 1e-4f);
        // Box must clear the sphere entirely: distance to the nearest face (1.0) + full radius (0.5).
        EXPECT_NEAR(result.PenetrationDepth, 1.5f, 1e-4f);
        ExpectMtvActuallySeparates(kUnitCube, sphereShape, result);
    }

    TEST(CollisionShape3DEXTTests, BoxSphere_TryGetCollision_EmbeddedNearEachFace_PicksThatFaceAndFullDepth)
    {
        // (sphere center, expected escape normal -- the OPPOSITE of the nearest face's own
        // outward direction, see CollisionShape3DEXT.cpp's file header for why) -- radius is
        // always 1.0, and every center sits 0.5 units from exactly one face of
        // kBoxAsymmetric, so expected depth is always 0.5 (distance to that face) + 1.0
        // (radius) = 1.5.
        const struct
        {
            Vector3 center;
            Vector3 expectedNormal;
        } cases[] = {
            {Vector3(1.5f, 0.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f)},
            {Vector3(-1.5f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f)},
            {Vector3(0.0f, 2.5f, 0.0f), Vector3(0.0f, -1.0f, 0.0f)},
            {Vector3(0.0f, -2.5f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)},
            {Vector3(0.0f, 0.0f, 3.5f), Vector3(0.0f, 0.0f, -1.0f)},
            {Vector3(0.0f, 0.0f, -3.5f), Vector3(0.0f, 0.0f, 1.0f)},
        };

        for (const auto& testCase : cases)
        {
            CollisionShape3DEXT box(kBoxAsymmetric);
            const BoundingSphere sphereShape(testCase.center, 1.0f);
            CollisionShape3DEXT sphere(sphereShape);

            CollisionResult3DEXT result;
            ASSERT_TRUE(box.TryGetCollision(sphere, result));
            EXPECT_NEAR(result.Normal.X, testCase.expectedNormal.X, 1e-4f);
            EXPECT_NEAR(result.Normal.Y, testCase.expectedNormal.Y, 1e-4f);
            EXPECT_NEAR(result.Normal.Z, testCase.expectedNormal.Z, 1e-4f);
            EXPECT_NEAR(result.PenetrationDepth, 1.5f, 1e-4f);
            ExpectMtvActuallySeparates(kBoxAsymmetric, sphereShape, result);
        }
    }

    TEST(CollisionShape3DEXTTests, BoxSphere_TryGetCollision_EmbeddedNonUniformBox_MtvActuallySeparates)
    {
        // Regression for the exact counter-example that caught round 1's direction bug
        // (see CollisionShape3DEXT.cpp's file header): box [-2,2] on X (part of
        // kBoxAsymmetric), sphere center x=1.5, radius 1. Round 1 returned +X/depth 1.5,
        // translating the box to [-0.5, 3.5] -- which still overlaps the sphere's
        // [0.5, 2.5] interval. The fixed code must return -X/depth 1.5 instead.
        CollisionShape3DEXT box(kBoxAsymmetric);
        const BoundingSphere sphereShape(Vector3(1.5f, 0.0f, 0.0f), 1.0f);
        CollisionShape3DEXT sphere(sphereShape);

        CollisionResult3DEXT result;
        ASSERT_TRUE(box.TryGetCollision(sphere, result));
        EXPECT_NEAR(result.Normal.X, -1.0f, 1e-4f);
        EXPECT_NEAR(result.PenetrationDepth, 1.5f, 1e-4f);
        ExpectMtvActuallySeparates(kBoxAsymmetric, sphereShape, result);
    }

    TEST(CollisionShape3DEXTTests, SphereBox_TryGetCollision_EmbeddedCase_IsInverseOfBoxSphere)
    {
        CollisionShape3DEXT box(kBoxAsymmetric);
        CollisionShape3DEXT sphere(BoundingSphere(Vector3(1.5f, 0.0f, 0.0f), 1.0f));

        CollisionResult3DEXT boxVsSphere;
        ASSERT_TRUE(box.TryGetCollision(sphere, boxVsSphere));

        CollisionResult3DEXT sphereVsBox;
        ASSERT_TRUE(sphere.TryGetCollision(box, sphereVsBox));

        EXPECT_NEAR(sphereVsBox.Normal.X, -boxVsSphere.Normal.X, 1e-4f);
        EXPECT_NEAR(sphereVsBox.PenetrationDepth, boxVsSphere.PenetrationDepth, 1e-4f);
        EXPECT_NEAR(sphereVsBox.PenetrationDepth, 1.5f, 1e-4f);
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
