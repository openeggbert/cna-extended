// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Triangulation/Triangle.cs. Upstream's own file header
// additionally credits nickgravelyn's Triangulator project
// (https://github.com/nickgravelyn/Triangulator, an ear-clipping triangulation implementation)
// as the origin of this code ("MIT Licensed"). Carried forward here for the same reason
// MonoGame.Extended carries it: this code traces back further than Craftwork Games. See
// NOTICE.md for a provenance note about this specific attribution.
//
// Upstream's A/B/C fields are `readonly`; ported as plain public mutable fields, matching this
// project's established convention elsewhere (see Vertex.hpp).
#pragma once

#include "CNA/Extended/Triangulation/Vertex.hpp"

#include <string>

namespace CNA::Extended::Triangulation
{
    /** @brief A basic triangle structure that holds the three vertices that make up a given triangle. */
    struct Triangle
    {
        /** @brief The first vertex of this Triangle. */
        Vertex A;

        /** @brief The second vertex of this Triangle. */
        Vertex B;

        /** @brief The third vertex of this Triangle. */
        Vertex C;

        Triangle() = default;

        /** @brief Initializes a new Triangle from the specified three vertices. */
        Triangle(const Vertex& a, const Vertex& b, const Vertex& c);

        /** @brief Determines whether this Triangle contains the specified point, using the even-odd rule. */
        [[nodiscard]] bool ContainsPoint(const Vertex& point) const;

        /** @brief Determines whether the triangle formed by (a, b, c) contains the specified point. */
        [[nodiscard]] static bool ContainsPoint(const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& point);

        [[nodiscard]] bool Equals(const Triangle& other) const;
        [[nodiscard]] int GetHashCode() const;

        friend bool operator==(const Triangle& first, const Triangle& second) { return first.Equals(second); }
        friend bool operator!=(const Triangle& first, const Triangle& second) { return !first.Equals(second); }

    private:
        [[nodiscard]] static bool checkPointToSegment(const Vertex& sA, const Vertex& sB, const Vertex& point);
    };
}
