// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Size.hpp"

#include "CNA/Extended/SizeF.hpp"

namespace CNA::Extended
{
    const Size Size::Empty = Size();

    Size::Size(const intcs width, const intcs height) : Width(width), Height(height)
    {
    }

    Size Size::Add(const Size& first, const Size& second)
    {
        return Size(first.Width + second.Width, first.Height + second.Height);
    }

    Size Size::Subtract(const Size& first, const Size& second)
    {
        return Size(first.Width - second.Width, first.Height - second.Height);
    }

    Size::Size(const SizeF& size) : Width(static_cast<intcs>(size.Width)), Height(static_cast<intcs>(size.Height))
    {
    }

    int Size::GetHashCode() const
    {
        // unchecked { (Width.GetHashCode() * 397) ^ Height.GetHashCode() }
        return (static_cast<int>(Width) * 397) ^ static_cast<int>(Height);
    }

    std::string Size::ToString() const
    {
        return "Width: " + std::to_string(Width) + ", Height: " + std::to_string(Height);
    }
}
