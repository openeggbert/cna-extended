// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collections/Deque.cs: a circular-array-backed double-ended
// queue implementing sharp-runtime's System::Collections::Generic::IList<T> (matching how
// sharp-runtime's own List<T> implements the same interface -- "reuse sharp-runtime, don't
// re-roll"). C#'s `Func<int,int> ResizeFunction` maps to `std::function<intcs(intcs)>`, matching
// this project's established Func<T> convention (HslColor.hpp's Match/Map).
//
// *** LIKELY UPSTREAM BUG, PRESERVED FOR FIDELITY -- the most significant one found in this
// port so far, flagged prominently, not silently fixed ***
// `IndexOf`'s final step computes `(index - _frontArrayIndex + _items.Length) % _items.Length`
// from the raw `Array.IndexOf` result (`index`), WITHOUT checking whether that result was -1
// (item not found in either physical segment). When the item genuinely isn't in the Deque, this
// formula does NOT return -1 -- it computes some other, non-negative, seemingly-valid-looking
// index instead. Traced by hand (see DequeTests.cpp's regression tests) and confirmed two
// distinct, reachable failure modes:
//   1. If the bogus computed index happens to fall within the "unused" tail of the backing
//      array (>= Count), `RemoveAt` on that index throws an unexpected exception.
//   2. If the bogus computed index happens to fall within [0, Count) -- which depends entirely
//      on `_frontArrayIndex`'s current wraparound offset -- `Remove(item)` for a genuinely-absent
//      `item` SILENTLY REMOVES THE WRONG (unrelated, real) ELEMENT and reports success (`true`),
//      a real data-corruption risk for any code that calls `Remove` on items it isn't certain
//      are present.
// This is preserved exactly (the "wrong index for a not-found item" formula is unchanged from
// upstream) -- IndexOf/Remove genuinely can misbehave for absent items here, just like upstream.
//
// One further consequence is NOT preserved as pure UB, because it can't be: if the formula above
// is evaluated on a *freshly-constructed, never-grown* Deque (`_items.Length == 0`, i.e. the
// default `Deque()` constructor's `_emptyArray` sentinel, never resized by any Add/AddToFront/
// AddToBack call), the modulo divisor is 0. In C#, integer `% 0` throws a *catchable*
// `DivideByZeroException` -- meaning `new Deque<T>().Remove(anything)` genuinely crashes upstream
// too, just via an exception rather than silently. In C++, integer modulo by zero is undefined
// behavior, not a catchable exception -- there is no faithful way to preserve undefined behavior
// as "the bug", since UB is not an observable, reproducible program behavior to preserve. This
// port therefore explicitly detects exactly that one condition (empty backing array) and throws
// `std::domain_error` instead, as the closest available C++ analog to C#'s catchable
// `DivideByZeroException` -- everything else about the bug (wrong index, not -1, for a
// genuinely-absent item once the Deque has any capacity at all) is left exactly as upstream.
//
// *** SECOND AND THIRD LIKELY UPSTREAM BUGS, PRESERVED FOR FIDELITY -- RemoveAt's middle-index
// removal is unreliable outside a narrow case, confirmed EMPIRICALLY (not just by hand-tracing)
// by running this port's own tests against it ***
// `RemoveAt(index)` for an index strictly between the front and back has two branches
// ("shift the shorter half" toward the removed slot, chosen by `index < Count / 2`). Both
// branches pick their shift's *source* range from FIXED physical positions (`0` for the
// front-half branch; `_items.Length / 2` for the back-half branch) rather than positions derived
// from `_frontArrayIndex`, and both share the same `if (_frontArrayIndex < arrayIndex)
// _frontArrayIndex = (_frontArrayIndex + 1) % _items.Length;` adjustment regardless of which
// branch ran. Tracing (and then empirically confirming via this port's own test suite, since
// this port is a faithful 1:1 translation of the algorithm) found:
//   - Front-half-shift branch on a NON-wrapped buffer (`_frontArrayIndex == 0`): CORRECT.
//     (`DequeTests.cpp`'s `RemoveAtMiddleShiftsShorterFrontHalf`.)
//   - Back-half-shift branch, even on the SAME non-wrapped buffer: BROKEN -- neither removes the
//     intended element nor preserves the order/values of the rest, because `arrayCenterIndex`
//     has no relationship to where the removal point or the front actually are.
//     (`RemoveAtMiddleShiftsShorterBackHalfReproducesKnownUpstreamBug`.)
//   - Front-half-shift branch once the buffer HAS wrapped (`_frontArrayIndex != 0`): ALSO
//     BROKEN -- an element can be silently lost/orphaned (its physical slot ends up outside the
//     new logical range after `_frontArrayIndex` is adjusted), replaced by a stale
//     default-constructed value. (`RemoveAtMiddleOnWrappedBufferReproducesKnownUpstreamBug`.)
// In short: `RemoveAt` on a middle index is only confirmed reliable for a Deque that has never
// wrapped AND happens to pick the front-half branch. This was not caught by upstream's own test
// suite because its one relevant test (`Deque_Remove`) only asserts that `Count` decrements
// correctly after each `RemoveAt` call, never that the *remaining values* are correct. All of
// this is reproduced exactly (not fixed) below -- the C++ translation is algorithmically
// identical to the C# source, verified line-by-line; this is upstream's behavior, not a
// translation defect.
//
// Other, smaller preserved quirks:
//   - `CopyTo` validates `destinationIndex >= destination.size()` UNCONDITIONALLY, before the
//     `Count == 0` early-return check -- meaning `CopyTo(emptyDestination, 0)` throws even
//     though there is nothing to copy, rather than succeeding as a no-op. Preserved as-is.
//   - `IList<T>::Insert` always throws (upstream: `void IList<T>.Insert(...) =>
//     throw new NotImplementedException();`, an explicit-interface-implementation-only member --
//     Deque<T> only ever supports insertion at the front or back). Matches the precedent set by
//     `OrientedRectangle::setPositionProperty` (task 17: "matches upstream, always throws").
//
// Iteration: like sharp-runtime's own List<T>, this provides both the heap-allocated
// IEnumerator<T>-based GetEnumerator() (to satisfy IEnumerable<T>/ICollection<T>/IList<T>) AND
// plain STL-style begin()/end() for range-based `for` and direct test use.
//
// *** SUBTLE FIDELITY POINT: iteration is intentionally tolerant of concurrent front-removal ***
// Upstream's `GetEnumerator()` is a C# iterator block (`yield return`); its `for` loop re-reads
// `_frontArrayIndex`/`Count` LIVE on every loop-condition check, not just once when iteration
// starts (only the loop *variable*'s initial value and the physical starting offset are captured
// once). This is deliberate and upstream has a dedicated test for it
// (`Deque_ForEach_Iteration_Modified`: repeatedly calling `RemoveFromFront()` from inside a
// `foreach` over the same Deque, and expecting it to visit every remaining element exactly once
// as the front shifts). A naive translation that snapshots `Count`/`frontArrayIndex_` once at
// `begin()`, or that re-derives each element via the *logical* index through `operator[]` (which
// itself reads the CURRENT, not the original, `frontArrayIndex_`), does not reproduce this.
// Instead, `Iterator`/`Enumerator` capture the *physical* starting offset and capacity once (at
// `begin()`/construction time), track a simple visited-count, compute each element's physical
// index directly (`(originalFrontArrayIndex + visitedCount) % originalCapacity` -- verified by
// hand to collapse upstream's two syntactically-different loop shapes, the non-wraparound single
// loop and the wraparound two-loop, into one identical formula for the physical index actually
// visited), and re-check LIVE `frontArrayIndex_ + count_` against `originalFrontArrayIndex +
// visitedCount` for loop termination on every step -- reproducing upstream's live-bound behavior
// exactly for the tested (front-removal-during-iteration) case.
//
// Header-only: a template, matching this project's established convention for generic C# types
// (see Interval<T>, Triangulation/CyclicalList<T>).
#pragma once

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/Generic/IList.hpp"

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <vector>

