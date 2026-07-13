// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Math/RectangleFTests.cs
// (Normalize + the GitHub issue #747 regression test), plus
// tests/MonoGame.Extended.Tests/Primitives/RectangleFTests.cs's constructor and Transform tests
// (now unblocked -- SizeF and Matrix3x2/PrimitivesHelper have landed). That upstream file's
// `Rectangle_Intersects_Test` is NOT ported: it exercises `Rectangle.Intersects`, a base
// XNA/FNA `Rectangle` method that belongs to (and is already tested in) CNA itself, not anything
// MonoGame.Extended adds.
#include "CNA/Extended/RectangleF.hpp"

#include "CNA/Extended/Matrix3x2.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(RectangleFTests, NormalizeWithNegativeWidthAndHeightAdjustsPositionAndMakesDimensionsPositive)
    {
        RectangleF rectangle(32, 32, -32, -32);
        rectangle.Normalize();

        const RectangleF expected(0, 0, 32, 32);
        EXPECT_EQ(expected, rectangle);
    }

    TEST(RectangleFTests, NormalizeWithNegativeWidthOnlyAdjustsXAndMakesWidthPositive)
    {
        RectangleF rectangle(100, 50, -20, 30);
        rectangle.Normalize();

        const RectangleF expected(80, 50, 20, 30);
        EXPECT_EQ(expected, rectangle);
    }

    TEST(RectangleFTests, NormalizeWithNegativeHeightOnlyAdjustsYAndMakesHeightPositive)
    {
        RectangleF rectangle(50, 100, 30, -20);
        rectangle.Normalize();

        const RectangleF expected(50, 80, 30, 20);
        EXPECT_EQ(expected, rectangle);
    }

    TEST(RectangleFTests, NormalizeWithPositiveDimensionsDoesNotModifyRectangle)
    {
        RectangleF rectangle(10, 20, 30, 40);
        rectangle.Normalize();

        const RectangleF expected(10, 20, 30, 40);
        EXPECT_EQ(expected, rectangle);
    }

    TEST(RectangleFTests, NormalizeStaticMethodReturnsNormalizedRectangle)
    {
        const RectangleF rectangle(32, 32, -32, -32);

        const RectangleF actual = RectangleF::Normalize(rectangle);
        const RectangleF expected(0, 0, 32, 32);

        EXPECT_EQ(expected, actual);
    }

    TEST(RectangleFTests, NormalizeRefOutMethodOutputsNormalizedRectangle)
    {
        const RectangleF rectangle(32, 32, -32, -32);

        RectangleF actual;
        RectangleF::Normalize(rectangle, actual);
        const RectangleF expected(0, 0, 32, 32);

        EXPECT_EQ(expected, actual);
    }

    // GitHub issue #747/#793 regression test.
    TEST(RectangleFIssue747Tests, NormalizeFixesIntersectionIssueFromGitHubIssue747)
    {
        RectangleF shape1(32, 32, -32, -32);
        RectangleF shape2(16, 16, -32, -32);

        shape1.Normalize();
        shape2.Normalize();

        EXPECT_TRUE(shape1.Intersects(shape2));
    }

    TEST(RectangleFPrimitivesTests, PassVector2AsConstructorParameterTest)
    {
        const RectangleF rect1(Vector2(0, 0), SizeF(12.34f, 56.78f));
        const RectangleF rect2(Vector2(0, 0), Vector2(12.34f, 56.78f));

        EXPECT_EQ(rect1, rect2);
    }

    TEST(RectangleFPrimitivesTests, PassPointAsConstructorParameterTest)
    {
        const RectangleF rect1(Vector2(0, 0), SizeF(12, 56));
        const RectangleF rect2(Vector2(0, 0), SizeF(12, 56));

        EXPECT_EQ(rect1, rect2);
    }

    TEST(RectangleFTransformTests, CenterPointIsNotTranslated)
    {
        const RectangleF rectangle(Vector2(0, 0), SizeF(20, 30));
        Matrix3x2 transform = Matrix3x2::Identity;

        const RectangleF result = RectangleF::Transform(rectangle, transform);

        EXPECT_EQ(result.getCenterProperty(), Vector2(10, 15));
    }

    TEST(RectangleFTransformTests, CenterPointIsTranslated)
    {
        const RectangleF rectangle(Vector2(0, 0), SizeF(20, 30));
        Matrix3x2 transform = Matrix3x2::CreateTranslation(1, 2);

        const RectangleF result = RectangleF::Transform(rectangle, transform);

        EXPECT_EQ(result.getCenterProperty(), Vector2(11, 17));
    }

    TEST(RectangleFTransformTests, SizeIsNotChangedByIdentityTransform)
    {
        const RectangleF rectangle(Vector2(0, 0), SizeF(20, 30));
        Matrix3x2 transform = Matrix3x2::Identity;

        const RectangleF result = RectangleF::Transform(rectangle, transform);

        EXPECT_EQ(result.getSizeProperty(), SizeF(20, 30));
    }

    TEST(RectangleFTransformTests, SizeIsNotChangedByTranslation)
    {
        const RectangleF rectangle(Vector2(0, 0), SizeF(20, 30));
        Matrix3x2 transform = Matrix3x2::CreateTranslation(1, 2);

        const RectangleF result = RectangleF::Transform(rectangle, transform);

        EXPECT_EQ(result.getSizeProperty(), SizeF(20, 30));
    }

    TEST(RectangleFTransformTests, AppliesRotationAndTranslation)
    {
        const RectangleF rectangle(Vector2(0, 0), SizeF(2, 4));
        Matrix3x2 transform = Matrix3x2::CreateRotationZ(Microsoft::Xna::Framework::MathHelper::PiOver2) * Matrix3x2::CreateTranslation(10, 20);

        const RectangleF result = RectangleF::Transform(rectangle, transform);

        constexpr float tolerance = 1e-5f;
        EXPECT_NEAR(result.getCenterProperty().X, -2 + 10, tolerance);
        EXPECT_NEAR(result.getCenterProperty().Y, 1 + 20, tolerance);
        EXPECT_NEAR(result.getSizeProperty().Width, 4, tolerance);
        EXPECT_NEAR(result.getSizeProperty().Height, 2, tolerance);
    }

    TEST(RectangleFTests, CreateFromPointsComputesBoundingRectangle)
    {
        const std::vector<Vector2> points = {Vector2(3, 4), Vector2(1, 8), Vector2(6, 2)};
        const RectangleF result = RectangleF::CreateFrom(points);

        const RectangleF expected(1, 2, 5, 6);
        EXPECT_EQ(expected, result);
    }

    TEST(RectangleFTests, UpdateFromPointsMutatesRectangle)
    {
        const std::vector<Vector2> points = {Vector2(3, 4), Vector2(1, 8), Vector2(6, 2)};
        RectangleF rectangle;
        rectangle.UpdateFromPoints(points);

        const RectangleF expected(1, 2, 5, 6);
        EXPECT_EQ(expected, rectangle);
    }

    TEST(RectangleFTests, SquaredDistanceToIsZeroWhenPointInside)
    {
        const RectangleF rectangle(0, 0, 10, 10);
        EXPECT_FLOAT_EQ(rectangle.SquaredDistanceTo(Vector2(5, 5)), 0.0f);
    }

    TEST(RectangleFTests, DistanceToMatchesSquareRootOfSquaredDistance)
    {
        const RectangleF rectangle(0, 0, 10, 10);
        EXPECT_FLOAT_EQ(rectangle.DistanceTo(Vector2(13, 0)), 3.0f);
    }

    TEST(RectangleFTests, ClosestPointToClampsToRectangle)
    {
        const RectangleF rectangle(0, 0, 10, 10);
        EXPECT_EQ(rectangle.ClosestPointTo(Vector2(-5, 20)), Vector2(0, 10));
    }
}
