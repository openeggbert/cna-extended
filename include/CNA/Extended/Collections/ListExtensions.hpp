// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collections/ListExtensions.cs. C# extension method on
// IList<T> -> a free function template taking std::vector<T>& in this namespace, matching the
// convention used throughout this project (see RandomExtensions.hpp); IList<T>/array parameters
// elsewhere in this project already translate to std::vector<T> (see PrimitivesHelper,
// RectangleF::CreateFrom(points)). Upstream's Shuffle both mutates in place AND returns the same
// list (a fluent/chainable extension method); the C++ translation does the same, returning a
// reference to the same std::vector passed in.
#pragma once

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Random.hpp"

#include <utility>
#include <vector>

namespace CNA::Extended::Collections
{
    /**
     * @brief Shuffles @p list in place using the Fisher-Yates algorithm.
     * @return A reference to @p list (the same instance, now shuffled).
     */
    template <typename T>
    std::vector<T>& Shuffle(std::vector<T>& list, System::Random& random)
    {
        auto n = static_cast<SharpRuntime::intcs>(list.size());
        while (n > 1)
        {
            n--;
            const SharpRuntime::intcs k = random.Next(n + 1);
            std::swap(list[static_cast<std::size_t>(k)], list[static_cast<std::size_t>(n)]);
        }
        return list;
    }
}
