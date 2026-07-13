// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// ConstructorTest/InverseTest ported 1:1 from MonoGame.Extended's
// tests/MonoGame.Extended.Tests/Math/Matrix3x2.cs (upstream's only 2 tests for this type).
// Remaining coverage below is fresh, since upstream doesn't test the rest of this large type.
#include "CNA/Extended/Matrix3x2.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(Matrix3x2Tests, ConstructorTest)
    {
        const Vector2 x(1, 2);
        const Vector2 y(3, 4);
        const Vector2 z(5, 6);

        const Matrix3x2 matrix(x.X, x.Y, y.X, y.Y, z.X, z.Y);

        EXPECT_EQ(matrix.getXProperty(), x);
        EXPECT_EQ(matrix.getYProperty(), y);
        EXPECT_EQ(matrix.getZProperty(), z);
    }

    TEST(Matrix3x2Tests, InverseTest)
    {
        const Matrix3x2 posDeterminant(3, -7, 4, 2, 13, -2);
        const Matrix3x2 singular(2, 1, 4, 2, 3, -4);
        const Matrix3x2 negDeterminant(1, -5, 3, 2, 3, -4);

        const Matrix3x2 posExpected(1.0f / 17, 7.0f / 34, -2.0f / 17, 3.0f / 34, -1.0f, -5.0f / 2);
        const Matrix3x2 singularExpected = Matrix3x2::Identity;
        const Matrix3x2 negExpected(2.0f / 17, 5.0f / 17, -3.0f / 17, 1.0f / 17, -18.0f / 17, -11.0f / 17);

        EXPECT_EQ(Matrix3x2::Invert(posDeterminant), posExpected);
        EXPECT_EQ(Matrix3x2::Invert(singular), singularExpected);
        EXPECT_EQ(Matrix3x2::Invert(negDeterminant), negExpected);
    }

    TEST(Matrix3x2Tests, IdentityHasNoEffectOnTransform)
    {
        const Vector2 v(3.0f, 4.0f);
        EXPECT_EQ(Matrix3x2::Identity.Transform(v), v);
    }

    TEST(Matrix3x2Tests, CreateTranslationTranslatesAPoint)
    {
        const Matrix3x2 translation = Matrix3x2::CreateTranslation(Vector2(10.0f, 20.0f));
        EXPECT_EQ(translation.Transform(Vector2::Zero), Vector2(10.0f, 20.0f));
    }

    TEST(Matrix3x2Tests, CreateScaleScalesAPoint)
    {
        const Matrix3x2 scale = Matrix3x2::CreateScale(2.0f, 3.0f);
        EXPECT_EQ(scale.Transform(Vector2(1.0f, 1.0f)), Vector2(2.0f, 3.0f));
    }

    TEST(Matrix3x2Tests, MultiplyByIdentityIsUnchanged)
    {
        const Matrix3x2 m(1, 2, 3, 4, 5, 6);
        EXPECT_EQ(Matrix3x2::Multiply(m, Matrix3x2::Identity), m);
    }

    TEST(Matrix3x2Tests, DecomposeRecoversTranslationRotationScale)
    {
        const Matrix3x2 matrix =
            Matrix3x2::CreateScale(2.0f, 2.0f) * Matrix3x2::CreateRotationZ(0.0f) * Matrix3x2::CreateTranslation(5.0f, 6.0f);

        Vector2 translation;
        float rotation = 0.0f;
        Vector2 scale;
        matrix.Decompose(translation, rotation, scale);

        EXPECT_FLOAT_EQ(translation.X, 5.0f);
        EXPECT_FLOAT_EQ(translation.Y, 6.0f);
        EXPECT_NEAR(scale.X, 2.0f, 0.0001f);
        EXPECT_NEAR(scale.Y, 2.0f, 0.0001f);
    }

    TEST(Matrix3x2Tests, GetHashCodeMatchesForEqualInstances)
    {
        const Matrix3x2 a(1, 2, 3, 4, 5, 6);
        const Matrix3x2 b(1, 2, 3, 4, 5, 6);
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(Matrix3x2Tests, ToStringContainsAllComponents)
    {
        const Matrix3x2 matrix(1, 2, 3, 4, 5, 6);
        const std::string text = matrix.ToString();
        EXPECT_NE(text.find("M11"), std::string::npos);
        EXPECT_NE(text.find("M32"), std::string::npos);
    }

    TEST(Matrix3x2Tests, ImplicitConversionToMatrixPreservesTranslation)
    {
        const Matrix3x2 matrix3x2 = Matrix3x2::CreateTranslation(1.0f, 2.0f);
        const Matrix matrix = matrix3x2;
        EXPECT_FLOAT_EQ(matrix.M41, 1.0f);
        EXPECT_FLOAT_EQ(matrix.M42, 2.0f);
    }
}
