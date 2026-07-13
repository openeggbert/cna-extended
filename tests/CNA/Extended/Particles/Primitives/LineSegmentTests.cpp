// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for Particles::Primitives::LineSegment (a distinct type from the
// already-tested CNA::Extended::LineSegment2D/Triangulation::LineSegment). Fresh tests below.
#include "CNA/Extended/Particles/Primitives/LineSegment.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Primitives
{
    TEST(LineSegmentTests, OriginAndDirection)
    {
        const LineSegment subject(Vector2(1.0f, 2.0f), Vector2(4.0f, 6.0f));

        EXPECT_FLOAT_EQ(subject.getOriginProperty().X, 1.0f);
        EXPECT_FLOAT_EQ(subject.getOriginProperty().Y, 2.0f);
        EXPECT_FLOAT_EQ(subject.getDirectionProperty().X, 3.0f);
        EXPECT_FLOAT_EQ(subject.getDirectionProperty().Y, 4.0f);
    }

    TEST(LineSegmentTests, ToVector2MatchesDirection)
    {
        const LineSegment subject(Vector2(0.0f, 0.0f), Vector2(3.0f, 4.0f));
        const Vector2 vector = subject.ToVector2();

        EXPECT_FLOAT_EQ(vector.X, 3.0f);
        EXPECT_FLOAT_EQ(vector.Y, 4.0f);
    }

    TEST(LineSegmentTests, TranslateOffsetsBothPoints)
    {
        const LineSegment subject(Vector2(1.0f, 1.0f), Vector2(2.0f, 2.0f));
        const LineSegment translated = subject.Translate(Vector2(10.0f, 10.0f));

        EXPECT_FLOAT_EQ(translated.getOriginProperty().X, 11.0f);
        EXPECT_FLOAT_EQ(translated.getOriginProperty().Y, 11.0f);
    }

    TEST(LineSegmentTests, FromOriginAddsVectorToOrigin)
    {
        const LineSegment subject = LineSegment::FromOrigin(Vector2(1.0f, 1.0f), Vector2(5.0f, 0.0f));

        EXPECT_FLOAT_EQ(subject.getOriginProperty().X, 1.0f);
        EXPECT_FLOAT_EQ(subject.ToVector2().X, 5.0f);
    }

    TEST(LineSegmentTests, EqualityComparesBothPoints)
    {
        EXPECT_EQ(LineSegment(Vector2(1, 2), Vector2(3, 4)), LineSegment(Vector2(1, 2), Vector2(3, 4)));
        EXPECT_NE(LineSegment(Vector2(1, 2), Vector2(3, 4)), LineSegment(Vector2(1, 2), Vector2(3, 5)));
    }
}
