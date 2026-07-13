// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for Size2JsonConverter. Fresh tests below.
#include "CNA/Extended/Serialization/Json/Size2JsonConverter.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Serialization::Json
{
    TEST(Size2JsonConverterTests, ReadsTwoComponentDelimitedString)
    {
        const nlohmann::ordered_json j = "3.5 4.5";
        const SizeF actual = j.get<SizeF>();
        EXPECT_FLOAT_EQ(actual.Width, 3.5f);
        EXPECT_FLOAT_EQ(actual.Height, 4.5f);
    }

    TEST(Size2JsonConverterTests, ReadsSingleValueAsSquare)
    {
        const nlohmann::ordered_json j = "6";
        const SizeF actual = j.get<SizeF>();
        EXPECT_FLOAT_EQ(actual.Width, 6.0f);
        EXPECT_FLOAT_EQ(actual.Height, 6.0f);
    }

    TEST(Size2JsonConverterTests, ThrowsOnInvalidComponentCount)
    {
        const nlohmann::ordered_json j = "1 2 3";
        EXPECT_THROW((void)j.get<SizeF>(), std::invalid_argument);
    }

    TEST(Size2JsonConverterTests, RoundTripPreservesValue)
    {
        const SizeF original(12.0f, 34.0f);
        const nlohmann::ordered_json j = original;
        const SizeF actual = j.get<SizeF>();
        EXPECT_FLOAT_EQ(actual.Width, original.Width);
        EXPECT_FLOAT_EQ(actual.Height, original.Height);
    }
}
