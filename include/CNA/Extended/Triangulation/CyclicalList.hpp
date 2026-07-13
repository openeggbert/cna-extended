// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Triangulation/CyclicalList.cs. Upstream's own file header
// additionally credits nickgravelyn's Triangulator project
// (https://github.com/nickgravelyn/Triangulator, an ear-clipping triangulation implementation)
// as the origin of this code ("MIT Licensed"). Carried forward here for the same reason
// MonoGame.Extended carries it: this code traces back further than Craftwork Games. See
// NOTICE.md for a provenance note about this specific attribution.
//
// This is an internal implementation-detail collection used only by Triangulator (upstream's own
// doc comment on Triangulator: "This is the sole public class in the entire library; all other
// classes/structures are intended as internal-only objects"). Upstream implements it as
// `class CyclicalList<T> : List<T>` with a `new T this[int]` cyclical indexer and `new RemoveAt`
// hiding the base List<T> members via C#'s `new` keyword. C++ has no equivalent method-hiding
// mechanism worth replicating for an internal type with no public API surface, so this composes
// (rather than inherits) sharp-runtime's System::Collections::Generic::List<T> instead, per this
// project's "reuse sharp-runtime, don't re-roll" convention -- List<T> already provides exactly
// the operations Triangulator.cs actually uses (Add, Contains, Remove, IndexOf, Clear, begin/end);
// only the cyclical operator[]/RemoveAt are added here. Header-only template, matching the
// precedent set by Interval<T> (include/CNA/Extended/Interval.hpp) for header-only generic types
// in this project -- even though in practice this is only ever instantiated with Vertex.
#pragma once

#include "System/Collections/Generic/List.hpp"

namespace CNA::Extended::Triangulation
{
    using SharpRuntime::intcs;

    /** @brief A List-like structure where out-of-range indices wrap cyclically instead of throwing. */
    template <typename T>
    class CyclicalList
    {
    public:
        CyclicalList() = default;

        /** @brief Initializes a new CyclicalList from the specified collection of items. */
        explicit CyclicalList(const std::vector<T>& collection)
        {
            for (const T& item : collection)
            {
                items_.Add(item);
            }
        }

        [[nodiscard]] intcs getCountProperty() const { return items_.getCountProperty(); }

        void Add(const T& item) { items_.Add(item); }

        void Clear() { items_.Clear(); }

        [[nodiscard]] bool Contains(const T& item) const { return items_.Contains(item); }

        bool Remove(const T& item) { return items_.Remove(item); }

        /** @brief Gets the item at @p index, wrapping cyclically if out of range. */
        [[nodiscard]] const T& operator[](intcs index) const { return items_[WrapIndex(index)]; }

        /** @brief Gets or sets the item at @p index, wrapping cyclically if out of range. */
        T& operator[](intcs index) { return items_[WrapIndex(index)]; }

        /** @brief Removes the item at @p index (wrapping cyclically if out of range), matching upstream's `Remove(this[index])`. */
        void RemoveAt(intcs index) { Remove((*this)[index]); }

        /** @brief Finds the (non-cyclical) index of @p item, or -1 if not found. Matches upstream: CyclicalList does NOT override IndexOf. */
        [[nodiscard]] intcs IndexOf(const T& item) const { return items_.IndexOf(item); }

        auto begin() { return items_.begin(); }
        auto end() { return items_.end(); }
        [[nodiscard]] auto begin() const { return items_.begin(); }
        [[nodiscard]] auto end() const { return items_.end(); }

    private:
        [[nodiscard]] intcs WrapIndex(intcs index) const
        {
            const intcs count = getCountProperty();
            while (index < 0)
            {
                index = count + index;
            }
            if (index >= count)
            {
                index %= count;
            }
            return index;
        }

        System::Collections::Generic::List<T> items_;
    };
}
