// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Math/IntervalTests.cs.
#include "CNA/Extended/Interval.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(IntervalTests, ConstructorWithMinLessThanMaxCreatesProperInterval)
    {
        const Interval<int> interval(10, 20);
        EXPECT_EQ(interval.getMinProperty(), 10);
        EXPECT_EQ(interval.getMaxProperty(), 20);
        EXPECT_TRUE(interval.getIsProperProperty());
        EXPECT_FALSE(interval.getIsDegenerateProperty());
        EXPECT_FALSE(interval.getIsEmptyProperty());
    }

    TEST(IntervalTests, ConstructorWithEqualBoundsCreatesDegenerateInterval)
    {
        const Interval<int> interval(10, 10);
        EXPECT_EQ(interval.getMinProperty(), 10);
        EXPECT_EQ(interval.getMaxProperty(), 10);
        EXPECT_FALSE(interval.getIsProperProperty());
        EXPECT_TRUE(interval.getIsDegenerateProperty());
        EXPECT_FALSE(interval.getIsEmptyProperty());
    }

    TEST(IntervalTests, ConstructorWithInvalidBoundsThrows)
    {
        EXPECT_THROW((Interval<int>(20, 10)), std::invalid_argument);
    }

    TEST(IntervalTests, ConstructorSingleValueCreatesDegenerate)
    {
        const Interval<int> interval(10);
        EXPECT_EQ(interval.getMinProperty(), 10);
        EXPECT_EQ(interval.getMaxProperty(), 10);
        EXPECT_TRUE(interval.getIsDegenerateProperty());
        EXPECT_FALSE(interval.getIsProperProperty());
        EXPECT_FALSE(interval.getIsEmptyProperty());
    }

    TEST(IntervalTests, EmptyCreatesEmptyInterval)
    {
        const Interval<int> empty = Interval<int>::Empty();
        EXPECT_TRUE(empty.getIsEmptyProperty());
        EXPECT_FALSE(empty.getIsDegenerateProperty());
        EXPECT_FALSE(empty.getIsProperProperty());
    }

    TEST(IntervalTests, EmptyAccessingBoundsThrows)
    {
        const Interval<int> empty = Interval<int>::Empty();
        EXPECT_THROW((void)empty.getMinProperty(), std::logic_error);
        EXPECT_THROW((void)empty.getMaxProperty(), std::logic_error);
    }

    TEST(IntervalTests, ContainsValueReturnsTrueForBoundaryAndInteriorValues)
    {
        const Interval<int> interval(10, 15);
        for (int i = 10; i <= 15; i++)
        {
            EXPECT_TRUE(interval.Contains(i));
        }
    }

    TEST(IntervalTests, ContainsValueReturnsFalseForValuesOutsideBounds)
    {
        const Interval<int> interval(10, 20);
        EXPECT_FALSE(interval.Contains(9));
        EXPECT_FALSE(interval.Contains(21));
    }

    TEST(IntervalTests, ContainsValueEmptyIntervalReturnsFalse)
    {
        const Interval<int> empty = Interval<int>::Empty();
        EXPECT_FALSE(empty.Contains(10));
    }

    TEST(IntervalTests, ContainsIntervalReturnsTrueForCompletelyContainedInterval)
    {
        const Interval<int> outer(10, 30);
        const Interval<int> inner(15, 25);
        EXPECT_TRUE(outer.Contains(inner));
    }

    TEST(IntervalTests, ContainsIntervalReturnsFalseForPartiallyOverlappingInterval)
    {
        const Interval<int> interval(10, 20);
        const Interval<int> overlapping(15, 25);
        EXPECT_FALSE(interval.Contains(overlapping));
    }

    TEST(IntervalTests, ContainsIntervalReturnsFalseForSeparateInterval)
    {
        const Interval<int> interval(10, 20);
        const Interval<int> separate(30, 40);
        EXPECT_FALSE(interval.Contains(separate));
    }

    TEST(IntervalTests, ContainsEmptyIntervalReturnsTrue)
    {
        const Interval<int> interval(10, 20);
        const Interval<int> empty = Interval<int>::Empty();
        EXPECT_TRUE(interval.Contains(empty));
        EXPECT_TRUE(empty.Contains(empty));
    }

    TEST(IntervalTests, ContainsSelfReturnsTrue)
    {
        const Interval<int> interval(10, 20);
        EXPECT_TRUE(interval.Contains(interval));
    }

    TEST(IntervalTests, OverlapReturnsTrueForOverlappingIntervals)
    {
        const Interval<int> a(10, 20);
        const Interval<int> b(15, 25);
        EXPECT_TRUE(a.Overlap(b));
        EXPECT_TRUE(b.Overlap(a));
    }

    TEST(IntervalTests, OverlapReturnsFalseForSeparateIntervals)
    {
        const Interval<int> a(10, 20);
        const Interval<int> c(30, 40);
        EXPECT_FALSE(a.Overlap(c));
        EXPECT_FALSE(c.Overlap(a));
    }

    TEST(IntervalTests, OverlapReturnsTrueForIntervalsTouchingAtBoundary)
    {
        const Interval<int> a(10, 20);
        const Interval<int> d(20, 30);
        EXPECT_TRUE(a.Overlap(d));
    }

    TEST(IntervalTests, OverlapWithEmptyIntervalReturnsFalse)
    {
        const Interval<int> interval(10, 20);
        const Interval<int> empty = Interval<int>::Empty();
        EXPECT_FALSE(interval.Overlap(empty));
        EXPECT_FALSE(empty.Overlap(empty));
    }

    TEST(IntervalTests, OverlapSelfReturnsTrue)
    {
        const Interval<int> interval(10, 20);
        EXPECT_TRUE(interval.Overlap(interval));
    }

    TEST(IntervalTests, IntersectOverlappingIntervalsReturnsCorrectBounds)
    {
        const Interval<int> a(10, 20);
        const Interval<int> b(15, 25);
        const Interval<int> intersection = a.Intersect(b);
        EXPECT_EQ(intersection.getMinProperty(), 15);
        EXPECT_EQ(intersection.getMaxProperty(), 20);
    }

    TEST(IntervalTests, IntersectNoOverlapReturnsEmpty)
    {
        const Interval<int> a(10, 20);
        const Interval<int> b(30, 40);
        EXPECT_TRUE(a.Intersect(b).getIsEmptyProperty());
    }

    TEST(IntervalTests, IntersectWithEmptyIntervalReturnsEmpty)
    {
        const Interval<int> interval(10, 20);
        const Interval<int> empty = Interval<int>::Empty();
        EXPECT_TRUE(interval.Intersect(empty).getIsEmptyProperty());
        EXPECT_TRUE(empty.Intersect(empty).getIsEmptyProperty());
    }

    TEST(IntervalTests, IntersectSelfReturnsOriginalInterval)
    {
        const Interval<int> interval(10, 20);
        EXPECT_EQ(interval, interval.Intersect(interval));
    }

    TEST(IntervalTests, HullSeparateIntervalsReturnsSpanningInterval)
    {
        const Interval<int> a(10, 20);
        const Interval<int> b(30, 40);
        const Interval<int> hull = a.Hull(b);
        EXPECT_EQ(hull.getMinProperty(), 10);
        EXPECT_EQ(hull.getMaxProperty(), 40);
    }

    TEST(IntervalTests, HullWithEmptyIntervalReturnsNonEmptyInterval)
    {
        const Interval<int> interval(10, 20);
        const Interval<int> empty = Interval<int>::Empty();
        EXPECT_EQ(interval, interval.Hull(empty));
        EXPECT_EQ(interval, empty.Hull(interval));
        EXPECT_TRUE(empty.Hull(empty).getIsEmptyProperty());
    }

    TEST(IntervalTests, HullSelfReturnsOriginalInterval)
    {
        const Interval<int> interval(10, 20);
        EXPECT_EQ(interval, interval.Hull(interval));
    }

    TEST(IntervalTests, HullStaticMethodHandlesParameterOrderCorrectly)
    {
        const Interval<int> hull1 = Interval<int>::Hull(20, 10);
        const Interval<int> hull2 = Interval<int>::Hull(10, 20);
        EXPECT_EQ(hull1.getMinProperty(), 10);
        EXPECT_EQ(hull1.getMaxProperty(), 20);
        EXPECT_EQ(hull1, hull2);
    }

    TEST(IntervalTests, EqualitySameIntervalsReturnsTrue)
    {
        const Interval<int> a(10, 20);
        const Interval<int> b(10, 20);
        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a.Equals(b));
        EXPECT_FALSE(a != b);
    }

    TEST(IntervalTests, EqualityDifferentIntervalsReturnsFalse)
    {
        const Interval<int> a(10, 20);
        const Interval<int> c(20, 30);
        EXPECT_FALSE(a == c);
        EXPECT_FALSE(a.Equals(c));
        EXPECT_TRUE(a != c);
    }

    TEST(IntervalTests, EqualityEmptyIntervalsReturnsTrue)
    {
        const Interval<int> empty1 = Interval<int>::Empty();
        const Interval<int> empty2 = Interval<int>::Empty();
        EXPECT_TRUE(empty1 == empty2);
        EXPECT_TRUE(empty1.Equals(empty2));
    }

    TEST(IntervalTests, EqualityEmptyVsNonEmptyReturnsFalse)
    {
        const Interval<int> interval(10, 20);
        const Interval<int> empty = Interval<int>::Empty();
        EXPECT_FALSE(interval == empty);
        EXPECT_TRUE(interval != empty);
    }

    TEST(IntervalTests, GetHashCodeEqualIntervalsReturnsSameHashCode)
    {
        const Interval<int> a(10, 20);
        const Interval<int> b(10, 20);
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(IntervalTests, ImplicitConversionFromValueCreatesDegenerate)
    {
        const Interval<int> interval = 10;
        EXPECT_EQ(interval.getMinProperty(), 10);
        EXPECT_EQ(interval.getMaxProperty(), 10);
        EXPECT_TRUE(interval.getIsDegenerateProperty());
    }

    TEST(IntervalTests, ToStringShowsCorrectFormat)
    {
        EXPECT_EQ(Interval<int>(10, 20).ToString(), "[10, 20]");
        EXPECT_EQ(Interval<int>(10).ToString(), "[10]");
        EXPECT_EQ(Interval<int>::Empty().ToString(), "\xE2\x88\x85");
    }

    TEST(IntervalTests, StringIntervalWorksWithComparable)
    {
        const Interval<std::string> interval("apple", "zebra");
        EXPECT_TRUE(interval.Contains(std::string("banana")));
        EXPECT_FALSE(interval.Contains(std::string("aardvark")));
    }
}