namespace CNA::Extended::Collections
{
    using SharpRuntime::intcs;

    /**
     * @brief A double-ended queue backed by a circular array; elements can be added to or
     * removed from either the front or the back in O(1) amortized time.
     * @tparam T The type of elements in the deque. Must support default construction,
     * copy-assignment, and operator==.
     */
    template <typename T>
    class Deque : public System::Collections::Generic::IList<T>
    {
    public:
        /** @brief Initializes a new, empty Deque with no initial backing storage. */
        Deque() = default;

        /** @brief Initializes a new Deque containing a copy of the elements of @p collection. */
        explicit Deque(const std::vector<T>& collection) : items_(collection), count_(static_cast<intcs>(collection.size()))
        {
        }

        /**
         * @brief Initializes a new, empty Deque with the specified initial capacity.
         * @throws std::invalid_argument if @p capacity is negative.
         */
        explicit Deque(const intcs capacity)
        {
            if (capacity < 0)
            {
                throw std::invalid_argument("Capacity was less than zero.");
            }
            if (capacity > 0)
            {
                items_.resize(static_cast<std::size_t>(capacity));
            }
        }

        /** @brief Gets the growth-strategy function used to calculate Capacity when a greater capacity is required. */
        [[nodiscard]] const std::function<intcs(intcs)>& getResizeFunctionProperty() const { return resizeFunction_; }

