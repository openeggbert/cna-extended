// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for PrimitiveTypeExtensions. Fresh tests below.
#include "CNA/Extended/Graphics/PrimitiveTypeExtensions.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

namespace CNA::Extended::Graphics
{
    TEST(PrimitiveTypeExtensionsTests, GetPrimitivesCountForEachType)
    {
        EXPECT_EQ(GetPrimitivesCount(PrimitiveType::LineStrip, 5), 4);
        EXPECT_EQ(GetPrimitivesCount(PrimitiveType::LineList, 6), 3);
        EXPECT_EQ(GetPrimitivesCount(PrimitiveType::TriangleStrip, 5), 3);
        EXPECT_EQ(GetPrimitivesCount(PrimitiveType::TriangleList, 9), 3);
    }

    TEST(PrimitiveTypeExtensionsTests, GetVerticesCountForEachType)
    {
        EXPECT_EQ(GetVerticesCount(PrimitiveType::LineStrip, 4), 5);
        EXPECT_EQ(GetVerticesCount(PrimitiveType::LineList, 3), 6);
        EXPECT_EQ(GetVerticesCount(PrimitiveType::TriangleStrip, 3), 5);
        EXPECT_EQ(GetVerticesCount(PrimitiveType::TriangleList, 3), 9);
    }

    TEST(PrimitiveTypeExtensionsTests, GetPrimitivesCountAndGetVerticesCountAreInverses)
    {
        for (const PrimitiveType type : {PrimitiveType::LineStrip, PrimitiveType::LineList, PrimitiveType::TriangleStrip, PrimitiveType::TriangleList})
        {
            const int vertices = GetVerticesCount(type, 4);
            EXPECT_EQ(GetPrimitivesCount(type, vertices), 4);
        }
    }
}
