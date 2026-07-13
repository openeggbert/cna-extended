// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's
// tests/MonoGame.Extended.Tests/Tilemaps/Properties/TilemapPropertyValueTests.cs. Each of
// upstream's 6 [Theory] blocks (one per As* accessor, parameterized over the *other* 6 types)
// becomes its own TEST_P suite, matching this project's established parameterized-test
// convention (see HslColorTests.cpp).
#include "CNA/Extended/Tilemaps/TilemapPropertyValue.hpp"

#include "System/InvalidOperationException.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Color;

    namespace
    {
        TilemapPropertyValue CreateValueOfType(TilemapPropertyType type)
        {
            switch (type)
            {
                case TilemapPropertyType::String:
                    return TilemapPropertyValue::CreateString("test");
                case TilemapPropertyType::Int:
                    return TilemapPropertyValue::CreateInt(42);
                case TilemapPropertyType::Float:
                    return TilemapPropertyValue::CreateFloat(3.14f);
                case TilemapPropertyType::Bool:
                    return TilemapPropertyValue::CreateBool(true);
                case TilemapPropertyType::Color:
                    return TilemapPropertyValue::CreateColor(::Microsoft::Xna::Framework::Color(255, 0, 0, 255));
                case TilemapPropertyType::File:
                    return TilemapPropertyValue::CreateFile("image.png");
                case TilemapPropertyType::Object:
                    return TilemapPropertyValue::CreateObject(123);
            }
            throw std::invalid_argument("type");
        }
    }

    class AsStringWrongTypeTests : public ::testing::TestWithParam<TilemapPropertyType>
    {
    };
    TEST_P(AsStringWrongTypeTests, ThrowsInvalidOperationException)
    {
        TilemapPropertyValue value = CreateValueOfType(GetParam());
        EXPECT_THROW((void)value.AsString(), System::InvalidOperationException);
    }
    INSTANTIATE_TEST_SUITE_P(TilemapPropertyValueTests, AsStringWrongTypeTests,
        ::testing::Values(TilemapPropertyType::Int, TilemapPropertyType::Float, TilemapPropertyType::Bool, TilemapPropertyType::Color,
            TilemapPropertyType::File, TilemapPropertyType::Object));

    class AsIntWrongTypeTests : public ::testing::TestWithParam<TilemapPropertyType>
    {
    };
    TEST_P(AsIntWrongTypeTests, ThrowsInvalidOperationException)
    {
        TilemapPropertyValue value = CreateValueOfType(GetParam());
        EXPECT_THROW((void)value.AsInt(), System::InvalidOperationException);
    }
    INSTANTIATE_TEST_SUITE_P(TilemapPropertyValueTests, AsIntWrongTypeTests,
        ::testing::Values(TilemapPropertyType::String, TilemapPropertyType::Float, TilemapPropertyType::Bool, TilemapPropertyType::Color,
            TilemapPropertyType::File, TilemapPropertyType::Object));

    class AsFloatWrongTypeTests : public ::testing::TestWithParam<TilemapPropertyType>
    {
    };
    TEST_P(AsFloatWrongTypeTests, ThrowsInvalidOperationException)
    {
        TilemapPropertyValue value = CreateValueOfType(GetParam());
        EXPECT_THROW((void)value.AsFloat(), System::InvalidOperationException);
    }
    INSTANTIATE_TEST_SUITE_P(TilemapPropertyValueTests, AsFloatWrongTypeTests,
        ::testing::Values(TilemapPropertyType::String, TilemapPropertyType::Int, TilemapPropertyType::Bool, TilemapPropertyType::Color,
            TilemapPropertyType::File, TilemapPropertyType::Object));

    class AsBoolWrongTypeTests : public ::testing::TestWithParam<TilemapPropertyType>
    {
    };
    TEST_P(AsBoolWrongTypeTests, ThrowsInvalidOperationException)
    {
        TilemapPropertyValue value = CreateValueOfType(GetParam());
        EXPECT_THROW((void)value.AsBool(), System::InvalidOperationException);
    }
    INSTANTIATE_TEST_SUITE_P(TilemapPropertyValueTests, AsBoolWrongTypeTests,
        ::testing::Values(TilemapPropertyType::String, TilemapPropertyType::Int, TilemapPropertyType::Float, TilemapPropertyType::Color,
            TilemapPropertyType::File, TilemapPropertyType::Object));

    class AsColorWrongTypeTests : public ::testing::TestWithParam<TilemapPropertyType>
    {
    };
    TEST_P(AsColorWrongTypeTests, ThrowsInvalidOperationException)
    {
        TilemapPropertyValue value = CreateValueOfType(GetParam());
        EXPECT_THROW((void)value.AsColor(), System::InvalidOperationException);
    }
    INSTANTIATE_TEST_SUITE_P(TilemapPropertyValueTests, AsColorWrongTypeTests,
        ::testing::Values(TilemapPropertyType::String, TilemapPropertyType::Int, TilemapPropertyType::Float, TilemapPropertyType::Bool,
            TilemapPropertyType::File, TilemapPropertyType::Object));

    class AsFileWrongTypeTests : public ::testing::TestWithParam<TilemapPropertyType>
    {
    };
    TEST_P(AsFileWrongTypeTests, ThrowsInvalidOperationException)
    {
        TilemapPropertyValue value = CreateValueOfType(GetParam());
        EXPECT_THROW((void)value.AsFile(), System::InvalidOperationException);
    }
    INSTANTIATE_TEST_SUITE_P(TilemapPropertyValueTests, AsFileWrongTypeTests,
        ::testing::Values(TilemapPropertyType::String, TilemapPropertyType::Int, TilemapPropertyType::Float, TilemapPropertyType::Bool,
            TilemapPropertyType::Color, TilemapPropertyType::Object));

    class AsObjectWrongTypeTests : public ::testing::TestWithParam<TilemapPropertyType>
    {
    };
    TEST_P(AsObjectWrongTypeTests, ThrowsInvalidOperationException)
    {
        TilemapPropertyValue value = CreateValueOfType(GetParam());
        EXPECT_THROW((void)value.AsObject(), System::InvalidOperationException);
    }
    INSTANTIATE_TEST_SUITE_P(TilemapPropertyValueTests, AsObjectWrongTypeTests,
        ::testing::Values(TilemapPropertyType::String, TilemapPropertyType::Int, TilemapPropertyType::Float, TilemapPropertyType::Bool,
            TilemapPropertyType::Color, TilemapPropertyType::File));

    // Fresh coverage: no upstream test verifies the *correct*-type path (only wrong-type
    // exception behavior). Added since this is otherwise-untested happy-path logic.
    TEST(TilemapPropertyValueTests, CorrectTypeAccessorsReturnStoredValue)
    {
        EXPECT_EQ(TilemapPropertyValue::CreateString("hello").AsString(), "hello");
        EXPECT_EQ(TilemapPropertyValue::CreateInt(7).AsInt(), 7);
        EXPECT_FLOAT_EQ(TilemapPropertyValue::CreateFloat(1.5f).AsFloat(), 1.5f);
        EXPECT_TRUE(TilemapPropertyValue::CreateBool(true).AsBool());
        const Color color(10, 20, 30, 255);
        EXPECT_EQ(TilemapPropertyValue::CreateColor(color).AsColor().getPackedValueProperty(), color.getPackedValueProperty());
        EXPECT_EQ(TilemapPropertyValue::CreateFile("a.png").AsFile(), "a.png");
        EXPECT_EQ(TilemapPropertyValue::CreateObject(99).AsObject(), 99);
    }
}
