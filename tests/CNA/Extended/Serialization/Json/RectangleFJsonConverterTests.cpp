// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/Serialization/RectangleFJsonConverterTest.cs.
// Upstream constructs a `JsonSerializerOptions` and explicitly registers `RectangleFJsonConverter`
// before deserializing a wrapping type -- this port's `RectangleF` converter is already globally
// active via ADL the moment ColorJsonConverter.hpp's sibling RectangleFJsonConverter.hpp is
// included (see that file's header comment), so no explicit registration step exists to test;
// deserializing the wrapping type directly is the equivalent exercise.
#include "CNA/Extended/Serialization/Json/RectangleFJsonConverter.hpp"

#include "System/Text/Json/JsonSerializer.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Serialization::Json
{
    namespace
    {
        struct TestContent
        {
            RectangleF Box;
        };

        void from_json(const nlohmann::ordered_json& j, TestContent& value)
        {
            value.Box = j.at("box").get<RectangleF>();
        }
    }

    TEST(RectangleFJsonConverterTests, DeserializesWrappingObject)
    {
        const char* const json = R"({"box": "1 1 10 10"})";
        const TestContent content = System::Text::Json::JsonSerializer::Deserialize<TestContent>(json);

        EXPECT_FLOAT_EQ(content.Box.getLeftProperty(), 1.0f);
        EXPECT_FLOAT_EQ(content.Box.getTopProperty(), 1.0f);
        EXPECT_FLOAT_EQ(content.Box.Width, 10.0f);
        EXPECT_FLOAT_EQ(content.Box.Height, 10.0f);
    }

    TEST(RectangleFJsonConverterTests, RoundTripPreservesRectangle)
    {
        const RectangleF original(2.5f, 3.5f, 40.0f, 50.0f);
        const nlohmann::ordered_json j = original;
        const RectangleF actual = j.get<RectangleF>();

        EXPECT_FLOAT_EQ(actual.X, original.X);
        EXPECT_FLOAT_EQ(actual.Y, original.Y);
        EXPECT_FLOAT_EQ(actual.Width, original.Width);
        EXPECT_FLOAT_EQ(actual.Height, original.Height);
    }
}
