// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Triangulation/Vertex.cs. Upstream's own file header
// additionally credits nickgravelyn's Triangulator project
// (https://github.com/nickgravelyn/Triangulator, an ear-clipping triangulation implementation)
// as the origin of this code ("MIT Licensed"). Carried forward here for the same reason
// MonoGame.Extended carries it: this code traces back further than Craftwork Games. See
// NOTICE.md for a provenance note about this specific attribution.
//
// Upstream's Position/Index fields are `readonly`; ported as plain public mutable fields,
// matching this project's established convention for other Math-folder structs (Segment2,
// CircleF, etc.) where C#'s `readonly` has no used C++ equivalent elsewhere in this codebase.
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended::Triangulation
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief A vertex used during polygon triangulation: a position paired with its original index. */
    struct Vertex
    {
        /** @brief The position of this Vertex. */
        Vector2 Position;

        /** @brief The original index of this Vertex within its source polygon. */
        int Index = 0;

        Vertex() = default;

        /**
         * @brief Initializes a new Vertex from the specified position and index.
         * @param position The position.
         * @param index The original index within the source polygon.
         */
        Vertex(const Vector2& position, int index);

        [[nodiscard]] bool Equals(const Vertex& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const Vertex& first, const Vertex& second) { return first.Equals(second); }
        friend bool operator!=(const Vertex& first, const Vertex& second) { return !first.Equals(second); }
    };
}