        /** @brief Sets the growth-strategy function. Setting an empty std::function resets it to the default (doubling) strategy. */
        void setResizeFunctionProperty(std::function<intcs(intcs)> value)
        {
            resizeFunction_ = value ? std::move(value) : std::function<intcs(intcs)>(&DefaultResizeFunction);
        }

        /** @brief Gets the total number of elements the backing storage can hold without resizing. */
        [[nodiscard]] intcs getCapacityProperty() const { return static_cast<intcs>(items_.size()); }

        /**
         * @brief Sets the backing storage capacity, reallocating and re-linearizing the circular
         * buffer.
         * @throws std::invalid_argument if @p value is less than Count.
         */
        void setCapacityProperty(const intcs value)
        {
            if (value < count_)
            {
                throw std::invalid_argument("capacity was less than the current size.");
            }
            if (value == getCapacityProperty())
            {
                return;
            }
            if (value == 0)
            {
                items_.clear();
                frontArrayIndex_ = 0;
                return;
            }

            std::vector<T> newItems(static_cast<std::size_t>(value));
            CopyTo(newItems);

            frontArrayIndex_ = 0;
            items_ = std::move(newItems);
        }

        /**
         * @brief Gets the element at the specified logical index.
         * @throws std::out_of_range if @p index is negative or >= Count.
         */
        [[nodiscard]] const T& operator[](const intcs index) const override
        {
            return getItem(index);
        }

        /**
         * @brief Gets the element at the specified logical index without mutating the Deque.
         * @throws std::out_of_range if @p index is negative or >= Count.
         */
        [[nodiscard]] const T& getItem(const intcs index) const override
        {
            const intcs arrayIndex = GetArrayIndex(index);
            if (arrayIndex == -1)
            {
                throw std::out_of_range("Index was out of range. Must be non-negative and less than the size of the collection.");
            }
            return items_[static_cast<std::size_t>(arrayIndex)];
        }

        /**
         * @brief Gets or sets the element at the specified logical index through Sharp
         * Runtime's tracked IList proxy.
         * @throws std::out_of_range if @p index is negative or >= Count.
         */
        System::Collections::detail::ElementReference<T> operator[](const intcs index) override
        {
            const intcs arrayIndex = GetArrayIndex(index);
            if (arrayIndex == -1)
            {
                throw std::out_of_range("Index was out of range. Must be non-negative and less than the size of the collection.");
            }
            return {&items_[static_cast<std::size_t>(arrayIndex)], &mutationCounter_};
        }

        /**
         * @brief Replaces the element at the specified logical index through the same tracked
         * write path as the mutable indexer.
         * @throws std::out_of_range if @p index is negative or >= Count.
         */
        void setItem(const intcs index, const T& value) override
        {
            operator[](index) = value;
        }

        /** @brief Gets the number of elements contained in this Deque. */
        [[nodiscard]] intcs getCountProperty() const override { return count_; }

        [[nodiscard]] bool getIsReadOnlyProperty() const override { return false; }

        /** @brief Adds an item to the back of this Deque. Matches upstream's explicit `ICollection<T>.Add`, which delegates to AddToBack. */
        void Add(const T& item) override { AddToBack(item); }

