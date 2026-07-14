// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collections/ObservableCollection.cs and
// Collections/IObservableCollection.cs: a Collection<T> that raises events on add/remove/clear.
//
// *** Distinct from sharp-runtime's own System::Collections::ObjectModel::ObservableCollection ***
// plan.md originally chose to reuse sharp-runtime's real BCL ObservableCollection instead of
// porting this MonoGame.Extended-specific one -- that decision still stands for general use.
// This type is ported IN ADDITION, at the project owner's explicit request, for users who
// specifically want MonoGame.Extended's own ObservableCollection/IObservableCollection surface
// (four separate events -- ItemAdded/ItemRemoved/Clearing/Cleared -- rather than sharp-runtime's
// single CollectionChanged event with a change-description payload). The two types are not
// related and are not interchangeable; pick whichever event shape a given call site needs.
//
// `Collection<T>` (the C# base class, System.Collections.ObjectModel.Collection<T>) ->
// sharp-runtime's own `System::Collections::ObjectModel::Collection<T>`, which already exposes
// the exact same protected virtual hooks this type overrides (InsertItem/RemoveItem/ClearItems/
// SetItem), per this project's "reuse sharp-runtime, don't re-roll" convention.
//
// `event EventHandler<ItemEventArgs<T>> ItemAdded/ItemRemoved` / `event EventHandler Clearing/
// Cleared` -> public `System::EventHandler<...>` members + `getXEvent()` accessors, matching
// `FramesPerSecondCounter.hpp`'s established pattern for exposing events on a type deriving
// `System::Object`. `System::EventHandler<TEventArgs>::Raise` requires a `System::Object*`
// sender (see EventHandler.hpp's own doc comment on why this C++ port bundles subscriber storage
// into the handler type itself); `Collection<T>` alone has no `Object` in its inheritance chain,
// so this type additionally inherits `System::Object` (no diamond: `Collection<T>`'s own base,
// `Generic::IList<T>` -> `ICollection<T>` -> `Generic::IEnumerable<T>` -> `Collections::
// IEnumerable`, never touches `Object`) purely to have a valid `this` to pass as sender.
// `GetTypeName()` returns a fixed, non-templated string (`"MonoGame.Extended.Collections.
// ObservableCollection"`) rather than attempting to embed `T`'s name -- matching this project's
// established "reflection-hazard, use a fixed sensible string" precedent (see GameTimer's
// constructor-supplied name), since C++ has no clean, demangling-free way to reproduce C#'s
// `GetType().Name` for an arbitrary template instantiation, and upstream's own reflection-based
// name (e.g. "ObservableCollection\`1[System.Int32]") isn't something meaningfully portable
// anyway.
//
// `IList<T> list` constructor overload -> takes a `const std::vector<T>&` to seed initial items
// (matching `Collection<T>`'s own `std::vector<T>`-backed storage; `Collection(IList<T>)`'s
// upstream null-check has no C++ equivalent need since a `const std::vector<T>&` can't be null).
//
// Header-only: a template, matching this project's established convention for generic C# types
// (see Interval<T>/KeyedCollection<TKey,TValue>).
#pragma once

#include "CNA/Extended/Collections/IObservableCollection.hpp"
#include "CNA/Extended/Collections/ItemEventArgs.hpp"
#include "System/EventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/Collections/ObjectModel/Collection.hpp"
#include "System/Object.hpp"

#include <string>
#include <vector>

namespace CNA::Extended::Collections
{
    /**
     * @brief A Collection<T> that raises ItemAdded/ItemRemoved/Clearing/Cleared events when its
     * contents change.
     * @tparam T The type of elements in the collection.
     */
    template <typename T>
    class ObservableCollection : public System::Collections::ObjectModel::Collection<T>,
                                  public System::Object,
                                  public IObservableCollection<T>
    {
    public:
        /** @brief Initializes a new, empty ObservableCollection. */
        ObservableCollection() = default;

        /** @brief Initializes a new ObservableCollection wrapping (copying) the specified initial items. */
        explicit ObservableCollection(const std::vector<T>& list)
        {
            for (const T& item : list)
            {
                this->items_.push_back(item);
            }
        }

        /** @brief Raised when an item has been added to the collection. */
        System::EventHandler<ItemEventArgs<T>> ItemAdded;

        /** @brief Raised when an item is removed from the collection. */
        System::EventHandler<ItemEventArgs<T>> ItemRemoved;

        /**
         * @brief Raised when the collection is about to be cleared.
         *
         * This could be covered by calling ItemRemoved for each item currently contained in the
         * collection, but it is often simpler and more efficient to process the clearing of the
         * entire collection as a special operation.
         */
        System::EventHandler<System::EventArgs> Clearing;

        /** @brief Raised when the collection has been cleared. */
        System::EventHandler<System::EventArgs> Cleared;

        /** @brief Gets a reference to the ItemAdded event. */
        [[nodiscard]] System::EventHandler<ItemEventArgs<T>>& getItemAddedEvent() override { return ItemAdded; }

        /** @brief Gets a reference to the ItemRemoved event. */
        [[nodiscard]] System::EventHandler<ItemEventArgs<T>>& getItemRemovedEvent() override { return ItemRemoved; }

        /** @brief Gets a reference to the Clearing event. */
        [[nodiscard]] System::EventHandler<System::EventArgs>& getClearingEvent() override { return Clearing; }

        /** @brief Gets a reference to the Cleared event. */
        [[nodiscard]] System::EventHandler<System::EventArgs>& getClearedEvent() override { return Cleared; }

        /** @brief Returns the fully qualified .NET type name of this object. */
        [[nodiscard]] const std::string& GetTypeName() const override
        {
            static const std::string typeName = "MonoGame.Extended.Collections.ObservableCollection";
            return typeName;
        }

    protected:
        /** @brief Removes all elements from the collection. */
        void ClearItems() override
        {
            OnClearing();
            System::Collections::ObjectModel::Collection<T>::ClearItems();
            OnCleared();
        }

        /** @brief Inserts an element at the specified index. */
        void InsertItem(SharpRuntime::intcs index, const T& item) override
        {
            System::Collections::ObjectModel::Collection<T>::InsertItem(index, item);
            OnAdded(item);
        }

        /** @brief Removes the element at the specified index. */
        void RemoveItem(SharpRuntime::intcs index) override
        {
            const T item = (*this)[index];
            System::Collections::ObjectModel::Collection<T>::RemoveItem(index);
            OnRemoved(item);
        }

        /** @brief Replaces the element at the specified index. */
        void SetItem(SharpRuntime::intcs index, const T& item) override
        {
            const T oldItem = (*this)[index];
            System::Collections::ObjectModel::Collection<T>::SetItem(index, item);
            OnRemoved(oldItem);
            OnAdded(item);
        }

        /** @brief Fires the ItemAdded event. */
        virtual void OnAdded(const T& item)
        {
            ItemAdded.Raise(this, ItemEventArgs<T>(item));
        }

        /** @brief Fires the ItemRemoved event. */
        virtual void OnRemoved(const T& item)
        {
            ItemRemoved.Raise(this, ItemEventArgs<T>(item));
        }

        /** @brief Fires the Clearing event. */
        virtual void OnClearing()
        {
            Clearing.Raise(this, System::EventArgs::Empty);
        }

        /** @brief Fires the Cleared event. */
        virtual void OnCleared()
        {
            Cleared.Raise(this, System::EventArgs::Empty);
        }
    };
}
