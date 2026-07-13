// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's
// tests/MonoGame.Extended.Tests/Tilemaps/Properties/TilemapPropertiesTests.cs.
#include "CNA/Extended/Tilemaps/TilemapProperties.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Color;

    TEST(TilemapPropertiesTests, OverwritingPropertyUpdatesValue)
    {
        TilemapProperties properties;
        properties["key"] = TilemapPropertyValue::CreateInt(10);
        properties["key"] = TilemapPropertyValue::CreateInt(20);

        EXPECT_EQ(properties.GetInt("key"), 20);
    }

    TEST(TilemapPropertiesTests, OverwritingPropertyCanChangeType)
    {
        TilemapProperties properties;
        properties["key"] = TilemapPropertyValue::CreateInt(42);
        properties["key"] = TilemapPropertyValue::CreateString("changed");

        EXPECT_EQ(properties.GetString("key"), "changed");
    }

    TEST(TilemapPropertiesTests, TryGetValueWithExistingKeyReturnsTrue)
    {
        TilemapProperties properties;
        properties["key"] = TilemapPropertyValue::CreateString("value");

        TilemapPropertyValue value;
        const bool result = properties.TryGetValue("key", value);

        EXPECT_TRUE(result);
        EXPECT_EQ(value.getTypeProperty(), TilemapPropertyType::String);
        EXPECT_EQ(value.AsString(), "value");
    }

    TEST(TilemapPropertiesTests, TryGetValueWithNonExistingKeyReturnsFalse)
    {
        TilemapProperties properties;
        TilemapPropertyValue value;
        EXPECT_FALSE(properties.TryGetValue("key", value));
    }

    TEST(TilemapPropertiesTests, GetStringWithNonExistingKeyReturnsDefaultValue)
    {
        TilemapProperties properties;
        EXPECT_TRUE(properties.GetString("key").empty());
    }

    TEST(TilemapPropertiesTests, GetStringWithWrongTypeReturnsDefaultValue)
    {
        TilemapProperties properties;
        properties["key"] = TilemapPropertyValue::CreateInt(42);

        EXPECT_EQ(properties.GetString("key", "default"), "default");
    }

    TEST(TilemapPropertiesTests, GetIntWithNonExistingKeyReturnsDefaultValue)
    {
        TilemapProperties properties;
        EXPECT_EQ(properties.GetInt("key"), 0);
    }

    TEST(TilemapPropertiesTests, GetIntWithCustomDefaultReturnsCustomDefault)
    {
        TilemapProperties properties;
        EXPECT_EQ(properties.GetInt("key", 42), 42);
    }

    TEST(TilemapPropertiesTests, GetIntWithWrongTypeReturnsDefaultValue)
    {
        TilemapProperties properties;
        properties["key"] = TilemapPropertyValue::CreateString("test");

        EXPECT_EQ(properties.GetInt("key", 42), 42);
    }

    TEST(TilemapPropertiesTests, GetFloatWithNonExistingKeyReturnsDefaultValue)
    {
        TilemapProperties properties;
        EXPECT_FLOAT_EQ(properties.GetFloat("key"), 0.0f);
    }

    TEST(TilemapPropertiesTests, GetFloatWithCustomDefaultReturnsCustomDefault)
    {
        TilemapProperties properties;
        EXPECT_FLOAT_EQ(properties.GetFloat("key", 3.14f), 3.14f);
    }

    TEST(TilemapPropertiesTests, GetFloatWithWrongTypeReturnsDefaultValue)
    {
        TilemapProperties properties;
        properties["key"] = TilemapPropertyValue::CreateString("test");

        EXPECT_FLOAT_EQ(properties.GetFloat("key", 3.14f), 3.14f);
    }

    TEST(TilemapPropertiesTests, GetBoolWithNonExistingKeyReturnsDefaultValue)
    {
        TilemapProperties properties;
        EXPECT_FALSE(properties.GetBool("key"));
    }

    TEST(TilemapPropertiesTests, GetBoolWithCustomDefaultReturnsCustomDefault)
    {
        TilemapProperties properties;
        EXPECT_TRUE(properties.GetBool("key", true));
    }

    TEST(TilemapPropertiesTests, GetBoolWithWrongTypeReturnsDefaultValue)
    {
        TilemapProperties properties;
        properties["key"] = TilemapPropertyValue::CreateString("test");

        EXPECT_TRUE(properties.GetBool("key", true));
    }

    TEST(TilemapPropertiesTests, GetColorWithNonExistingKeyReturnsDefaultValue)
    {
        TilemapProperties properties;
        const Color result = properties.GetColor("key");
        EXPECT_EQ(result.getPackedValueProperty(), Color::White.getPackedValueProperty());
    }

    TEST(TilemapPropertiesTests, GetColorWithCustomDefaultReturnsCustomDefault)
    {
        TilemapProperties properties;
        const Color result = properties.GetColor("key", Color::Orange);
        EXPECT_EQ(result.getPackedValueProperty(), Color::Orange.getPackedValueProperty());
    }

    TEST(TilemapPropertiesTests, GetColorWithWrongTypeReturnsDefaultValue)
    {
        TilemapProperties properties;
        properties["key"] = TilemapPropertyValue::CreateString("test");

        const Color result = properties.GetColor("key", Color::Orange);
        EXPECT_EQ(result.getPackedValueProperty(), Color::Orange.getPackedValueProperty());
    }

    TEST(TilemapPropertiesTests, GetEnumeratorCanIterateProperties)
    {
        TilemapProperties properties;
        properties["key1"] = TilemapPropertyValue::CreateString("test");
        properties["key2"] = TilemapPropertyValue::CreateInt(42);
        properties["key3"] = TilemapPropertyValue::CreateBool(true);

        int count = 0;
        bool foundKey1 = false;
        bool foundKey2 = false;
        bool foundKey3 = false;
        for (const auto& kvp : properties)
        {
            ++count;
            if (kvp.first == "key1" && kvp.second.getTypeProperty() == TilemapPropertyType::String)
            {
                foundKey1 = true;
            }
            if (kvp.first == "key2" && kvp.second.getTypeProperty() == TilemapPropertyType::Int)
            {
                foundKey2 = true;
            }
            if (kvp.first == "key3" && kvp.second.getTypeProperty() == TilemapPropertyType::Bool)
            {
                foundKey3 = true;
            }
        }

        EXPECT_EQ(count, 3);
        EXPECT_TRUE(foundKey1);
        EXPECT_TRUE(foundKey2);
        EXPECT_TRUE(foundKey3);
    }
}
