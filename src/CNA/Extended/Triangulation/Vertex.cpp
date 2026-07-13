// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Triangulation/Vertex.hpp"

#include <functional>

namespace CNA::Extended::Triangulation
{
    Vertex::Vertex(const Vector2& position, const int index) : Position(position), Index(index)
    {
    }

    bool Vertex::Equals(const Vertex& other) const
    {
        return other.Position.Equals(Position) && other.Index == Index;
    }

    int Vertex::GetHashCode() const
    {
        // Matches upstream's `unchecked { (Position.GetHashCode() * 397) ^ Index; }`.
        return (Position.GetHashCode() * 397) ^ Index;
    }

    std::string Vertex::ToString() const
    {
        return Position.ToString() + " (" + std::to_string(Index) + ")";
    }
}
