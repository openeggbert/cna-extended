// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/Serialization/ColorJsonConverterTests.cs.
// Upstream's CanConvert tests have no equivalent here: this port dispatches via nlohmann's
// compile-time ADL customization point (a specific C++ type, resolved at compile time), not a
// runtime Type-based CanConvert(Type) check -- there is no runtime "can this JSON value convert to
// Color" question to test. The null-writer test is dropped for the same reason as elsewhere in
// this port: reference parameters are non-null by construction in C++ (see ActorPairKey.hpp's
// precedent). Read/Write are exercised directly against `nlohmann::ordered_json`, the closest
// equivalent to upstream's direct `Read`/`Write` calls against a raw reader/writer.
#include "CNA/Extended/Serialization/Json/ColorJsonConverter.hpp"

#include "CNA/Extended/ColorHelper.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Serialization::Json
{
    using Microsoft::Xna::Framework::Color;

    TEST(ColorJsonConverterTests, ReadNamedColorReturnsExpectedColor)
    {
        const nlohmann::ordered_json j = "Red";
        const Color actual = j.get<Color>();
        EXPECT_EQ(actual, Color(255, 0, 0, 255));
    }

    TEST(ColorJsonConverterTests, ReadHexColorReturnsExpectedColor)
    {
        const nlohmann::ordered_json j = "#FF0000FF";
        const Color actual = j.get<Color>();
        EXPECT_EQ(actual, Color(255, 0, 0, 255));
    }

    TEST(ColorJsonConverterTests, WriteValidColorWritesExpectedJson)
    {
        const std::string expected = "#ff000000";
        const Color color = CNA::Extended::ColorHelper::FromHex(expected);

        const nlohmann::ordered_json j = color;

        EXPECT_EQ(j.get<std::string>(), expected);
    }

    TEST(ColorJsonConverterTests, RoundTripPreservesColor)
    {
        const Color original(12, 34, 56, 78);
        const nlohmann::ordered_json j = original;
        const Color actual = j.get<Color>();
        EXPECT_EQ(actual, original);
    }
}
