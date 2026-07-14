// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for SizeJsonConverter. Fresh tests below, mirroring
// Size2JsonConverterTests.cpp's structure for the SizeF converter.
#include "CNA/Extended/Serialization/Json/SizeJsonConverter.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Serialization::Json
{
    TEST(SizeJsonConverterTests, ReadsTwoComponentDelimitedString)
    {
        const nlohmann::ordered_json j = "3 4";
        const Size actual = j.get<Size>();
        EXPECT_EQ(actual.Width, 3);
        EXPECT_EQ(actual.Height, 4);
    }

    TEST(SizeJsonConverterTests, ReadsSingleValueAsSquare)
    {
        const nlohmann::ordered_json j = "6";
        const Size actual = j.get<Size>();
        EXPECT_EQ(actual.Width, 6);
        EXPECT_EQ(actual.Height, 6);
    }

    TEST(SizeJsonConverterTests, ThrowsOnInvalidComponentCount)
    {
        const nlohmann::ordered_json j = "1 2 3";
        EXPECT_THROW((void)j.get<Size>(), std::invalid_argument);
    }

    TEST(SizeJsonConverterTests, RoundTripPreservesValue)
    {
        const Size original(12, 34);
        const nlohmann::ordered_json j = original;
        const Size actual = j.get<Size>();
        EXPECT_EQ(actual.Width, original.Width);
        EXPECT_EQ(actual.Height, original.Height);
    }
}
