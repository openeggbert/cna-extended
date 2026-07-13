// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Profiles/LineProfile.cs.
#pragma once

#include "CNA/Extended/Particles/Profiles/LineRadiation.hpp"
#include "CNA/Extended/Particles/Profiles/Profile.hpp"

namespace CNA::Extended::Particles::Profiles
{
    /** @brief Emits particles uniformly along a line segment, with a configurable radiation pattern for headings. */
    class LineProfile final : public Profile
    {
    public:
        Vector2 Axis;
        float Length = 0.0f;

        /** @brief Heading for Directional mode, or normal-direction scale factor for PerpendicularUp/Down. Ignored for None. */
        Vector2 Direction = Vector2::UnitY;

        LineRadiation Radiate = LineRadiation::None;

        /** @throws System::InvalidOperationException Radiate is not a recognized value. */
        void GetOffsetAndHeading(Vector2* offset, Vector2* heading) override;
        [[nodiscard]] std::string ToString() const override { return "LineProfile"; }
    };
}
