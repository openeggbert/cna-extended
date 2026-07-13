// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Triangulation/IndexableCyclicalLinkedList.cs. Upstream's
// own file header additionally credits nickgravelyn's Triangulator project
// (https://github.com/nickgravelyn/Triangulator, an ear-clipping triangulation implementation)
// as the origin of this code ("MIT Licensed"). Carried forward here for the same reason
// MonoGame.Extended carries it: this code traces back further than Craftwork Games. See
// NOTICE.md for a provenance note about this specific attribution.
//
// Internal implementation-detail collection, same rationale as CyclicalList.hpp: composes
// (rather than inherits) sharp-runtime's System::Collections::Generic::LinkedList<T> instead of
// replicating upstream's `class IndexableCyclicalLinkedList<T> : LinkedList<T>` inheritance, per
// this project's "reuse sharp-runtime, don't re-roll" convention -- LinkedList<T> already
// provides everything Triangulator.cs actually uses (AddFirst, AddLast, AddAfter, Remove,
// Contains, Clear, begin/end); only the cyclical indexer, RemoveAt, and IndexOf are added here.
// Header-only template, matching Interval<T>'s precedent (include/CNA/Extended/Interval.hpp).
#pragma once

#include "System/Collections/Generic/LinkedList.hpp"

namespace CNA::Extended::Triangulation
{
    using SharpRuntime::intcs;

    /**
     * @brief A LinkedList-like structure that is both indexable and cyclical: indexing with an
     * out-of-range index automatically wraps around the list to find a valid node.
     */
    template <typename T>
    class IndexableCyclicalLinkedList
    {
    public:
        using Node = System::Collections::Generic::LinkedListNode<T>;

        IndexableCyclicalLinkedList() = default;

        [[nodiscard]] intcs getCountProperty() const { return list_.getCountProperty(); }

        Node AddFirst(const T& value) { return list_.AddFirst(value); }
        Node AddLast(const T& value) { return list_.AddLast(value); }
        Node AddAfter(Node node, const T& value) { return list_.AddAfter(node, value); }

        void Clear() { list_.Clear(); }

        bool Remove(const T& value) { return list_.Remove(value); }
        void Remove(Node node) { list_.Remove(node); }

        [[nodiscard]] bool Contains(const T& value) const { return list_.Contains(value); }

        /** @brief Gets the Node at @p index, wrapping cyclically if out of range. */
        [[nodiscard]] Node operator[](intcs index) const
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

            Node node = list_.getFirstProperty();
            for (intcs i = 0; i < index; i++)
            {
                node = node.getNextProperty();
            }

            return node;
        }

        /** @brief Removes the Node at @p index (wrapping cyclically if out of range). */
        void RemoveAt(intcs index) { Remove((*this)[index]); }

        /** @brief Finds the (cyclically-indexed) position of @p item, or -1 if not found. */
        [[nodiscard]] intcs IndexOf(const T& item) const
        {
            const intcs count = getCountProperty();
            for (intcs i = 0; i < count; i++)
            {
                if ((*this)[i].getValueProperty() == item)
                {
                    return i;
                }
            }

            return -1;
        }

        auto begin() { return list_.begin(); }
        auto end() { return list_.end(); }
        [[nodiscard]] auto begin() const { return list_.begin(); }
        [[nodiscard]] auto end() const { return list_.end(); }

    private:
        System::Collections::Generic::LinkedList<T> list_;
    };
}
