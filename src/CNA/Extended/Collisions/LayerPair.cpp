// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collisions/LayerPair.hpp"

#include "System/HashCode.hpp"

namespace CNA::Extended::Collisions
{
    LayerPair::LayerPair(const Layer& first, const Layer& second)
    {
        if (&first == &second || &first <= &second)
        {
            First = &first;
            Second = &second;
        }
        else
        {
            First = &second;
            Second = &first;
        }
    }

    std::size_t LayerPair::GetHashCode() const
    {
        return static_cast<std::size_t>(System::HashCode::Combine(reinterpret_cast<std::uintptr_t>(First), reinterpret_cast<std::uintptr_t>(Second)));
    }
}
