// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collections/IPoolable.cs. C#'s `Action<IPoolable>` maps to
// `std::function<void(IPoolable*)>`, matching this project's established Func/Action-to-
// std::function convention (see HslColor.hpp's Match/Map). NextNode/PreviousNode are typed
// IPoolable* rather than a specific T* -- ObjectPool<T> casts to T* internally wherever upstream
// does `(T)node.NextNode`, matching upstream's own unchecked casts (trusting that only
// T-implementing instances are ever linked into a given ObjectPool<T>'s intrusive list).
#pragma once

#include <functional>

namespace CNA::Extended::Collections
{
    /** @brief Implemented by types usable with ObjectPool<T>: supports an intrusive doubly-linked list plus pool-return notification. */
    class IPoolable
    {
    public:
        virtual ~IPoolable() = default;

        [[nodiscard]] virtual IPoolable* getNextNodeProperty() const = 0;
        virtual void setNextNodeProperty(IPoolable* value) = 0;

        [[nodiscard]] virtual IPoolable* getPreviousNodeProperty() const = 0;
        virtual void setPreviousNodeProperty(IPoolable* value) = 0;

        /** @brief Called by ObjectPool<T> when this instance is taken out of the pool, providing the delegate to call from Return(). */
        virtual void Initialize(std::function<void(IPoolable*)> returnDelegate) = 0;

        /** @brief Returns this instance to the pool it was taken from (by invoking the delegate captured in Initialize). */
        virtual void Return() = 0;
    };
}
