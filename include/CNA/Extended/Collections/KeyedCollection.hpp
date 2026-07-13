// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collections/KeyedCollection.cs: a collection of values keyed
// by a key derived from each value via a supplied function, backed by a dictionary.
// `Func<TValue, TKey>` maps to `std::function<TKey(const TValue&)>`, matching this project's
// established Func<T>-to-std::function convention. Backed by sharp-runtime's own
// `System::Collections::Generic::Dictionary<TKey, TValue>` (its `operator[](key) const` already
// throws `KeyNotFoundException` for a missing key, exactly matching C#'s Dictionary indexer),
// per this project's "reuse sharp-runtime, don't re-roll" convention. `CopyTo` matches upstream:
// always throws (`NotSupportedException` upstream), since this type doesn't support ordered
// array-copy semantics. Enumeration walks VALUES only (matching upstream's
// `_dictionary.Values.GetEnumerator()`), via a small iterator over the backing dictionary.
//
// Header-only: a template, matching this project's established convention for generic C# types.
#pragma once

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/Generic/Dictionary.hpp"

#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace CNA::Extended::Collections
{
    using SharpRuntime::intcs;

    /**
     * @brief A collection of values keyed by a key derived from each value.
     * @tparam TKey The key type, derived from each TValue via the function supplied to the
     * constructor.
     * @tparam TValue The value type stored in this collection.
     */
    template <typename TKey, typename TValue>
    class KeyedCollection
    {
    public:
        /** @brief Initializes a new KeyedCollection using @p getKey to derive each value's key. */
        explicit KeyedCollection(std::function<TKey(const TValue&)> getKey) : getKey_(std::move(getKey))
        {
        }

        /** @brief Gets the value associated with the specified key. @throws System::Collections::Generic::KeyNotFoundException if @p key is absent. */
        [[nodiscard]] const TValue& operator[](const TKey& key) const { return dictionary_[key]; }

        /** @brief Gets the keys in this collection. */
        [[nodiscard]] std::vector<TKey> getKeysProperty() const { return dictionary_.getKeysProperty(); }

        /** @brief Gets the values in this collection. */
        [[nodiscard]] std::vector<TValue> getValuesProperty() const { return dictionary_.getValuesProperty(); }

        /** @brief Gets the number of values in this collection. */
        [[nodiscard]] intcs getCountProperty() const { return dictionary_.getCountProperty(); }

        [[nodiscard]] bool getIsReadOnlyProperty() const { return false; }

        /** @brief Adds a value, deriving its key via the function supplied to the constructor. */
        void Add(const TValue& item) { dictionary_.Add(getKey_(item), item); }

        /** @brief Removes all values from this collection. */
        void Clear() { dictionary_.Clear(); }

        /** @brief Determines whether this collection contains a value whose derived key matches @p item's. */
        [[nodiscard]] bool Contains(const TValue& item) const { return dictionary_.ContainsKey(getKey_(item)); }

        /** @brief Always throws. Matches upstream, which does not support this operation. */
        void CopyTo(TValue*, intcs) const
        {
            throw std::logic_error("KeyedCollection<TKey, TValue>::CopyTo is not supported (matches upstream MonoGame.Extended's NotSupportedException)");
        }

        /** @brief Removes the value whose derived key matches @p item's. */
        bool Remove(const TValue& item) { return dictionary_.Remove(getKey_(item)); }

        /** @brief Determines whether this collection contains a value associated with @p key. */
        [[nodiscard]] bool ContainsKey(const TKey& key) const { return dictionary_.ContainsKey(key); }

        /** @brief Attempts to get the value associated with @p key. */
        bool TryGetValue(const TKey& key, TValue& value) const { return dictionary_.TryGetValue(key, value); }

        /** @brief A forward iterator over this collection's values (not key/value pairs), matching upstream's Values-only enumeration. */
        class Iterator
        {
        public:
            explicit Iterator(typename std::unordered_map<TKey, TValue>::const_iterator it) : it_(it) {}

            const TValue& operator*() const { return it_->second; }
            Iterator& operator++()
            {
                ++it_;
                return *this;
            }
            bool operator!=(const Iterator& other) const { return it_ != other.it_; }
            bool operator==(const Iterator& other) const { return it_ == other.it_; }

        private:
            typename std::unordered_map<TKey, TValue>::const_iterator it_;
        };

        [[nodiscard]] Iterator begin() const { return Iterator(dictionary_.ToMap().begin()); }
        [[nodiscard]] Iterator end() const { return Iterator(dictionary_.ToMap().end()); }

    private:
        std::function<TKey(const TValue&)> getKey_;
        System::Collections::Generic::Dictionary<TKey, TValue> dictionary_;
    };
}
