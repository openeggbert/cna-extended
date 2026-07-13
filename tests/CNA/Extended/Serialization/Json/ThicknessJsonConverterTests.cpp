// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for ThicknessJsonConverter. Fresh tests below.
#include "CNA/Extended/Serialization/Json/ThicknessJsonConverter.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Serialization::Json
{
    TEST(ThicknessJsonConverterTests, ReadsFourComponentDelimitedString)
    {
        const nlohmann::ordered_json j = "1 2 3 4";
        const Thickness actual = j.get<Thickness>();
        EXPECT_EQ(actual.getLeftProperty(), 1);
        EXPECT_EQ(actual.getTopProperty(), 2);
        EXPECT_EQ(actual.getRightProperty(), 3);
        EXPECT_EQ(actual.getBottomProperty(), 4);
    }

    TEST(ThicknessJsonConverterTests, ReadsSingleValueAsUniformThickness)
    {
        const nlohmann::ordered_json j = "5";
        const Thickness actual = j.get<Thickness>();
        EXPECT_EQ(actual.getLeftProperty(), 5);
        EXPECT_EQ(actual.getTopProperty(), 5);
        EXPECT_EQ(actual.getRightProperty(), 5);
        EXPECT_EQ(actual.getBottomProperty(), 5);
    }

    TEST(ThicknessJsonConverterTests, RoundTripPreservesValue)
    {
        const Thickness original(1, 2, 3, 4);
        const nlohmann::ordered_json j = original;
        const Thickness actual = j.get<Thickness>();
        EXPECT_EQ(actual.getLeftProperty(), original.getLeftProperty());
        EXPECT_EQ(actual.getTopProperty(), original.getTopProperty());
        EXPECT_EQ(actual.getRightProperty(), original.getRightProperty());
        EXPECT_EQ(actual.getBottomProperty(), original.getBottomProperty());
    }
}
