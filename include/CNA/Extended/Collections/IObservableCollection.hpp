// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collections/IObservableCollection.cs: interface for
// collections that can be observed. `event EventHandler<ItemEventArgs<T>> ItemAdded/
// ItemRemoved` / `event EventHandler Clearing/Cleared` -> pure virtual `getXEvent()` accessors
// returning `System::EventHandler<...>&`, matching CNA's own `IUpdateable` interface's
// established translation of the identical C# "interface exposing an event" shape
// (`getEnabledChangedEvent()`/`getUpdateOrderChangedEvent()`).
#pragma once

#include "CNA/Extended/Collections/ItemEventArgs.hpp"
#include "System/EventArgs.hpp"
#include "System/EventHandler.hpp"

namespace CNA::Extended::Collections
{
    /**
     * @brief Interface for collections that can be observed.
     * @tparam T Type of items managed in the collection.
     */
    template <typename T>
    class IObservableCollection
    {
    public:
        /** @brief Virtual destructor. */
        virtual ~IObservableCollection() = default;

        /** @brief Returns the ItemAdded event, raised when an item has been added to the collection. */
        [[nodiscard]] virtual System::EventHandler<ItemEventArgs<T>>& getItemAddedEvent() = 0;

        /** @brief Returns the ItemRemoved event, raised when an item is removed from the collection. */
        [[nodiscard]] virtual System::EventHandler<ItemEventArgs<T>>& getItemRemovedEvent() = 0;

        /**
         * @brief Returns the Clearing event, raised when the collection is about to be cleared.
         *
         * This could be covered by calling ItemRemoved for each item currently contained in the
         * collection, but it is often simpler and more efficient to process the clearing of the
         * entire collection as a special operation.
         */
        [[nodiscard]] virtual System::EventHandler<System::EventArgs>& getClearingEvent() = 0;

        /** @brief Returns the Cleared event, raised when the collection has been cleared of its items. */
        [[nodiscard]] virtual System::EventHandler<System::EventArgs>& getClearedEvent() = 0;
    };
}
