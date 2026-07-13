// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's FloatHelper.cs. Header-only: the whole type is one trivial
// method. C#'s [MethodImpl(MethodImplOptions.AggressiveInlining)] hint has no direct portable
// C++ attribute equivalent; `inline` (plus the definition living in the header) is the closest
// portable match.
#pragma once

namespace CNA::Extended
{
    class FloatHelper
    {
    public:
        FloatHelper() = delete;

        /** @brief Swaps the values of two floats. */
        static inline void Swap(float& value1, float& value2)
        {
            const float temp = value1;
            value1 = value2;
            value2 = temp;
        }
    };
}