        /**
         * @brief Searches for the specified element and returns the zero-based logical index of
         * the first occurrence within this Deque, or -1 if not found.
         * @warning See this file's header comment: for a genuinely-absent @p item, this can
         * return a bogus non-negative index instead of -1, matching a likely upstream bug.
         */
        [[nodiscard]] intcs IndexOf(const T& item) const override
        {
            T checkFrontBackItem;
            if (Get(0, checkFrontBackItem) && checkFrontBackItem == item)
            {
                return 0;
            }

            const intcs backIndex = count_ - 1;
            if (Get(backIndex, checkFrontBackItem) && checkFrontBackItem == item)
            {
                return backIndex;
            }

            const intcs capacity = static_cast<intcs>(items_.size());
            intcs index;

            if (count_ <= capacity - frontArrayIndex_)
            {
                index = ArrayIndexOf(item, frontArrayIndex_, count_);
            }
            else
            {
                index = ArrayIndexOf(item, frontArrayIndex_, capacity - frontArrayIndex_);
                if (index < 0)
                {
                    index = ArrayIndexOf(item, 0, frontArrayIndex_ + count_ - capacity);
                }
            }

            if (capacity == 0)
            {
                // See this file's header comment: C#'s equivalent `% 0` throws a catchable
                // DivideByZeroException here (a freshly-constructed, never-grown Deque); C++
                // integer modulo by zero is undefined behavior instead, so this is explicitly
                // guarded and thrown rather than left as UB.
                throw std::domain_error("Deque<T>::IndexOf: division by zero (matches upstream's DivideByZeroException on an empty, never-grown Deque)");
            }

            // Matches upstream exactly: preserved as-is, including for `index == -1` (not
            // found) -- see this file's header comment for the resulting bug.
            return (index - frontArrayIndex_ + capacity) % capacity;
        }

        /**
         * @brief Always throws. Matches upstream's explicit `IList<T>.Insert`, which always
         * throws NotImplementedException -- Deque<T> only supports insertion at the front or
         * back (AddToFront/AddToBack), never at an arbitrary index.
         */
        void Insert(intcs, const T&) override
        {
            throw std::logic_error("Deque<T>::Insert is not implemented (matches upstream MonoGame.Extended's IList<T>.Insert, which always throws)");
        }

        /**
         * @brief Removes the first occurrence of the specified element.
         * @warning See this file's header comment: for a genuinely-absent @p item, this can
         * throw or silently remove an unrelated element instead of returning false, matching a
         * likely upstream bug in IndexOf.
         */
        bool Remove(const T& item) override
        {
            const intcs index = IndexOf(item);
            if (index == -1)
            {
                return false;
            }
            RemoveAt(index);
            return true;
        }

        /**
         * @brief Removes the element at the specified logical index.
         * @throws std::out_of_range if @p index is negative or >= Count.
         */
        void RemoveAt(const intcs index) override
        {
            if (index < 0)
            {
                throw std::out_of_range("Index was less than zero.");
            }
            if (index >= count_)
            {
                throw std::out_of_range("Index was equal or greater than TotalCount.");
            }

            if (index == 0)
            {
                T discarded;
                RemoveFromFront(discarded);
            }
            else if (index == count_ - 1)
            {
                T discarded;
                RemoveFromBack(discarded);
            }
            else if (index < count_ / 2)
            {
                // See this file's header comment (SECOND/THIRD LIKELY UPSTREAM BUGS): this
                // front-half-shift branch is only correct on a NON-wrapped buffer
                // (frontArrayIndex_ == 0); once the buffer has wrapped, an element is silently
                // lost/orphaned. Preserved exactly, not fixed.
                const intcs arrayIndex = GetArrayIndex(index);
                // Shift the array from 0 to before the index to remove by 1 to the right; the
                // element to remove is overwritten by the copy.
                ArrayCopy(items_, 0, items_, 1, arrayIndex);
                items_[0] = T{};
                if (frontArrayIndex_ < arrayIndex)
                {
                    frontArrayIndex_ = (frontArrayIndex_ + 1) % static_cast<intcs>(items_.size());
                }
                count_--;
            }
            else
            {
                // See this file's header comment (SECOND LIKELY UPSTREAM BUG): this
                // back-half-shift branch is broken even on a non-wrapped buffer -- its shift
                // source, arrayCenterIndex, is a fixed physical midpoint with no relationship to
                // the actual removal point, so it neither removes the intended element nor
                // preserves the order/values of the rest. Preserved exactly, not fixed.
                const intcs arrayIndex = GetArrayIndex(index);
                const intcs arrayCenterIndex = static_cast<intcs>(items_.size()) / 2;
                // Shift the array from the center to before the index to remove by 1 to the
                // right; the element to remove is overwritten by the copy.
                ArrayCopy(items_, arrayCenterIndex, items_, arrayCenterIndex + 1, static_cast<intcs>(items_.size()) - 1 - arrayIndex);
                items_[items_.size() - 1] = T{};
                if (frontArrayIndex_ < arrayIndex)
                {
                    frontArrayIndex_ = (frontArrayIndex_ + 1) % static_cast<intcs>(items_.size());
                }
                count_--;
            }
        }

