// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/LayerPair3DEXT.hpp"

#include "System/HashCode.hpp"

namespace CNA::Extended::World3DEXT
{
    LayerPair3DEXT::LayerPair3DEXT(const Layer3DEXT& first, const Layer3DEXT& second)
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

    std::size_t LayerPair3DEXT::GetHashCode() const
    {
        return static_cast<std::size_t>(System::HashCode::Combine(reinterpret_cast<std::uintptr_t>(First), reinterpret_cast<std::uintptr_t>(Second)));
    }
}
