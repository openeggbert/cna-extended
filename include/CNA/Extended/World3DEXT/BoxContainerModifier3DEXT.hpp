// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::BoxContainerModifier3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::Containers::
// RectangleContainerModifier (Phase 11 C-3, 2026-07-14) -- a box (not a rectangle) centered
// at each particle's TriggeredPositionEXT. X/Y axis checks preserve 2D RectangleContainerModifier's
// exact (documented, upstream) inconsistent int-truncation quirk (left/bottom truncate,
// right/top don't) rather than "fixing" it; the new Z axis has no 2D precedent to preserve, so
// it uses consistent (non-truncating) checks on both sides instead of inventing a new
// asymmetry (see BoxContainerModifier3DEXT.cpp).
#pragma once

#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Constrains particles inside a box centered at each particle's trigger position, reflecting off the faces. */
    class BoxContainerModifier3DEXT final : public Modifier3DEXT
    {
    public:
        int WidthEXT = 0;
        int HeightEXT = 0;
        int DepthEXT = 0;
        float RestitutionCoefficientEXT = 1.0f;

        BoxContainerModifier3DEXT() : Modifier3DEXT("BoxContainerModifier3DEXT") {}

    protected:
        void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override;
    };
}
