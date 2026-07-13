// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collections/ObjectPool.cs: an object pool for IPoolable-
// implementing types, tracking free items in a circular-buffer Deque (O(1) operations) and
// in-use items in an intrusive doubly-linked list (via IPoolable::NextNode/PreviousNode), so the
// whole in-use set can be iterated without a separate allocation.
//
// C#'s `where T : class, IPoolable` reference-type + interface constraint maps to `T` denoting
// the pooled class itself (must derive from IPoolable, enforced via static_assert), with the
// pool working in T* throughout -- matching how ObjectPool<T> in upstream already always holds
// references, and how Pool<T>/GameComponentCollectionExtensions.hpp translate C# reference-type
// generic parameters elsewhere in this project. `Func<T>` -> `std::function<T*()>`.
// `event Action<T> ItemUsed`/`ItemReturned` -> `System::MulticastAction<T*>` (a true C# multicast
// event, unlike EventHandler<TEventArgs>'s (sender, args) shape -- MulticastAction<Args...> was
// added to sharp-runtime earlier in this project specifically for this kind of need).
//
// Everywhere upstream does an unchecked `(T)node.NextNode`/`(T)item` cast (trusting that only
// T-implementing instances are ever linked into a given ObjectPool<T>'s own list), this uses
// `static_cast<T*>` -- well-defined given that same trusted invariant, matching upstream's own
// lack of a runtime type check at these points.
//
// *** SEVERE UPSTREAM BUG, PRESERVED FOR FIDELITY -- causes a genuine infinite loop, not just a
// wrong value; confirmed by hand-tracing the exact C# source byte-for-byte, not assumed ***
// `CreateObject()` unconditionally sets `_tailNode = item` as its LAST step, before returning to
// `New()`, which then immediately calls `Use(item)` on that same item. `Use()`'s very first
// structural decision is `if (_tailNode is null)` -- intended to detect "is this the pool's very
// first-ever node" and, if so, leave its PreviousNode/NextNode as null (a proper list head/tail).
// But because `CreateObject()` already set `_tailNode = item` moments earlier, this check is
// FALSE for every freshly-created item, without exception -- the "is this the first node" branch
// is unreachable dead code. `Use()` instead always takes the `else` branch, which sets
// `item->PreviousNode = _tailNode` and `_tailNode->NextNode = item` -- but since `_tailNode` IS
// `item` at this point, this makes the node point to ITSELF: `item.PreviousNode == item` and
// `item.NextNode == item`.
// For a pool where this is the very first (and, so far, only) item ever created, this means
// `headNode_ == tailNode_ == item`, with `item->NextNode == item` (a self-loop). `GetEnumerator`
// (`while (node != null) { yield return node; node = node.NextNode; }`) then NEVER terminates,
// because `node.NextNode` is `node` itself -- an infinite loop on the single most basic usage
// pattern of this class (create one pooled object, then enumerate the pool). Confirmed by hand-
// tracing this exact single-item scenario line-by-line against the C# source (not assumed).
// More generally: since `_tailNode` is always exactly the item just passed to `Use()` at the
// moment of that call, EVERY freshly-created item's NextNode briefly self-references, and it is
// only "fixed" retroactively -- either because a later `CreateObject()` call points the *former*
// tail's NextNode at the newly-created item instead (see `CreateObject()`'s
// `if (_tailNode != null) _tailNode.NextNode = item;`), or because `Return()` unconditionally
// resets the CURRENT tail's NextNode to null as its last bookkeeping step
// (`if (_tailNode != null) _tailNode.NextNode = null;`). So enumerating a pool where every item
// was obtained purely via `New()` and NONE has ever been `Return()`ed yet will always hang on the
// current tail node, not just in the single-item case -- this port only directly proves and
// tests the single-item case below (the simplest, most certainly-reproducible instance), rather
// than claiming to have exhaustively verified every possible sequence of New()/Return() calls.
// Reproduced exactly below via a faithful line-by-line translation, NOT fixed -- see
// `ObjectPoolTests.cpp`'s regression tests, which assert the self-referencing pointers directly
// via `getNextNodeProperty()`/`getPreviousNodeProperty()` rather than by iterating the pool
// (iterating in that exact scenario would hang the test process forever, since it is the bug
// itself).
//
// Header-only: a template, matching this project's established convention for generic C# types.
#pragma once

