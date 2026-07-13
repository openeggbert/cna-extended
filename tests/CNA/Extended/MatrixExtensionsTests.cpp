// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream test file for MatrixExtensions.cs. These tests are
// fresh, covering Decompose's two outcomes.
#include "CNA/Extended/MatrixExtensions.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(MatrixExtensionsTests, DecomposeRecoversTranslationAndRotation)
    {
        const Matrix matrix = Matrix::CreateRotationZ(0.0f) * Matrix::CreateTranslation(5.0f, 6.0f, 0.0f);

        Vector2 position;
        float rotation = 0.0f;
        Vector2 scale;
        const bool result = Decompose(matrix, position, rotation, scale);

        EXPECT_TRUE(result);
        EXPECT_NEAR(position.X, 5.0f, 0.0001f);
        EXPECT_NEAR(position.Y, 6.0f, 0.0001f);
        EXPECT_NEAR(rotation, 0.0f, 0.0001f);
    }

    TEST(MatrixExtensionsTests, DecomposeOfIdentityYieldsZeroPositionAndRotation)
    {
        Vector2 position;
        float rotation = 0.0f;
        Vector2 scale;
        const bool result = Decompose(Matrix::getIdentityProperty(), position, rotation, scale);

        EXPECT_TRUE(result);
        EXPECT_NEAR(position.X, 0.0f, 0.0001f);
        EXPECT_NEAR(position.Y, 0.0f, 0.0001f);
        EXPECT_NEAR(rotation, 0.0f, 0.0001f);
    }
}
