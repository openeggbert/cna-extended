// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Upstream's 17 active tests from tests/MonoGame.Extended.Tests/Collections/DequeTests.cs are
// ported 1:1 below (each [Theory]'s [InlineData(0,1,2,50)] cases become an in-body loop over the
// same 4 values, rather than gtest TEST_P machinery, to keep a direct visual correspondence to
// the C# source). `Deque_Get_Index`'s `.Shuffle(_random)` has no bearing on correctness (the
// assertion only depends on Get(index, ...) being correct at every index, not visitation order),
// so it iterates indices in natural order instead of replicating the shuffle -- documented at
// that test. Fresh tests are added afterward for: the documented IndexOf/Remove bug (see
// Deque.hpp's header comment) in both its failure modes, the CopyTo-throws-on-empty-destination
// quirk, IList<T>::Insert always throwing, Pop() throwing on empty, ResizeFunction, and explicit
// wraparound coverage for RemoveAt's shift-shorter-half logic.
#include "CNA/Extended/Collections/Deque.hpp"

#include <gtest/gtest.h>

#include <array>

namespace CNA::Extended::Collections
{
    namespace
    {
        struct TestDequeElement
        {
            int Value = 0;

            friend bool operator==(const TestDequeElement& first, const TestDequeElement& second) { return first.Value == second.Value; }
        };

        std::vector<TestDequeElement> MakeElements(const int count)
        {
            std::vector<TestDequeElement> elements(static_cast<std::size_t>(count));
            for (int i = 0; i < count; i++)
            {
                elements[static_cast<std::size_t>(i)].Value = i;
            }
            return elements;
        }

        constexpr std::array<int, 4> kCounts = {0, 1, 2, 50};
    }

    TEST(DequeTests, ConstructorDefault)
    {
        const Deque<int> deque;
        EXPECT_EQ(deque.getCountProperty(), 0);
        EXPECT_EQ(deque.getCapacityProperty(), 0);
    }

