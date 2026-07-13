// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collections/Bag.cs. Unlike every other file ported so far,
// Bag.cs's own upstream header credits a *BSD-licensed* (not MIT) origin: GAMADU.COM's C# port
// of thelinuxlich's artemis_CSharp Bag<T>
// (https://github.com/thelinuxlich/artemis_CSharp/blob/master/Artemis_XNA_INDEPENDENT/Utils/Bag.cs),
// Copyright (c) 2013 GAMADU.COM, under a 2-clause-BSD-style license. See NOTICE.md's
// "Code directly derived from other permissively-licensed (non-MIT) projects" section for the
// full reproduced license text and provenance details.
//
// Two deliberate, purely-mechanical simplifications with no observable behavioral consequence:
//   - Upstream's `_isPrimitive` (`typeof(T).IsPrimitive`) exists solely to skip `Array.Clear` for
//     primitive T as a garbage-collector hygiene hint -- C++ has no GC to hint, so this always
//     clears emptied/removed slots to `T{}` unconditionally instead. `Count` bookkeeping is
//     identical either way; nothing about the public API's observable behavior changes.
//   - Upstream's `BagEnumerator` struct plus the `IEnumerable<T>.GetEnumerator()`/
//     `IEnumerable.GetEnumerator()` triad exist solely so C#'s `foreach` can avoid
//     heap-allocating/boxing an enumerator -- a struct-vs-interface distinction with no C++
//     analog (range-based `for` over `begin()/end()` never allocates regardless). Replaced with
//     plain `begin()`/`end()` (mutable and const) iterating the backing array's first `Count`
//     elements directly.
//
// Upstream's `this[int index]` is a single C# indexed property with asymmetric get/set behavior
// (get returns `default(T)` for an out-of-range index without growing; set auto-grows the Bag).
// C++'s `operator[]` cannot express two different behaviors for read vs. write through the same
// syntax without returning a reference from both (which the get side can't safely do for an
// out-of-range index, since there is no live T to reference). Split into a read-only
// `operator[](index) const` returning by value (matching the get side exactly, including
// returning `T{}` rather than growing/throwing) and a separate `Set(index, value)` method
// (matching the set side exactly, including auto-growing).
//
// Header-only: a template, matching this project's established convention for generic C# types
// (see Interval<T>, Triangulation/CyclicalList<T>).
#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace CNA::Extended::Collections
{
    /**
     * @brief An unordered, growable collection with O(1) swap-with-last removal.
     * @tparam T The type of elements in the bag. Must support default construction,
     * copy-assignment, and operator==.
     */
    template <typename T>
    class Bag
    {
    public:
        /**
         * @brief Initializes a new Bag with the specified initial capacity.
         * @param capacity The initial backing-array capacity. Defaults to 16, matching upstream.
         */
        explicit Bag(const std::size_t capacity = 16) : items_(capacity)
        {
        }

        /** @brief Gets the total capacity of the backing array. */
        [[nodiscard]] std::size_t getCapacityProperty() const { return items_.size(); }

        /** @brief Gets whether this Bag contains no elements. */
        [[nodiscard]] bool getIsEmptyProperty() const { return count_ == 0; }

        /** @brief Gets the number of elements in this Bag. */
        [[nodiscard]] std::size_t getCountProperty() const { return count_; }

        /**
         * @brief Gets or sets the element at the specified index.
         * @param index The index to access.
         * @return The element at @p index, or T{} if @p index is beyond the current capacity
         * (matches upstream: get never throws for an out-of-range index).
         */
        [[nodiscard]] T operator[](const std::size_t index) const { return index >= items_.size() ? T{} : items_[index]; }

        /**
         * @brief Sets the element at the specified index, growing the Bag (and, if necessary,
         * Count) as needed.
         */
        void Set(const std::size_t index, const T& value)
        {
            EnsureCapacity(index + 1);
            if (index >= count_)
            {
                count_ = index + 1;
            }
            items_[index] = value;
        }

        /** @brief Adds an element to the end of this Bag. */
        void Add(const T& element)
        {
            EnsureCapacity(count_ + 1);
            items_[count_] = element;
            ++count_;
        }

        /** @brief Adds every element of @p range to the end of this Bag. */
        void AddRange(const Bag& range)
        {
            for (std::size_t index = 0; index < range.count_; ++index)
            {
                Add(range[index]);
            }
        }

        /** @brief Removes all elements from this Bag, resetting Count to 0. */
        void Clear()
        {
            if (count_ == 0)
            {
                return;
            }
            std::fill_n(items_.begin(), count_, T{});
            count_ = 0;
        }

        /** @brief Determines whether this Bag contains the specified element. */
        [[nodiscard]] bool Contains(const T& element) const
        {
            for (std::size_t index = count_; index > 0; --index)
            {
                if (element == items_[index - 1])
                {
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief Removes the element at the specified index by swapping it with the last
         * element (does NOT preserve order).
         * @return The removed element.
         */
        T RemoveAt(const std::size_t index)
        {
            const T result = items_[index];
            --count_;
            items_[index] = items_[count_];
            items_[count_] = T{};
            return result;
        }

        /**
         * @brief Removes the first occurrence of the specified element by swapping it with the
         * last element (does NOT preserve order).
         */
        bool Remove(const T& element)
        {
            for (std::size_t index = count_; index > 0; --index)
            {
                if (element == items_[index - 1])
                {
                    --count_;
                    items_[index - 1] = items_[count_];
                    items_[count_] = T{};
                    return true;
                }
            }
            return false;
        }

        /** @brief Removes every element of @p bag from this Bag. Returns true if at least one element was removed. */
        bool RemoveAll(const Bag& bag)
        {
            bool isResult = false;
            for (std::size_t index = bag.count_; index > 0; --index)
            {
                if (Remove(bag[index - 1]))
                {
                    isResult = true;
                }
            }
            return isResult;
        }

        [[nodiscard]] auto begin() { return items_.begin(); }
        [[nodiscard]] auto end() { return items_.begin() + static_cast<std::ptrdiff_t>(count_); }
        [[nodiscard]] auto begin() const { return items_.begin(); }
        [[nodiscard]] auto end() const { return items_.begin() + static_cast<std::ptrdiff_t>(count_); }

    private:
        void EnsureCapacity(const std::size_t capacity)
        {
            if (capacity < items_.size())
            {
                return;
            }
            const std::size_t newCapacity = std::max(static_cast<std::size_t>(static_cast<double>(items_.size()) * 1.5), capacity);
            items_.resize(newCapacity);
        }

        std::vector<T> items_;
        std::size_t count_ = 0;
    };
}
