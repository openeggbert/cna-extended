// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/Primitives/
// OrientedRectangleTests.cs, including the nested `Transform` test class (9 tests), now
// unblocked since OrientedRectangle::Transform landed (task 22). The self-contained
// Intersects(...) SAT test has no upstream coverage, so fresh tests were written for it.
//
// Upstream's `Points_are_translated` and `Applies_rotation_and_translation` tests use its
// `CollectionAssert.Equal` helper (tests/MonoGame.Extended.Tests/CollectionAssert.cs), which is
// an order-INSENSITIVE containment check (same count + every actual element present in
// expected), not a sequence comparison -- unlike gtest's `EXPECT_EQ` on a std::vector, which is
// ordered. `ExpectUnorderedPointsEqual` below replicates that exact upstream semantics so these
// two tests are a faithful port of what upstream actually asserts, not just its literal syntax.
#include "CNA/Extended/OrientedRectangle.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <algorithm>
#include <gtest/gtest.h>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::MathHelper;

    namespace
    {
        void ExpectUnorderedPointsEqual(const std::vector<Vector2>& actual, const std::vector<Vector2>& expected)
        {
            ASSERT_EQ(actual.size(), expected.size()) << "The number of items in the collections does not match.";
            for (const Vector2& point : actual)
            {
                EXPECT_NE(std::find(expected.begin(), expected.end(), point), expected.end());
            }
        }
    }

    TEST(OrientedRectangleTests, InitializesOrientedRectangle)
    {
        const OrientedRectangle rectangle(Vector2(1, 2), SizeF(3, 4), Matrix3x2(5, 6, 7, 8, 9, 10));

        EXPECT_EQ(rectangle.Center, Vector2(1, 2));
        EXPECT_EQ(rectangle.Radii, Vector2(3, 4));
        EXPECT_EQ(rectangle.Orientation, Matrix3x2(5, 6, 7, 8, 9, 10));

        const std::vector<Vector2> expectedPoints = {
            Vector2(-3, -2),
            Vector2(-33, -38),
            Vector2(23, 26),
            Vector2(53, 62),
        };
        EXPECT_EQ(rectangle.getPointsProperty(), expectedPoints);
    }

    TEST(OrientedRectangleTests, EqualsComparisonEmptyComparedWithEmptyIsTrue)
    {
        const OrientedRectangle first(Vector2::Zero, SizeF::Empty, Matrix3x2::Identity);
        const OrientedRectangle second(Vector2::Zero, SizeF::Empty, Matrix3x2::Identity);

        EXPECT_TRUE(first == second);
        EXPECT_FALSE(first != second);
    }

    TEST(OrientedRectangleTests, EqualsComparisonInitializedComparedWithInitializedIsTrue)
    {
        const OrientedRectangle first(Vector2(1, 2), SizeF(3, 4), Matrix3x2(5, 6, 7, 8, 9, 10));
        const OrientedRectangle second(Vector2(1, 2), SizeF(3, 4), Matrix3x2(5, 6, 7, 8, 9, 10));

        EXPECT_TRUE(first == second);
        EXPECT_FALSE(first != second);
    }

    TEST(OrientedRectangleTests, GetHashCodeMatchesForEqualInstances)
    {
        const OrientedRectangle first(Vector2(1, 2), SizeF(3, 4), Matrix3x2(5, 6, 7, 8, 9, 10));
        const OrientedRectangle second(Vector2(1, 2), SizeF(3, 4), Matrix3x2(5, 6, 7, 8, 9, 10));

        EXPECT_EQ(first.GetHashCode(), second.GetHashCode());
    }

    TEST(OrientedRectangleTests, PositionGetterMatchesUpstreamFormula)
    {
        const OrientedRectangle rectangle(Vector2(1, 2), SizeF(3, 4), Matrix3x2::Identity);
        EXPECT_EQ(rectangle.getPositionProperty(), Vector2::Transform(-rectangle.Radii, rectangle.Orientation) + rectangle.Center);
    }

    TEST(OrientedRectangleTests, PositionSetterThrows)
    {
        OrientedRectangle rectangle(Vector2::Zero, SizeF::Empty, Matrix3x2::Identity);
        EXPECT_THROW(rectangle.setPositionProperty(Vector2::Zero), std::logic_error);
    }

    TEST(OrientedRectangleTests, ConvertsFromAxisAlignedRectangleF)
    {
        const RectangleF source(0.0f, 0.0f, 10.0f, 20.0f);
        const OrientedRectangle rectangle(source);

        EXPECT_EQ(rectangle.Center, Vector2(5.0f, 10.0f));
        EXPECT_EQ(rectangle.Radii, Vector2(5.0f, 10.0f));
        EXPECT_EQ(rectangle.Orientation, Matrix3x2::Identity);
    }

    TEST(OrientedRectangleTests, IntersectsDetectsOverlappingRectangles)
    {
        const OrientedRectangle a(Vector2::Zero, SizeF(5, 5), Matrix3x2::Identity);
        const OrientedRectangle b(Vector2(4, 0), SizeF(5, 5), Matrix3x2::Identity);

        const OrientedRectangleIntersection result = OrientedRectangle::Intersects(a, b);
        EXPECT_TRUE(result.Intersects);
    }

    TEST(OrientedRectangleTests, IntersectsDetectsNonOverlappingRectangles)
    {
        const OrientedRectangle a(Vector2::Zero, SizeF(5, 5), Matrix3x2::Identity);
        const OrientedRectangle b(Vector2(100, 0), SizeF(5, 5), Matrix3x2::Identity);

        const OrientedRectangleIntersection result = OrientedRectangle::Intersects(a, b);
        EXPECT_FALSE(result.Intersects);
    }

    TEST(OrientedRectangleTransformTests, CenterPointIsNotTranslated)
    {
        const OrientedRectangle rectangle(Vector2(1, 2), SizeF(), Matrix3x2::Identity);
        Matrix3x2 transform = Matrix3x2::Identity;

        const OrientedRectangle result = OrientedRectangle::Transform(rectangle, transform);

        EXPECT_EQ(result.Center, Vector2(1, 2));
    }

    TEST(OrientedRectangleTransformTests, CenterPointIsTranslated)
    {
        const OrientedRectangle rectangle(Vector2(0, 0), SizeF(), Matrix3x2());
        Matrix3x2 transform = Matrix3x2::CreateTranslation(1, 2);

        const OrientedRectangle result = OrientedRectangle::Transform(rectangle, transform);

        EXPECT_EQ(result.Center, Vector2(1, 2));
    }

    TEST(OrientedRectangleTransformTests, RadiiIsNotChangedByIdentityTransform)
    {
        const OrientedRectangle rectangle(Vector2(), SizeF(10, 20), Matrix3x2());
        Matrix3x2 transform = Matrix3x2::Identity;

        const OrientedRectangle result = OrientedRectangle::Transform(rectangle, transform);

        EXPECT_EQ(result.Radii, Vector2(10, 20));
    }

    TEST(OrientedRectangleTransformTests, RadiiIsNotChangedByTranslation)
    {
        const OrientedRectangle rectangle(Vector2(1, 2), SizeF(10, 20), Matrix3x2());
        Matrix3x2 transform = Matrix3x2::CreateTranslation(1, 2);

        const OrientedRectangle result = OrientedRectangle::Transform(rectangle, transform);

        EXPECT_EQ(result.Radii, Vector2(10, 20));
    }

    TEST(OrientedRectangleTransformTests, OrientationIsRotated45DegreesLeft)
    {
        const OrientedRectangle rectangle(Vector2(), SizeF(), Matrix3x2::Identity);
        Matrix3x2 transform = Matrix3x2::CreateRotationZ(MathHelper::PiOver4);

        const OrientedRectangle result = OrientedRectangle::Transform(rectangle, transform);

        EXPECT_EQ(result.Center, Vector2());
        EXPECT_EQ(result.Radii, Vector2());
        EXPECT_EQ(result.Orientation, Matrix3x2::CreateRotationZ(MathHelper::PiOver4));
    }

    TEST(OrientedRectangleTransformTests, OrientationIsRotatedTo45DegreesFrom180)
    {
        const OrientedRectangle rectangle(Vector2(), SizeF(), Matrix3x2::CreateRotationZ(MathHelper::Pi));
        Matrix3x2 transform = Matrix3x2::CreateRotationZ(-3 * MathHelper::PiOver4);
        const Matrix3x2 expectedOrientation = Matrix3x2::CreateRotationZ(MathHelper::PiOver4);

        const OrientedRectangle result = OrientedRectangle::Transform(rectangle, transform);

        EXPECT_EQ(result.Center, Vector2());
        EXPECT_EQ(result.Radii, Vector2());
        constexpr float tolerance = 1e-5f;
        EXPECT_NEAR(expectedOrientation.M11, result.Orientation.M11, tolerance);
        EXPECT_NEAR(expectedOrientation.M12, result.Orientation.M12, tolerance);
        EXPECT_NEAR(expectedOrientation.M21, result.Orientation.M21, tolerance);
        EXPECT_NEAR(expectedOrientation.M22, result.Orientation.M22, tolerance);
        EXPECT_NEAR(expectedOrientation.M31, result.Orientation.M31, tolerance);
        EXPECT_NEAR(expectedOrientation.M32, result.Orientation.M32, tolerance);
    }

    TEST(OrientedRectangleTransformTests, PointsAreSameAsCenter)
    {
        const OrientedRectangle rectangle(Vector2(1, 2), SizeF(), Matrix3x2::Identity);
        Matrix3x2 transform = Matrix3x2::Identity;

        const OrientedRectangle result = OrientedRectangle::Transform(rectangle, transform);

        const std::vector<Vector2> expected = {Vector2(1, 2), Vector2(1, 2), Vector2(1, 2), Vector2(1, 2)};
        EXPECT_EQ(result.getPointsProperty(), expected);
    }

    TEST(OrientedRectangleTransformTests, PointsAreTranslated)
    {
        const OrientedRectangle rectangle(Vector2(0, 0), SizeF(2, 4), Matrix3x2::Identity);
        Matrix3x2 transform = Matrix3x2::CreateTranslation(10, 20);

        const OrientedRectangle result = OrientedRectangle::Transform(rectangle, transform);

        const std::vector<Vector2> expected = {Vector2(8, 16), Vector2(8, 24), Vector2(12, 24), Vector2(12, 16)};
        ExpectUnorderedPointsEqual(result.getPointsProperty(), expected);
    }

    TEST(OrientedRectangleTransformTests, AppliesRotationAndTranslation)
    {
        const OrientedRectangle rectangle(Vector2(1, 2), SizeF(2, 4), Matrix3x2::Identity);
        Matrix3x2 transform = Matrix3x2::CreateRotationZ(MathHelper::PiOver2) * Matrix3x2::CreateTranslation(10, 20);

        const OrientedRectangle result = OrientedRectangle::Transform(rectangle, transform);

        constexpr float tolerance = 1e-5f;
        EXPECT_NEAR(result.Center.X, 8, tolerance);
        EXPECT_NEAR(result.Center.Y, 21, tolerance);
        EXPECT_NEAR(result.Radii.X, 2, tolerance);
        EXPECT_NEAR(result.Radii.Y, 4, tolerance);
        EXPECT_EQ(result.Orientation, Matrix3x2::CreateRotationZ(MathHelper::PiOver2));

        const std::vector<Vector2> expectedPoints = {Vector2(4, 23), Vector2(4, 19), Vector2(12, 19), Vector2(12, 23)};
        ExpectUnorderedPointsEqual(result.getPointsProperty(), expectedPoints);
    }

    TEST(OrientedRectangleTests, BoundingRectanglePropertyMatchesRectangleFConversion)
    {
        const OrientedRectangle rectangle(Vector2(5, 5), SizeF(5, 5), Matrix3x2::Identity);
        EXPECT_EQ(rectangle.getBoundingRectangleProperty(), static_cast<RectangleF>(rectangle));
    }

    TEST(OrientedRectangleTests, RectangleFConversionOfAxisAlignedRectangleMatchesOriginal)
    {
        const RectangleF source(0.0f, 0.0f, 10.0f, 20.0f);
        const OrientedRectangle rectangle(source);

        const auto converted = static_cast<RectangleF>(rectangle);
        EXPECT_FLOAT_EQ(converted.X, source.X);
        EXPECT_FLOAT_EQ(converted.Y, source.Y);
        EXPECT_FLOAT_EQ(converted.Width, source.Width);
        EXPECT_FLOAT_EQ(converted.Height, source.Height);
    }
}
