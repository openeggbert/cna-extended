// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ECS/BitArrayExtensions.hpp"

#include <algorithm>

namespace CNA::Extended::ECS
{
    bool IsEmpty(const BitArray& bitArray)
    {
        for (SharpRuntime::intcs i = 0; i < bitArray.getLengthProperty(); ++i)
        {
            if (bitArray[i])
            {
                return false;
            }
        }
        return true;
    }

    bool ContainsAll(const BitArray& bitArray, const BitArray& other)
    {
        const SharpRuntime::intcs otherBitsLength = other.getLengthProperty();
        const SharpRuntime::intcs bitsLength = bitArray.getLengthProperty();

        for (SharpRuntime::intcs i = bitsLength; i < otherBitsLength; ++i)
        {
            if (other[i])
            {
                return false;
            }
        }

        const SharpRuntime::intcs s = std::min(bitsLength, otherBitsLength);

        for (SharpRuntime::intcs i = 0; i < s; ++i)
        {
            if ((bitArray[i] && other[i]) != other[i])
            {
                return false;
            }
        }

        return true;
    }

    bool Intersects(const BitArray& bitArray, const BitArray& other)
    {
        const SharpRuntime::intcs s = std::min(bitArray.getLengthProperty(), other.getLengthProperty());

        for (SharpRuntime::intcs i = 0; i < s; ++i)
        {
            if (bitArray[i] && other[i])
            {
                return true;
            }
        }

        return false;
    }
}
