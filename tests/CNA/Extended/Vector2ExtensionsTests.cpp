// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Vector2ExtensionsTests.cs.
// Upstream's `#if FNA || KNI` Rotate tests are ported unconditionally (CNA mirrors FNA, matching
// Vector2Extensions.hpp's own precedent); the non-FNA branch of Vector2_ToAngle_Test is dropped
// as dead code for this project (CNA always takes the FNA branch).
#include "CNA/Extended/Vector2Extensions.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::MathHelper;

    TEST(Vector2ExtensionsTests, Vector2EqualsWithTolerenceTest)
    {
        const Vector2 a(1.0f, 1.0f);
        const Vector2 b(1.0000001f, 1.0000001f);

        EXPECT_FALSE(a == b);
        EXPECT_TRUE(EqualsWithTolerence(a, b));
    }

    TEST(Vector2ExtensionsTests, Vector2NormalizedCopyTest)
    {
        const Vector2 a(5, -10);
        const Vector2 b = NormalizedCopy(a);

        EXPECT_TRUE(EqualsWithTolerence(Vector2(0.4472136f, -0.8944272f), b));
    }

    TEST(Vector2ExtensionsTests, Vector2PerpendicularTest)
    {
        // http://mathworld.wolfram.com/PerpendicularVector.html
        const Vector2 a(5, -10);
        const Vector2 b = PerpendicularClockwise(a);
        const Vector2 c = PerpendicularCounterClockwise(a);

        EXPECT_EQ(b, Vector2(-10, -5));
        EXPECT_EQ(c, Vector2(10, 5));
    }

    TEST(Vector2ExtensionsTests, Vector2Rotate90DegreesTest)
    {
        const Vector2 a(0, -10);
        const Vector2 b = Rotate(a, MathHelper::ToRadians(90));

        EXPECT_TRUE(EqualsWithTolerence(Vector2(10, 0), b));
    }

    TEST(Vector2ExtensionsTests, Vector2Rotate360DegreesTest)
    {
        const Vector2 a(0, 10);
        const Vector2 b = Rotate(a, MathHelper::ToRadians(360));

        EXPECT_TRUE(EqualsWithTolerence(Vector2(0, 10), b));
    }

    TEST(Vector2ExtensionsTests, Vector2Rotate45DegreesTest)
    {
        const Vector2 a(0, -10);
        const Vector2 b = Rotate(a, MathHelper::ToRadians(45));

        EXPECT_TRUE(EqualsWithTolerence(Vector2(7.071068f, -7.071068f), b));
    }

    TEST(Vector2ExtensionsTests, Vector2TruncateTest)
    {
        const Vector2 a(10, 10);
        const Vector2 b = Truncate(a, 5);

        EXPECT_NEAR(b.Length(), 5.0f, 0.001f);
    }

    TEST(Vector2ExtensionsTests, Vector2IsNaNTest)
    {
        const Vector2 a(std::numeric_limits<float>::quiet_NaN(), 10);
        const Vector2 b(10, std::numeric_limits<float>::quiet_NaN());
        const Vector2 c(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN());
        const Vector2 d(10, 10);

        EXPECT_TRUE(IsNaN(a));
        EXPECT_TRUE(IsNaN(b));
        EXPECT_TRUE(IsNaN(c));
        EXPECT_FALSE(IsNaN(d));
    }

    TEST(Vector2ExtensionsTests, Vector2ToAngleTest)
    {
        const Vector2 a(0, -10);
        const Vector2 b(10, 0);
        const Vector2 c = Rotate(-Vector2::UnitY, MathHelper::ToRadians(45));

        EXPECT_FLOAT_EQ(ToAngle(a), MathHelper::ToRadians(0));
        EXPECT_FLOAT_EQ(ToAngle(b), MathHelper::ToRadians(90));
        EXPECT_FLOAT_EQ(ToAngle(c), MathHelper::ToRadians(45));
    }

    // Additional coverage beyond upstream (no dedicated PerpDot/Dot/ScalarProjectOnto/ProjectOnto/
    // SetX/SetY/Translate/ToSize/ToAbsoluteSize/Round tests exist in Vector2ExtensionsTests.cs).
    TEST(Vector2ExtensionsTests, PerpDotOfPerpendicularVectorsIsNonZero)
    {
        EXPECT_FLOAT_EQ(PerpDot(Vector2::UnitX, Vector2::UnitY), 1.0f);
        EXPECT_FLOAT_EQ(PerpDot(Vector2::UnitX, Vector2::UnitX), 0.0f);
    }

    TEST(Vector2ExtensionsTests, DotOfOrthogonalVectorsIsZero)
    {
        EXPECT_FLOAT_EQ(Dot(Vector2::UnitX, Vector2::UnitY), 0.0f);
        EXPECT_FLOAT_EQ(Dot(Vector2(2.0f, 0.0f), Vector2(3.0f, 0.0f)), 6.0f);
    }

    TEST(Vector2ExtensionsTests, SetXSetYReplaceOnlyOneComponent)
    {
        const Vector2 v(1.0f, 2.0f);
        EXPECT_EQ(SetX(v, 9.0f), Vector2(9.0f, 2.0f));
        EXPECT_EQ(SetY(v, 9.0f), Vector2(1.0f, 9.0f));
    }

    TEST(Vector2ExtensionsTests, TranslateAddsComponents)
    {
        EXPECT_EQ(Translate(Vector2(1.0f, 2.0f), 3.0f, 4.0f), Vector2(4.0f, 6.0f));
    }

    TEST(Vector2ExtensionsTests, ToSizeAndToAbsoluteSizeMatchExpectations)
    {
        const SizeF size = ToSize(Vector2(3.0f, -4.0f));
        EXPECT_FLOAT_EQ(size.Width, 3.0f);
        EXPECT_FLOAT_EQ(size.Height, -4.0f);

        const SizeF absSize = ToAbsoluteSize(Vector2(3.0f, -4.0f));
        EXPECT_FLOAT_EQ(absSize.Width, 3.0f);
        EXPECT_FLOAT_EQ(absSize.Height, 4.0f);
    }

    TEST(Vector2ExtensionsTests, RoundRoundsToNearestInteger)
    {
        EXPECT_EQ(Round(Vector2(1.4f, 1.6f)), Vector2(1.0f, 2.0f));
    }

    TEST(Vector2ExtensionsTests, ScalarProjectOntoAndProjectOntoAgree)
    {
        const Vector2 v1(3.0f, 0.0f);
        const Vector2 v2(0.0f, 5.0f);
        EXPECT_FLOAT_EQ(ScalarProjectOnto(v1, v2), 0.0f);

        const Vector2 projection = ProjectOnto(v1, v2);
        EXPECT_FLOAT_EQ(projection.X, 0.0f);
        EXPECT_FLOAT_EQ(projection.Y, 0.0f);
    }

    TEST(Vector2ExtensionsTests, ToPointAndBackRoundTrips)
    {
        const Point point = ToPoint(Vector2(3.0f, 4.0f));
        EXPECT_EQ(point.X, 3);
        EXPECT_EQ(point.Y, 4);

        const Vector2 back = ToVector2(point);
        EXPECT_EQ(back, Vector2(3.0f, 4.0f));
    }
}