#include "CNA/Extended/Collections/Deque.hpp"
#include "CNA/Extended/Collections/IPoolable.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/MulticastAction.hpp"

#include <functional>
#include <stdexcept>
#include <type_traits>

namespace CNA::Extended::Collections
{
    using SharpRuntime::intcs;

    /** @brief Governs ObjectPool<T>::New()'s behavior once TotalCount exceeds Capacity. */
    enum class ObjectPoolIsFullPolicy
    {
        /** @brief New() returns nullptr. */
        ReturnNull,
        /** @brief Capacity is increased by one and a new instance is created. */
        IncreaseSize,
        /** @brief The oldest in-use instance (the head of the intrusive list) is force-returned and reused. */
        KillExisting,
    };

    /**
     * @brief A pool of IPoolable-implementing instances, tracking free items in O(1) and
     * in-use items in an iterable intrusive linked list.
     * @tparam T The pooled type; must derive from IPoolable.
     */
    template <typename T>
    class ObjectPool
    {
        static_assert(std::is_base_of_v<IPoolable, T>, "T must implement IPoolable");

    public:
        /** @brief Raised when an item is taken out of the pool via New(). */
        System::MulticastAction<T*> ItemUsed;

        /** @brief Raised when an item is returned to the pool. */
        System::MulticastAction<T*> ItemReturned;

        /**
         * @brief Initializes a new ObjectPool.
         * @param instantiationFunction Factory function used to create new instances.
         * @param capacity The initial capacity; also the threshold at which IsFullPolicy applies.
         * @param isFullPolicy The policy applied once TotalCount exceeds Capacity.
         */
        explicit ObjectPool(std::function<T*()> instantiationFunction, const intcs capacity = 16,
            const ObjectPoolIsFullPolicy isFullPolicy = ObjectPoolIsFullPolicy::ReturnNull)
            : freeItems_(capacity),
              instantiationFunction_(std::move(instantiationFunction)),
              isFullPolicy_(isFullPolicy),
              capacity_(capacity)
        {
            returnToPoolDelegate_ = [this](IPoolable* item) { Return(item); };
        }

        [[nodiscard]] ObjectPoolIsFullPolicy getIsFullPolicyProperty() const { return isFullPolicy_; }
        [[nodiscard]] intcs getCapacityProperty() const { return capacity_; }
        [[nodiscard]] intcs getTotalCountProperty() const { return totalCount_; }
        [[nodiscard]] intcs getAvailableCountProperty() const { return freeItems_.getCountProperty(); }
        [[nodiscard]] intcs getInUseCountProperty() const { return totalCount_ - getAvailableCountProperty(); }

        /**
         * @brief Obtains an instance from the pool: a free one if available, otherwise a newly
         * created one (subject to IsFullPolicy once TotalCount exceeds Capacity).
         * @return The obtained instance, or nullptr per ObjectPoolIsFullPolicy::ReturnNull.
         */
        T* New()
        {
            T* poolable = nullptr;
            if (!freeItems_.RemoveFromFront(poolable))
            {
                if (totalCount_ <= capacity_)
                {
                    poolable = CreateObject();
                }
                else
                {
                    switch (isFullPolicy_)
                    {
                        case ObjectPoolIsFullPolicy::ReturnNull:
                            return nullptr;
                        case ObjectPoolIsFullPolicy::IncreaseSize:
                            capacity_++;
                            poolable = CreateObject();
                            break;
                        case ObjectPoolIsFullPolicy::KillExisting:
                            if (headNode_ == nullptr)
                            {
                                return nullptr;
                            }
                            {
                                T* newHeadNode = static_cast<T*>(headNode_->getNextNodeProperty());
                                headNode_->Return();
                                freeItems_.RemoveFromBack(poolable);
                                headNode_ = newHeadNode;
                            }
                            break;
                        default:
                            throw std::logic_error("ObjectPool<T>::New: unrecognized ObjectPoolIsFullPolicy");
                    }
                }
            }

            Use(poolable);
            return poolable;
        }

