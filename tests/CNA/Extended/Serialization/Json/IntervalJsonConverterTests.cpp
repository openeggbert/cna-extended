// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for IntervalJsonConverter. Fresh tests below.
#include "CNA/Extended/Serialization/Json/IntervalJsonConverter.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Serialization::Json
{
    TEST(IntervalJsonConverterTests, ReadsTwoElementArrayInGivenOrder)
    {
        const nlohmann::ordered_json j = nlohmann::ordered_json::array({2, 5});
        const CNA::Extended::Interval<int> actual = j.get<CNA::Extended::Interval<int>>();
        EXPECT_EQ(actual.getMinProperty(), 2);
        EXPECT_EQ(actual.getMaxProperty(), 5);
    }

    TEST(IntervalJsonConverterTests, ReadsTwoElementArraySwappedWhenOutOfOrder)
    {
        const nlohmann::ordered_json j = nlohmann::ordered_json::array({5, 2});
        const CNA::Extended::Interval<int> actual = j.get<CNA::Extended::Interval<int>>();
        EXPECT_EQ(actual.getMinProperty(), 2);
        EXPECT_EQ(actual.getMaxProperty(), 5);
    }

    TEST(IntervalJsonConverterTests, ReadsSingleValueAsDegenerateInterval)
    {
        const nlohmann::ordered_json j = nlohmann::ordered_json::array({7});
        const CNA::Extended::Interval<int> actual = j.get<CNA::Extended::Interval<int>>();
        EXPECT_EQ(actual.getMinProperty(), 7);
        EXPECT_EQ(actual.getMaxProperty(), 7);
    }

    TEST(IntervalJsonConverterTests, RoundTripPreservesValue)
    {
        const CNA::Extended::Interval<float> original(1.5f, 9.5f);
        const nlohmann::ordered_json j = original;
        const CNA::Extended::Interval<float> actual = j.get<CNA::Extended::Interval<float>>();
        EXPECT_FLOAT_EQ(actual.getMinProperty(), original.getMinProperty());
        EXPECT_FLOAT_EQ(actual.getMaxProperty(), original.getMaxProperty());
    }
}
