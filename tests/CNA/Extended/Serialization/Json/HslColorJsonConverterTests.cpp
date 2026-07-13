// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for HslColorJsonConverter. Fresh tests below.
#include "CNA/Extended/Serialization/Json/HslColorJsonConverter.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Serialization::Json
{
    using Microsoft::Xna::Framework::Color;

    TEST(HslColorJsonConverterTests, ReadDelegatesToColorHexParsing)
    {
        const nlohmann::ordered_json j = "#FF0000FF";
        const HslColor actual = j.get<HslColor>();
        EXPECT_EQ(HslColor::ToRgb(actual), Color(255, 0, 0, 255));
    }

    TEST(HslColorJsonConverterTests, RoundTripPreservesRgbColorApproximately)
    {
        // HslColor::FromRgb/ToRgb (pre-existing, independently tested in HslColorTests.cpp)
        // involve floating-point HSL math and are not exact bitwise inverses of each other for
        // arbitrary RGB values -- verified directly (no JSON involved) that
        // ToRgb(FromRgb(Color(10,20,30,255))) already yields (9,20,30,255), and that a further
        // round trip of that same value drifts again to (9,19,30,255) rather than stabilizing.
        // This test therefore checks the JSON converter's plumbing (delegates to Color's own
        // hex-string round trip via ToRgb/FromRgb, without corrupting or garbling values) using
        // an approximate per-channel comparison, rather than asserting an exact round-trip
        // invariant that doesn't hold for this conversion in general.
        const Color originalRgb(10, 20, 30, 255);
        const HslColor original = HslColor::FromRgb(originalRgb);

        const nlohmann::ordered_json j = original;
        const HslColor actual = j.get<HslColor>();
        const Color roundTripped = HslColor::ToRgb(actual);

        constexpr int kTolerance = 2;
        EXPECT_NEAR(static_cast<int>(roundTripped.getRProperty()), static_cast<int>(originalRgb.getRProperty()), kTolerance);
        EXPECT_NEAR(static_cast<int>(roundTripped.getGProperty()), static_cast<int>(originalRgb.getGProperty()), kTolerance);
        EXPECT_NEAR(static_cast<int>(roundTripped.getBProperty()), static_cast<int>(originalRgb.getBProperty()), kTolerance);
        EXPECT_EQ(roundTripped.getAProperty(), originalRgb.getAProperty());
    }
}