        /** @brief Removes all elements from this Deque. Capacity is left unchanged. */
        void Clear() override
        {
            if (count_ == 0)
            {
                return;
            }

            const intcs capacity = static_cast<intcs>(items_.size());
            if (count_ > capacity - frontArrayIndex_)
            {
                std::fill_n(items_.begin() + frontArrayIndex_, capacity - frontArrayIndex_, T{});
                std::fill_n(items_.begin(), frontArrayIndex_ + count_ - capacity, T{});
            }
            else
            {
                std::fill_n(items_.begin() + frontArrayIndex_, count_, T{});
            }
            count_ = 0;
            frontArrayIndex_ = 0;
        }

        /**
         * @brief Determines whether this Deque contains the specified element. Implemented via
         * direct front-to-back iteration (matching upstream's LINQ-based Contains), NOT via the
         * buggy IndexOf -- Contains itself has no known fidelity issue.
         */
        [[nodiscard]] bool Contains(const T& item) const override
        {
            for (auto it = begin(); it != end(); ++it)
            {
                if (*it == item)
                {
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief Copies the entire Deque, in front-to-back logical order, into @p destination
         * starting at @p destinationIndex.
         * @warning Matches upstream: bounds are validated (and can throw) even when Count == 0,
         * i.e. before it would otherwise be a harmless no-op -- see this file's header comment.
         */
        void CopyTo(std::vector<T>& destination, const intcs destinationIndex = 0) const
        {
            if (destinationIndex < 0)
            {
                throw std::invalid_argument("Index was less than the array's lower bound.");
            }
            if (destinationIndex >= static_cast<intcs>(destination.size()))
            {
                throw std::invalid_argument("Index was greater than the array's upper bound.");
            }
            if (static_cast<intcs>(destination.size()) - destinationIndex < count_)
            {
                throw std::invalid_argument("Destination array was not long enough.");
            }

            if (count_ == 0)
            {
                return;
            }

            const intcs capacity = static_cast<intcs>(items_.size());
            const bool loopsAround = count_ > capacity - frontArrayIndex_;
            if (!loopsAround)
            {
                std::copy_n(items_.begin() + frontArrayIndex_, count_, destination.begin() + destinationIndex);
            }
            else
            {
                const intcs firstSegmentCount = capacity - frontArrayIndex_;
                std::copy_n(items_.begin() + frontArrayIndex_, firstSegmentCount, destination.begin() + destinationIndex);
                const intcs secondSegmentCount = frontArrayIndex_ + (count_ - capacity);
                std::copy_n(items_.begin(), secondSegmentCount, destination.begin() + destinationIndex + firstSegmentCount);
            }
        }

        /**
         * @brief Shrinks Capacity to Count, but only if Count is at most 90% of the current
         * Capacity (matches upstream's threshold exactly).
         */
        void TrimExcess()
        {
            if (static_cast<double>(count_) > static_cast<double>(items_.size()) * 0.9)
            {
                return;
            }
            setCapacityProperty(count_);
        }

        /** @brief Adds an element to the front of this Deque. O(1) amortized. */
        void AddToFront(const T& item)
        {
            EnsureCapacity(count_ + 1);
            frontArrayIndex_ = (frontArrayIndex_ - 1 + static_cast<intcs>(items_.size())) % static_cast<intcs>(items_.size());
            items_[static_cast<std::size_t>(frontArrayIndex_)] = item;
            count_++;
        }

        /** @brief Adds an element to the back of this Deque. O(1) amortized. */
        void AddToBack(const T& item)
        {
            EnsureCapacity(count_ + 1);
            const intcs index = (frontArrayIndex_ + count_) % static_cast<intcs>(items_.size());
            count_++;
            items_[static_cast<std::size_t>(index)] = item;
        }

        /**
         * @brief Gets the element at the specified logical index without throwing.
         * @param index The logical index to retrieve.
         * @param item Receives the element if found; otherwise T{}.
         * @return true if @p index was non-negative and less than Count.
         */
        bool Get(const intcs index, T& item) const
        {
            const intcs arrayIndex = GetArrayIndex(index);
            if (arrayIndex == -1)
            {
                item = T{};
                return false;
            }
            item = items_[static_cast<std::size_t>(arrayIndex)];
            return true;
        }

        /** @brief Gets the element at the front of this Deque without removing it. Returns false if this Deque is empty. */
        bool GetFront(T& item) const { return Get(0, item); }

        /** @brief Gets the element at the back of this Deque without removing it. Returns false if this Deque is empty. */
        bool GetBack(T& item) const { return Get(count_ - 1, item); }

        /**
         * @brief Removes the element at the front of this Deque.
         * @param item Receives the removed element if this Deque was not empty; otherwise T{}.
         * @return true if an element was removed.
         */
        bool RemoveFromFront(T& item)
        {
            if (count_ == 0)
            {
                item = T{};
                return false;
            }
            const intcs index = frontArrayIndex_ % static_cast<intcs>(items_.size());
            item = items_[static_cast<std::size_t>(index)];
            items_[static_cast<std::size_t>(index)] = T{};
            frontArrayIndex_ = (frontArrayIndex_ + 1) % static_cast<intcs>(items_.size());
            count_--;
            return true;
        }

        /** @brief Removes the element at the front of this Deque, discarding it. @return true if an element was removed. */
        bool RemoveFromFront()
        {
            T discarded;
            return RemoveFromFront(discarded);
        }

        /**
         * @brief Removes the element at the back of this Deque.
         * @param item Receives the removed element if this Deque was not empty; otherwise T{}.
         * @return true if an element was removed.
         */
        bool RemoveFromBack(T& item)
        {
            if (count_ == 0)
            {
                item = T{};
                return false;
            }
            const intcs circularBackIndex = (frontArrayIndex_ + (count_ - 1)) % static_cast<intcs>(items_.size());
            item = items_[static_cast<std::size_t>(circularBackIndex)];
            items_[static_cast<std::size_t>(circularBackIndex)] = T{};
            count_--;
            return true;
        }

        /** @brief Removes the element at the back of this Deque, discarding it. @return true if an element was removed. */
        bool RemoveFromBack()
        {
            T discarded;
            return RemoveFromBack(discarded);
        }

        /**
         * @brief Removes and returns the element at the back of this Deque.
         * @throws std::logic_error if this Deque is empty (matches upstream's InvalidOperationException).
         */
        [[nodiscard]] T Pop()
        {
            T item;
            if (RemoveFromBack(item))
            {
                return item;
            }
            throw std::logic_error("Deque<T>::Pop() called on an empty deque (matches upstream's InvalidOperationException)");
        }

        /**
         * @brief A forward iterator walking this Deque's elements in front-to-back logical
         * order. See this file's header comment for why this tolerates (and is required to
         * tolerate) front-removal during iteration, matching upstream.
         */
        class Iterator
        {
        public:
            Iterator(const Deque* deque, const intcs originalFrontArrayIndex, const intcs originalCapacity, const intcs visitedCount)
                : deque_(deque), originalFrontArrayIndex_(originalFrontArrayIndex), originalCapacity_(originalCapacity), visitedCount_(visitedCount)
            {
            }

            const T& operator*() const
            {
                const intcs physicalIndex = originalCapacity_ != 0 ? (originalFrontArrayIndex_ + visitedCount_) % originalCapacity_ : 0;
                return deque_->items_[static_cast<std::size_t>(physicalIndex)];
            }

            Iterator& operator++()
            {
                ++visitedCount_;
                return *this;
            }

            // `other` is unused: range-based `for` always compares against a cached `end()`, and
            // termination depends only on this iterator's captured starting state plus the
            // deque's CURRENT live state (see header comment) -- not on `other`'s state.
            bool operator!=(const Iterator&) const { return originalFrontArrayIndex_ + visitedCount_ < deque_->frontArrayIndex_ + deque_->count_; }
            bool operator==(const Iterator& other) const { return !(*this != other); }

        private:
            const Deque* deque_;
            intcs originalFrontArrayIndex_;
            intcs originalCapacity_;
            intcs visitedCount_;
        };

        [[nodiscard]] Iterator begin() const { return Iterator(this, frontArrayIndex_, static_cast<intcs>(items_.size()), 0); }
        [[nodiscard]] Iterator end() const { return Iterator(this, frontArrayIndex_, static_cast<intcs>(items_.size()), count_); }

        /**
         * @brief IEnumerator<T>-compatible enumerator, satisfying IEnumerable<T>::GetEnumerator().
         * Mirrors Iterator's physical-index/live-termination logic exactly (see header comment).
         */
        class Enumerator : public System::Collections::Generic::IEnumerator<T>
        {
        public:
            explicit Enumerator(const Deque& deque)
                : deque_(deque), originalFrontArrayIndex_(deque.frontArrayIndex_), originalCapacity_(static_cast<intcs>(deque.items_.size()))
            {
            }

            bool MoveNext() override
            {
                ++visitedCount_;
                return originalFrontArrayIndex_ + visitedCount_ < deque_.frontArrayIndex_ + deque_.count_;
            }

            void Reset() override { visitedCount_ = -1; }

            [[nodiscard]] const T& Current() const override
            {
                const intcs physicalIndex = originalCapacity_ != 0 ? (originalFrontArrayIndex_ + visitedCount_) % originalCapacity_ : 0;
                return deque_.items_[static_cast<std::size_t>(physicalIndex)];
            }

        private:
            const Deque& deque_;
            intcs originalFrontArrayIndex_;
            intcs originalCapacity_;
            intcs visitedCount_ = -1;
        };

        /** @brief Returns a heap-allocated enumerator; caller takes ownership (matches sharp-runtime's IEnumerable<T> contract). */
        System::Collections::Generic::IEnumerator<T>* GetEnumerator() override { return new Enumerator(*this); }

    private:
        static intcs DefaultResizeFunction(const intcs x) { return x * 2; }

        [[nodiscard]] intcs GetArrayIndex(const intcs index) const
        {
            if (index < 0 || index >= count_)
            {
                return -1;
            }
            return !items_.empty() ? (frontArrayIndex_ + index) % static_cast<intcs>(items_.size()) : 0;
        }

        void EnsureCapacity(const intcs minimumCapacity)
        {
            if (static_cast<intcs>(items_.size()) >= minimumCapacity)
            {
                return;
            }
            intcs newCapacity = kDefaultCapacity;
            if (!items_.empty())
            {
                newCapacity = resizeFunction_(static_cast<intcs>(items_.size()));
            }
            newCapacity = std::max(newCapacity, minimumCapacity);
            setCapacityProperty(newCapacity);
        }

        [[nodiscard]] intcs ArrayIndexOf(const T& item, const intcs startIndex, const intcs length) const
        {
            for (intcs i = startIndex; i < startIndex + length; i++)
            {
                if (items_[static_cast<std::size_t>(i)] == item)
                {
                    return i;
                }
            }
            return -1;
        }

        // Matches .NET's Array.Copy semantics: correctly handles overlapping in-place shifts
        // (used by RemoveAt) regardless of shift direction, like memmove.
        static void ArrayCopy(
            std::vector<T>& source, const intcs sourceIndex, std::vector<T>& destination, const intcs destinationIndex, const intcs length)
        {
            if (length <= 0)
            {
                return;
            }
            if (&source == &destination && destinationIndex > sourceIndex && destinationIndex < sourceIndex + length)
            {
                std::copy_backward(source.begin() + sourceIndex, source.begin() + sourceIndex + length, destination.begin() + destinationIndex + length);
            }
            else
            {
                std::copy(source.begin() + sourceIndex, source.begin() + sourceIndex + length, destination.begin() + destinationIndex);
            }
        }

        static constexpr intcs kDefaultCapacity = 4;

        std::vector<T> items_;
        intcs frontArrayIndex_ = 0;
        intcs count_ = 0;
        std::function<intcs(intcs)> resizeFunction_ = &DefaultResizeFunction;
        // Deque's upstream iterator deliberately tolerates front removal and therefore does
        // not consume this counter. It is still required by IList<T>'s tracked mutable
        // indexer so indexed writes cannot escape through an untracked mutable reference.
        System::Collections::detail::MutationCounter mutationCounter_;
    };
}
