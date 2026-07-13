// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/Tweening/TweenerTests.cs and
// its ColorHandler.cs fixture. Calling convention changed to pointer-to-member syntax (see
// Tweener.hpp's header comment) -- `tweener.TweenTo(obj, x => x.Color, Color.Red, 2f)` becomes
// `tweener.TweenTo(&obj, &ColorHandler::Value, Color::Red, 2.0f)`. Upstream's FloatHandler/
// ColorHandler test fixtures use C# auto-properties (`public float Value { get; set; }`);
// translated to plain public fields here since pointer-to-member needs an actual data member --
// behaviorally identical, since the C# auto-properties had no custom logic. Upstream's
// `ColorHandler.Color` field is renamed to `Value` here (matching FloatHandler's own field name):
// a field named the same as its own type (`Color Color;`) is legal C# but not usable C++ -- the
// field declaration shadows the type name for any later unqualified lookup within the class,
// which breaks the type's own (deleted) default constructor resolution. Not a fidelity gap, a
// forced-by-the-language rename.
#include "CNA/Extended/Tweening/Tweener.hpp"

#include "Microsoft/Xna/Framework/Color.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace CNA::Extended::Tweening
{
    using Microsoft::Xna::Framework::Color;

    namespace
    {
        struct ColorHandler
        {
            Color Value = Color::Black;
        };

        struct FloatHandler
        {
            float Value = 0.0f;
            float Other = 0.0f;
        };
    }

    TEST(TweenerTests, TweenToSucceeds)
    {
        Tweener tweener;
        ColorHandler obj;
        tweener.TweenTo(&obj, &ColorHandler::Value, Color::Red, 2.0f);
        SUCCEED();
    }

    TEST(TweenerTests, ActiveTweensCountIsZeroWhenNoTweensStarted)
    {
        const Tweener tweener;
        EXPECT_EQ(tweener.getActiveTweensCountProperty(), 0);
    }

    TEST(TweenerTests, ActiveTweensCountReflectsNumberOfRunningTweens)
    {
        Tweener tweener;
        FloatHandler obj;

        tweener.TweenTo(&obj, &FloatHandler::Value, 10.0f, 2.0f);
        EXPECT_EQ(tweener.getActiveTweensCountProperty(), 1);

        tweener.TweenTo(&obj, &FloatHandler::Other, 5.0f, 2.0f);
        EXPECT_EQ(tweener.getActiveTweensCountProperty(), 2);
    }

    TEST(TweenerTests, ActiveTweensCountDecreasesAfterTweenCompletes)
    {
        Tweener tweener;
        FloatHandler obj;

        tweener.TweenTo(&obj, &FloatHandler::Value, 10.0f, 1.0f);
        EXPECT_EQ(tweener.getActiveTweensCountProperty(), 1);

        // Advance past the duration so the tween is removed on Update.
        tweener.Update(2.0f);
        EXPECT_EQ(tweener.getActiveTweensCountProperty(), 0);
    }

    TEST(TweenerTests, OnUpdateFiresAfterEachUpdateWithInterpolatedValue)
    {
        Tweener tweener;
        FloatHandler obj;
        int callCount = 0;
        std::vector<float> recordedValues;

        tweener.TweenTo(&obj, &FloatHandler::Value, 10.0f, 1.0f)
            ->OnUpdate(
                [&](Tween&)
                {
                    ++callCount;
                    recordedValues.push_back(obj.Value);
                });

        tweener.Update(0.5f);
        EXPECT_EQ(callCount, 1);
        ASSERT_EQ(recordedValues.size(), 1u);
        EXPECT_FLOAT_EQ(recordedValues[0], 5.0f);

        tweener.Update(0.5f);
        EXPECT_EQ(callCount, 2);
        ASSERT_EQ(recordedValues.size(), 2u);
        EXPECT_FLOAT_EQ(recordedValues[1], 10.0f);
    }

    TEST(TweenerTests, ActiveTweensCountIsZeroAfterCancelAll)
    {
        Tweener tweener;
        FloatHandler obj;

        tweener.TweenTo(&obj, &FloatHandler::Value, 10.0f, 2.0f);
        tweener.CancelAll();
        tweener.Update(0.0f);

        EXPECT_EQ(tweener.getActiveTweensCountProperty(), 0);
    }
}
