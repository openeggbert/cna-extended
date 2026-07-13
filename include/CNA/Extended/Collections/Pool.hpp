// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collections/Pool.cs: a simple object pool with no intrusive
// linked-list/IPoolable requirement (unlike ObjectPool<T> in the same folder). C#'s
// `Func<T> createItem`/`Action<T> resetItem` map to `std::function<T*()>`/
// `std::function<void(T*)>`, matching this project's established Func/Action-to-std::function
// convention. T is a C# reference-type constraint (`where T : class`), so this pool works with
// T* throughout, matching how ObjectPool<T>/GameComponentCollectionExtensions.hpp translate C#
// reference-type generic parameters. Backed by the just-ported Deque<T*>.
//
// Note (preserved, not "fixed"): Free() calls resetItem unconditionally, even when the item was
// NOT actually kept because _freeItems was already at _maximum -- matches upstream exactly (see
// Free() below).
//
// Header-only: a template, matching this project's established convention for generic C# types.
#pragma once

#include "CNA/Extended/Collections/Deque.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

#include <functional>
#include <limits>
#include <stdexcept>

namespace CNA::Extended::Collections
{
    using SharpRuntime::intcs;

    /**
     * @brief A simple pool of reusable T* instances, backed by a circular-buffer Deque.
     * @tparam T The pooled type. Instances are managed by pointer (matching C#'s `where T : class`
     * reference-type constraint).
     */
    template <typename T>
    class Pool
    {
    public:
        /**
         * @brief Initializes a new Pool with the specified creation and reset functions.
         * @param createItem Factory function invoked when Obtain() finds no free item available.
         * @param resetItem Function invoked on every item passed to Free(), whether or not it was
         * actually kept.
         * @param capacity The initial backing-storage capacity of the free-item Deque.
         * @param maximum The maximum number of free items retained; Free() beyond this discards
         * the item (after still calling resetItem on it).
         */
        explicit Pool(std::function<T*()> createItem, std::function<void(T*)> resetItem, const intcs capacity = 16,
            const intcs maximum = std::numeric_limits<intcs>::max())
            : createItem_(std::move(createItem)), resetItem_(std::move(resetItem)), freeItems_(capacity), maximum_(maximum)
        {
        }

        /** @brief Initializes a new Pool with the specified creation function and a no-op reset function. */
        explicit Pool(
            std::function<T*()> createItem, const intcs capacity = 16, const intcs maximum = std::numeric_limits<intcs>::max())
            : Pool(
                  std::move(createItem), [](T*) {}, capacity, maximum)
        {
        }

        /** @brief Gets the number of free items currently available for Obtain(). */
        [[nodiscard]] intcs getAvailableCountProperty() const { return freeItems_.getCountProperty(); }

        /** @brief Obtains an item: a free one if available, otherwise a newly-created one. */
        [[nodiscard]] T* Obtain()
        {
            if (freeItems_.getCountProperty() > 0)
            {
                return freeItems_.Pop();
            }
            return createItem_();
        }

        /**
         * @brief Returns an item to the pool, resetting it (whether or not it was actually kept
         * -- see this file's header comment).
         * @throws std::invalid_argument if @p item is nullptr.
         */
        void Free(T* item)
        {
            if (item == nullptr)
            {
                throw std::invalid_argument("item");
            }
            if (freeItems_.getCountProperty() < maximum_)
            {
                freeItems_.AddToBack(item);
            }
            resetItem_(item);
        }

        /** @brief Removes all free items from the pool. */
        void Clear() { freeItems_.Clear(); }

    private:
        std::function<T*()> createItem_;
        std::function<void(T*)> resetItem_;
        Deque<T*> freeItems_;
        intcs maximum_;
    };
}
