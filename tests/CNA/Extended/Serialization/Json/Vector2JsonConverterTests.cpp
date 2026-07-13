// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for Vector2JsonConverter. Fresh tests below.
#include "CNA/Extended/Serialization/Json/Vector2JsonConverter.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Serialization::Json
{
    using Microsoft::Xna::Framework::Vector2;

    TEST(Vector2JsonConverterTests, ReadsTwoComponentDelimitedString)
    {
        const nlohmann::ordered_json j = "1.5 2.5";
        const Vector2 actual = j.get<Vector2>();
        EXPECT_FLOAT_EQ(actual.X, 1.5f);
        EXPECT_FLOAT_EQ(actual.Y, 2.5f);
    }

    TEST(Vector2JsonConverterTests, ReadsSingleValueAsUniformComponents)
    {
        const nlohmann::ordered_json j = "4";
        const Vector2 actual = j.get<Vector2>();
        EXPECT_FLOAT_EQ(actual.X, 4.0f);
        EXPECT_FLOAT_EQ(actual.Y, 4.0f);
    }

    TEST(Vector2JsonConverterTests, ReadsJsonArray)
    {
        const nlohmann::ordered_json j = nlohmann::ordered_json::array({1.0f, 2.0f});
        const Vector2 actual = j.get<Vector2>();
        EXPECT_FLOAT_EQ(actual.X, 1.0f);
        EXPECT_FLOAT_EQ(actual.Y, 2.0f);
    }

    TEST(Vector2JsonConverterTests, ThrowsOnInvalidComponentCount)
    {
        const nlohmann::ordered_json j = "1 2 3";
        EXPECT_THROW((void)j.get<Vector2>(), std::invalid_argument);
    }

    TEST(Vector2JsonConverterTests, RoundTripPreservesValue)
    {
        const Vector2 original(3.25f, -1.75f);
        const nlohmann::ordered_json j = original;
        const Vector2 actual = j.get<Vector2>();
        EXPECT_FLOAT_EQ(actual.X, original.X);
        EXPECT_FLOAT_EQ(actual.Y, original.Y);
    }
}