    TEST(DequeTests, ConstructorCollection)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            const Deque<TestDequeElement> deque(elements);
            EXPECT_EQ(deque.getCountProperty(), count);
            EXPECT_EQ(deque.getCapacityProperty(), count);
            for (int index = 0; index < deque.getCountProperty(); index++)
            {
                EXPECT_EQ(deque[index].Value, index);
            }
        }
    }

    TEST(DequeTests, ConstructorCapacity)
    {
        for (const int capacity : kCounts)
        {
            const Deque<TestDequeElement> deque(capacity);
            EXPECT_EQ(deque.getCountProperty(), 0);
            EXPECT_EQ(deque.getCapacityProperty(), capacity);
        }
    }

    TEST(DequeTests, Clear)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            Deque<TestDequeElement> deque(elements);
            deque.Clear();
            EXPECT_EQ(deque.getCountProperty(), 0);
            EXPECT_GE(deque.getCapacityProperty(), count);
        }
    }

    TEST(DequeTests, TrimAndClear)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            Deque<TestDequeElement> deque(elements);
            deque.Clear();
            deque.TrimExcess();
            EXPECT_EQ(deque.getCountProperty(), 0);
            EXPECT_EQ(deque.getCapacityProperty(), 0);
        }
    }

    TEST(DequeTests, TrimFront)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            Deque<TestDequeElement> deque(elements);

            for (int i = 0; i < count; i++)
            {
                deque.RemoveFromFront();
                deque.setCapacityProperty(deque.getCountProperty());
                EXPECT_EQ(deque.getCountProperty(), count - 1 - i);
                EXPECT_EQ(deque.getCapacityProperty(), count - 1 - i);
            }
        }
    }

    TEST(DequeTests, TrimBack)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            Deque<TestDequeElement> deque(elements);

            for (int i = 0; i < count; i++)
            {
                deque.RemoveFromBack();
                deque.setCapacityProperty(deque.getCountProperty());
                EXPECT_EQ(deque.getCountProperty(), count - 1 - i);
                EXPECT_EQ(deque.getCapacityProperty(), count - 1 - i);
            }
        }
    }

    TEST(DequeTests, AddFront)
    {
        for (const int count : kCounts)
        {
            Deque<TestDequeElement> deque;
            for (int i = 0; i < count; i++)
            {
                deque.AddToFront(TestDequeElement{i});
            }
            EXPECT_EQ(deque.getCountProperty(), count);
            EXPECT_GE(deque.getCapacityProperty(), count);
            for (int index = 0; index < deque.getCountProperty(); index++)
            {
                EXPECT_EQ(deque.getItem(index).Value, deque.getCountProperty() - 1 - index);
            }
        }
    }

    TEST(DequeTests, AddBack)
    {
        for (const int count : kCounts)
        {
            Deque<TestDequeElement> deque;
            for (int i = 0; i < count; i++)
            {
                deque.AddToBack(TestDequeElement{i});
            }
            EXPECT_EQ(deque.getCountProperty(), count);
            EXPECT_GE(deque.getCapacityProperty(), count);
            for (int index = 0; index < deque.getCountProperty(); index++)
            {
                EXPECT_EQ(deque.getItem(index).Value, index);
            }
        }
    }

    TEST(DequeTests, RemoveFront)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            Deque<TestDequeElement> deque(elements);

            int index = 0;
            TestDequeElement element;
            while (deque.RemoveFromFront(element))
            {
                EXPECT_EQ(element.Value, index);
                index++;
            }
        }
    }

    TEST(DequeTests, RemoveBack)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            Deque<TestDequeElement> deque(elements);

            int index = 0;
            TestDequeElement element;
            while (deque.RemoveFromBack(element))
            {
                EXPECT_EQ(element.Value, static_cast<int>(elements.size()) - 1 - index);
                index++;
            }
        }
    }

    TEST(DequeTests, GetFront)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            Deque<TestDequeElement> deque(elements);
            for (int index = 0; index < count; index++)
            {
                TestDequeElement element;
                deque.GetFront(element);
                deque.RemoveFromFront();
                EXPECT_EQ(element.Value, index);
            }
        }
    }

    TEST(DequeTests, GetBack)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            Deque<TestDequeElement> deque(elements);
            for (int index = 0; index < count; index++)
            {
                TestDequeElement element;
                deque.GetBack(element);
                deque.RemoveFromBack();
                EXPECT_EQ(element.Value, count - 1 - index);
            }
        }
    }

    // Upstream shuffles the visited indices via `.Shuffle(_random)`; visitation order has no
    // bearing on the assertion (Get(index, ...) must be correct at every index regardless of
    // order visited), so this iterates in natural order instead of replicating the shuffle.
    TEST(DequeTests, GetIndex)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            const Deque<TestDequeElement> deque(elements);
            for (int index = 0; index < count; index++)
            {
                TestDequeElement element;
                deque.Get(index, element);
                EXPECT_EQ(element.Value, index);
            }
        }
    }

    TEST(DequeTests, ForEachIteration)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            const Deque<TestDequeElement> deque(elements);
            int counter = 0;
            for (const TestDequeElement& element : deque)
            {
                EXPECT_EQ(element.Value, counter);
                counter++;
            }
        }
    }

    TEST(DequeTests, ForEachIterationModified)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            Deque<TestDequeElement> deque(elements);
            int counter = 0;
            for (const TestDequeElement& element : deque)
            {
                EXPECT_EQ(element.Value, counter);
                counter++;
                deque.RemoveFromFront();
            }
        }
    }

    // Upstream picks a random index (`_random.Next(0, deque.Count - 1)`) each iteration; only
    // the resulting Count bookkeeping is actually asserted, not which index was chosen, so this
    // substitutes a deterministic midpoint index for reproducibility -- same stress-test intent
    // (repeatedly RemoveAt from a shrinking, potentially-wrapping deque), without relying on an
    // unseeded RNG.
    TEST(DequeTests, Remove)
    {
        for (const int count : kCounts)
        {
            const std::vector<TestDequeElement> elements = MakeElements(count);
            Deque<TestDequeElement> deque(elements);
            int counter = count;
            while (deque.getCountProperty() > 0)
            {
                const intcs index = deque.getCountProperty() > 1 ? (deque.getCountProperty() - 1) / 2 : 0;
                deque.RemoveAt(index);
                counter--;
                EXPECT_EQ(deque.getCountProperty(), counter);
            }
        }
    }

    // --- Fresh tests below: no upstream coverage. ---

    TEST(DequeTests, IndexOfFindsPresentElement)
    {
        const Deque<TestDequeElement> deque(MakeElements(5));
        EXPECT_EQ(deque.IndexOf(TestDequeElement{3}), 3);
    }

    // Regression test for the upstream bug documented in Deque.hpp's header comment, failure
    // mode 1: a genuinely-absent item's bogus computed index lands past Count, so RemoveAt (via
    // Remove) throws rather than Remove() returning false. Requires Capacity > Count (built via
    // AddToBack from empty, which grows to capacity 4 for 3 elements) -- the collection
    // constructor instead sizes Capacity == Count exactly, which lands in the OTHER failure mode
    // (see RemoveOfAbsentItemReproducesKnownUpstreamBugSilentCorruptionCase below).
    TEST(DequeTests, RemoveOfAbsentItemReproducesKnownUpstreamBugThrowingCase)
    {
        Deque<TestDequeElement> deque;
        deque.AddToBack(TestDequeElement{10});
        deque.AddToBack(TestDequeElement{20});
        deque.AddToBack(TestDequeElement{30});
        ASSERT_EQ(deque.getCapacityProperty(), 4);
        ASSERT_EQ(deque.getCountProperty(), 3);

        EXPECT_THROW((void)deque.Remove(TestDequeElement{999}), std::out_of_range);
    }

    // Regression test for the upstream bug, failure mode 2: once the deque has wrapped (front
    // index > 0), the same bogus-index formula can land WITHIN [0, Count), causing Remove() to
    // silently remove an unrelated real element and incorrectly report success.
    TEST(DequeTests, RemoveOfAbsentItemReproducesKnownUpstreamBugSilentCorruptionCase)
    {
        Deque<TestDequeElement> deque(4);
        deque.AddToBack(TestDequeElement{10});
        deque.AddToBack(TestDequeElement{20});
        deque.AddToBack(TestDequeElement{30});
        deque.AddToBack(TestDequeElement{40});
        deque.RemoveFromFront(); // frontArrayIndex_ becomes 1; deque is now [20, 30, 40].

        const bool result = deque.Remove(TestDequeElement{999});

        // Matches upstream's bug: reports success even though 999 was never in the deque...
        EXPECT_TRUE(result);
        // ...and a real, unrelated element (40, the back) was silently removed instead.
        EXPECT_EQ(deque.getCountProperty(), 2);
        EXPECT_FALSE(deque.Contains(TestDequeElement{999}));
        EXPECT_FALSE(deque.Contains(TestDequeElement{40}))
            << "If this starts failing, upstream's IndexOf bug may have been fixed and this port's fidelity note should be revisited.";
    }

    // Regression test for the true-UB-avoidance guard: C#'s equivalent throws a catchable
    // DivideByZeroException on a virgin, never-grown Deque; C++ integer modulo by zero is
    // undefined behavior instead, so this is explicitly guarded (see Deque.hpp's header comment).
    TEST(DequeTests, IndexOfOnVirginEmptyDequeThrowsRatherThanUndefinedBehavior)
    {
        const Deque<TestDequeElement> deque;
        EXPECT_THROW((void)deque.IndexOf(TestDequeElement{1}), std::domain_error);
    }

    TEST(DequeTests, RemoveOnVirginEmptyDequeThrowsRatherThanUndefinedBehavior)
    {
        Deque<TestDequeElement> deque;
        EXPECT_THROW((void)deque.Remove(TestDequeElement{1}), std::domain_error);
    }

    TEST(DequeTests, ContainsIsUnaffectedByTheIndexOfBug)
    {
        // Contains uses direct iteration (matching upstream's LINQ-based implementation), not
        // the buggy IndexOf, so it correctly returns false for an absent item even on a virgin
        // empty deque (no exception, no false positive).
        const Deque<TestDequeElement> deque;
        EXPECT_FALSE(deque.Contains(TestDequeElement{1}));
    }

    TEST(DequeTests, CopyToThrowsForEmptyDestinationEvenWhenCountIsZero)
    {
        // Matches upstream: bounds are validated unconditionally, before the Count == 0
        // early-return check -- see Deque.hpp's header comment.
        const Deque<TestDequeElement> deque;
        std::vector<TestDequeElement> destination;
        EXPECT_THROW(deque.CopyTo(destination), std::invalid_argument);
    }

    TEST(DequeTests, CopyToCopiesElementsInFrontToBackOrder)
    {
        const Deque<TestDequeElement> deque(MakeElements(5));
        std::vector<TestDequeElement> destination(5);
        deque.CopyTo(destination);
        for (int i = 0; i < 5; i++)
        {
            EXPECT_EQ(destination[static_cast<std::size_t>(i)].Value, i);
        }
    }

    TEST(DequeTests, CopyToHandlesWraparound)
    {
        Deque<TestDequeElement> deque(4);
        deque.AddToBack(TestDequeElement{10});
        deque.AddToBack(TestDequeElement{20});
        deque.AddToBack(TestDequeElement{30});
        deque.AddToBack(TestDequeElement{40});
        deque.RemoveFromFront();
        deque.AddToBack(TestDequeElement{50}); // Wraps: physical layout is now [50, 20, 30, 40].

        std::vector<TestDequeElement> destination(4);
        deque.CopyTo(destination);

        EXPECT_EQ(destination[0].Value, 20);
        EXPECT_EQ(destination[1].Value, 30);
        EXPECT_EQ(destination[2].Value, 40);
        EXPECT_EQ(destination[3].Value, 50);
    }

    TEST(DequeTests, IListInsertAlwaysThrows)
    {
        Deque<TestDequeElement> deque(MakeElements(2));
        System::Collections::Generic::IList<TestDequeElement>& asIList = deque;
        EXPECT_THROW(asIList.Insert(0, TestDequeElement{99}), std::logic_error);
    }

    TEST(DequeTests, PopRemovesAndReturnsBackElement)
    {
        Deque<TestDequeElement> deque(MakeElements(3));
        const TestDequeElement popped = deque.Pop();
        EXPECT_EQ(popped.Value, 2);
        EXPECT_EQ(deque.getCountProperty(), 2);
    }

    TEST(DequeTests, PopOnEmptyDequeThrows)
    {
        Deque<TestDequeElement> deque;
        EXPECT_THROW((void)deque.Pop(), std::logic_error);
    }

    TEST(DequeTests, ResizeFunctionDefaultsToDoubling)
    {
        const Deque<TestDequeElement> deque;
        EXPECT_EQ(deque.getResizeFunctionProperty()(3), 6);
    }

    TEST(DequeTests, ResizeFunctionCanBeCustomized)
    {
        Deque<TestDequeElement> deque;
        deque.setResizeFunctionProperty([](const intcs x) { return x + 10; });
        deque.AddToBack(TestDequeElement{1});
        // From capacity 0, EnsureCapacity uses the default-capacity path (4) regardless of
        // ResizeFunction (matches upstream: the custom function only applies once _items is
        // already non-empty).
        EXPECT_EQ(deque.getCapacityProperty(), 4);

        for (int i = 0; i < 4; i++)
        {
            deque.AddToBack(TestDequeElement{i});
        }
        // Now growing from a non-empty backing array: capacity 4 -> resizeFunction(4) = 14.
        EXPECT_EQ(deque.getCapacityProperty(), 14);
    }

    TEST(DequeTests, SetResizeFunctionToEmptyResetsToDefault)
    {
        Deque<TestDequeElement> deque;
        deque.setResizeFunctionProperty([](const intcs x) { return x + 10; });
        deque.setResizeFunctionProperty(nullptr);
        EXPECT_EQ(deque.getResizeFunctionProperty()(3), 6);
    }

    // Confirmed CORRECT: front-half-shift removal on a non-wrapped buffer (frontArrayIndex_ ==
    // 0) is the one case where RemoveAt's middle-removal logic actually works as intended -- see
    // Deque.hpp's header comment for the two confirmed-BROKEN cases below.
    TEST(DequeTests, RemoveAtMiddleShiftsShorterFrontHalf)
    {
        // 7 elements, removing index 1 (< Count/2 == 3): shifts the shorter front half.
        Deque<TestDequeElement> deque(MakeElements(7));
        deque.RemoveAt(1);
        EXPECT_EQ(deque.getCountProperty(), 6);
        const std::vector<int> expected = {0, 2, 3, 4, 5, 6};
        for (std::size_t i = 0; i < expected.size(); i++)
        {
            EXPECT_EQ(deque.getItem(static_cast<intcs>(i)).Value, expected[i]);
        }
    }

    // Regression test for a SECOND significant upstream RemoveAt bug (see Deque.hpp's header
    // comment): the back-half-shift branch is broken even for a simple NON-wrapped buffer. This
    // was discovered empirically -- an initial version of this test asserted the mathematically
    // correct post-removal sequence and failed against this port, which faithfully reproduces
    // upstream's algorithm; the failure was then confirmed to be inherent to the algorithm
    // itself (traced by hand against the C# source), not a translation error, before being
    // rewritten to assert the actual (buggy) result, as this project's fidelity mandate requires.
    TEST(DequeTests, RemoveAtMiddleShiftsShorterBackHalfReproducesKnownUpstreamBug)
    {
        // 7 elements, removing index 5 (>= Count/2 == 3): "shifts the shorter back half" --
        // but does not actually remove the intended element, and additionally corrupts the
        // front of the sequence (see header comment for the root cause: arrayCenterIndex is a
        // fixed physical-array midpoint unrelated to frontArrayIndex_, and the shared
        // frontArrayIndex_-adjustment check is copy-pasted from the front-half branch without
        // being back-half-appropriate).
        Deque<TestDequeElement> deque(MakeElements(7));
        deque.RemoveAt(5);
        EXPECT_EQ(deque.getCountProperty(), 6);
        // NOT the mathematically-correct {0,1,2,3,4,6} (value 5 removed, order preserved) --
        // matches upstream's actual (buggy) output instead: value 5 is not removed at all (it
        // survives at logical index 4), value 4 is lost/duplicated away, and value 0 (the
        // original front!) is displaced to the back.
        const std::vector<int> expected = {1, 2, 3, 3, 5, 0};
        for (std::size_t i = 0; i < expected.size(); i++)
        {
            EXPECT_EQ(deque.getItem(static_cast<intcs>(i)).Value, expected[i]);
        }
    }

    // Regression test for a THIRD significant upstream RemoveAt bug (see Deque.hpp's header
    // comment): the front-half-shift branch is ALSO broken once the buffer has wrapped
    // (frontArrayIndex_ != 0), even though it works correctly for the non-wrapped case above.
    // Discovered and confirmed the same way as the back-half bug: an initial version of this
    // test asserted the correct logical result, failed, and was traced by hand to be inherent
    // to the upstream algorithm before being rewritten to assert the actual output.
    TEST(DequeTests, RemoveAtMiddleOnWrappedBufferReproducesKnownUpstreamBug)
    {
        Deque<TestDequeElement> deque(4);
        deque.AddToBack(TestDequeElement{10});
        deque.AddToBack(TestDequeElement{20});
        deque.AddToBack(TestDequeElement{30});
        deque.AddToBack(TestDequeElement{40});
        deque.RemoveFromFront();
        deque.AddToBack(TestDequeElement{50}); // Physical layout: [50, 20, 30, 40], front at physical index 1.

        deque.RemoveAt(1); // Logical [20,30,40,50] -> intends to remove logical index 1 (value 30).

        EXPECT_EQ(deque.getCountProperty(), 3);
        // NOT the mathematically-correct {20, 40, 50} -- element 50 is lost/orphaned (its
        // physical slot is shifted out of the new logical range) and a stale default-constructed
        // element appears instead. Matches upstream's actual (buggy) output.
        EXPECT_EQ(deque.getItem(0).Value, 20);
        EXPECT_EQ(deque.getItem(1).Value, 40);
        EXPECT_EQ(deque.getItem(2).Value, 0)
            << "If this starts failing, upstream's RemoveAt wraparound bug may have been fixed and this port's fidelity note should be revisited.";
    }

    TEST(DequeTests, IndexerThrowsForOutOfRangeIndex)
    {
        const Deque<TestDequeElement> deque(MakeElements(2));
        EXPECT_THROW((void)deque[5], std::out_of_range);
        EXPECT_THROW((void)deque[-1], std::out_of_range);
    }

    TEST(DequeTests, SetCapacityLessThanCountThrows)
    {
        Deque<TestDequeElement> deque(MakeElements(3));
        EXPECT_THROW(deque.setCapacityProperty(1), std::invalid_argument);
    }

    TEST(DequeTests, ConstructorWithNegativeCapacityThrows)
    {
        EXPECT_THROW((Deque<TestDequeElement>(-1)), std::invalid_argument);
    }

    TEST(DequeTests, IsReadOnlyIsAlwaysFalse)
    {
        const Deque<TestDequeElement> deque;
        EXPECT_FALSE(deque.getIsReadOnlyProperty());
    }

    TEST(DequeTests, ICollectionAddDelegatesToAddToBack)
    {
        Deque<TestDequeElement> deque;
        System::Collections::Generic::ICollection<TestDequeElement>& asICollection = deque;
        asICollection.Add(TestDequeElement{7});
        EXPECT_EQ(deque.getCountProperty(), 1);
        EXPECT_EQ(deque.getItem(0).Value, 7);
    }

    TEST(DequeTests, GetEnumeratorMatchesRangeBasedForIteration)
    {
        const Deque<TestDequeElement> deque(MakeElements(3));
        System::Collections::Generic::IEnumerator<TestDequeElement>* enumerator = const_cast<Deque<TestDequeElement>&>(deque).GetEnumerator();

        int expected = 0;
        while (enumerator->MoveNext())
        {
            EXPECT_EQ(enumerator->Current().Value, expected);
            expected++;
        }
        EXPECT_EQ(expected, 3);
        delete enumerator;
    }

    TEST(DequeTests, IListTrackedIndexerAndExplicitAccessorsUseTheSameSlot)
    {
        Deque<TestDequeElement> deque(MakeElements(2));
        System::Collections::Generic::IList<TestDequeElement>& asIList = deque;

        EXPECT_EQ(asIList.getItem(0).Value, 0);
        asIList[0] = TestDequeElement{7};
        EXPECT_EQ(deque.getItem(0).Value, 7);

        asIList.setItem(1, TestDequeElement{9});
        EXPECT_EQ(deque.getItem(1).Value, 9);
    }
}
