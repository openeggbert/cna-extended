// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/SizeF.hpp"

namespace CNA::Extended
{
    const SizeF SizeF::Empty = SizeF();

    SizeF::SizeF(const float width, const float height) : Width(width), Height(height)
    {
    }

    SizeF SizeF::Add(const SizeF& first, const SizeF& second)
    {
        return SizeF(first.Width + second.Width, first.Height + second.Height);
    }

    SizeF SizeF::Subtract(const SizeF& first, const SizeF& second)
    {
        return SizeF(first.Width - second.Width, first.Height - second.Height);
    }

    Vector2 operator-(const Vector2& first, const SizeF& second)
    {
        return SizeF::Subtract(first, second);
    }

    int SizeF::GetHashCode() const
    {
        // unchecked { (Width.GetHashCode() * 397) ^ Height.GetHashCode() }
        return (static_cast<int>(std::hash<float>{}(Width)) * 397) ^ static_cast<int>(std::hash<float>{}(Height));
    }

    std::string SizeF::ToString() const
    {
        return "Width: " + std::to_string(Width) + ", Height: " + std::to_string(Height);
    }
}
