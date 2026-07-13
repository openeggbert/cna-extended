// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Profiles/RingProfile.cs.
#pragma once

#include "CNA/Extended/Particles/Profiles/CircleRadiation.hpp"
#include "CNA/Extended/Particles/Profiles/Profile.hpp"

namespace CNA::Extended::Particles::Profiles
{
    /** @brief Emits particles from the perimeter of a circle, with a configurable radiation pattern for headings. */
    class RingProfile final : public Profile
    {
    public:
        float Radius = 0.0f;
        CircleRadiation Radiate = CircleRadiation::None;

        /** @throws System::ArgumentOutOfRangeException Radiate is not a recognized value. */
        void GetOffsetAndHeading(Vector2* offset, Vector2* heading) override;
        [[nodiscard]] std::string ToString() const override { return "RingProfile"; }
    };
}
