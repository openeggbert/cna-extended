// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collections/ItemEventArgs.cs: a generic EventArgs subclass for
// collections that hand over an item in an event. Header-only: a template, matching this
// project's established convention for generic C# types (see Interval<T>).
#pragma once

#include "System/EventArgs.hpp"

#include <utility>

namespace CNA::Extended::Collections
{
    /** @brief Arguments class for collections wanting to hand over an item in an event. */
    template <typename T>
    class ItemEventArgs : public System::EventArgs
    {
    public:
        /** @brief Initializes a new event arguments supplier for the specified item. */
        explicit ItemEventArgs(T item) : Item_(std::move(item))
        {
        }

        /** @brief Gets the collection item the event arguments are carrying. */
        [[nodiscard]] const T& getItemProperty() const { return Item_; }

    private:
        T Item_;
    };
}
