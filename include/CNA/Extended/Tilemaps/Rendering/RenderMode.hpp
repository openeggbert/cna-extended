// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/RenderMode.cs.
#pragma once

namespace CNA::Extended::Tilemaps::Rendering
{
    /** @brief Specifies how ungrouped layers are rendered. */
    enum class RenderMode
    {
        /**
         * @brief All ungrouped layers are merged into a single draw call.
         * Typically faster for maps with multiple layers due to reduced draw call overhead.
         */
        Merged,

        /**
         * @brief Each ungrouped layer is drawn individually.
         * May be faster for maps with frequently changing tiles, as only modified layers need
         * rebuilding. Use benchmarks to determine best mode for your use case.
         */
        Separate
    };
}
