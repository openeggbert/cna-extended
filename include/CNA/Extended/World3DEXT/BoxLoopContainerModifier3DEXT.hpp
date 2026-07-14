// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::BoxLoopContainerModifier3DEXT -- new, non-upstream addition.
// See 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::Containers::
// RectangleLoopContainerModifier (Phase 11 C-3, 2026-07-14) -- a box (not a rectangle)
// centered at each particle's TriggeredPositionEXT. Unlike BoxContainerModifier3DEXT, 2D's
// loop variant already truncates consistently on both sides for both X and Y, so the new Z
// axis follows that same consistent pattern with no invented asymmetry needed.
#pragma once

#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Wraps particles to the opposite face when they cross a box centered at each particle's trigger position. */
    class BoxLoopContainerModifier3DEXT final : public Modifier3DEXT
    {
    public:
        int WidthEXT = 0;
        int HeightEXT = 0;
        int DepthEXT = 0;

        BoxLoopContainerModifier3DEXT() : Modifier3DEXT("BoxLoopContainerModifier3DEXT") {}

    protected:
        void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override;
    };
}