        /**
         * @brief A forward iterator walking the currently in-use instances, oldest (head) to
         * newest (tail), following the intrusive linked list.
         */
        class Iterator
        {
        public:
            explicit Iterator(T* node) : node_(node) {}

            T* operator*() const { return node_; }
            Iterator& operator++()
            {
                node_ = static_cast<T*>(node_->getNextNodeProperty());
                return *this;
            }
            bool operator!=(const Iterator& other) const { return node_ != other.node_; }
            bool operator==(const Iterator& other) const { return node_ == other.node_; }

        private:
            T* node_;
        };

        [[nodiscard]] Iterator begin() const { return Iterator(headNode_); }
        [[nodiscard]] Iterator end() const { return Iterator(nullptr); }

    private:
        T* CreateObject()
        {
            totalCount_++;
            T* item = instantiationFunction_();
            if (item == nullptr)
            {
                throw std::runtime_error("The created pooled object is null.");
            }
            item->setPreviousNodeProperty(tailNode_);
            item->setNextNodeProperty(nullptr);
            if (headNode_ == nullptr)
            {
                headNode_ = item;
            }
            if (tailNode_ != nullptr)
            {
                tailNode_->setNextNodeProperty(item);
            }
            tailNode_ = item;
            return item;
        }

        void Return(IPoolable* item)
        {
            T* poolable = static_cast<T*>(item);

            T* previousNode = static_cast<T*>(item->getPreviousNodeProperty());
            T* nextNode = static_cast<T*>(item->getNextNodeProperty());

            if (previousNode != nullptr)
            {
                previousNode->setNextNodeProperty(nextNode);
            }
            if (nextNode != nullptr)
            {
                nextNode->setPreviousNodeProperty(previousNode);
            }

            if (item == headNode_)
            {
                headNode_ = nextNode;
            }
            if (item == tailNode_)
            {
                tailNode_ = previousNode;
            }

            if (tailNode_ != nullptr)
            {
                tailNode_->setNextNodeProperty(nullptr);
            }

            freeItems_.AddToBack(poolable);

            ItemReturned(poolable);
        }

        void Use(T* item)
        {
            item->Initialize(returnToPoolDelegate_);
            item->setNextNodeProperty(nullptr);
            // See this file's header comment (SEVERE UPSTREAM BUG): CreateObject() always sets
            // tailNode_ = item as its last step before calling Use(item) on that same item, so
            // this check is FALSE for every freshly-created item without exception -- the
            // "is this the pool's first node" branch below is unreachable dead code in that
            // path. The else branch then links item to itself (PreviousNode == item,
            // NextNode == item), which causes GetEnumerator() to loop forever once this is the
            // current tail. Preserved exactly, not fixed.
            if (tailNode_ == nullptr)
            {
                headNode_ = item;
                tailNode_ = item;
                item->setPreviousNodeProperty(nullptr);
            }
            else
            {
                item->setPreviousNodeProperty(tailNode_);
                tailNode_->setNextNodeProperty(item);
                tailNode_ = item;
            }

            ItemUsed(item);
        }

        std::function<void(IPoolable*)> returnToPoolDelegate_;
        Deque<T*> freeItems_;
        T* headNode_ = nullptr;
        T* tailNode_ = nullptr;
        std::function<T*()> instantiationFunction_;
        ObjectPoolIsFullPolicy isFullPolicy_;
        intcs capacity_;
        intcs totalCount_ = 0;
    };
}
