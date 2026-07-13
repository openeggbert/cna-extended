// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Profiles/BoxProfile.cs.
#pragma once

#include "CNA/Extended/Particles/Profiles/Profile.hpp"

namespace CNA::Extended::Particles::Profiles
{
    /** @brief Emits particles from one of the four edges of a rectangle (equal probability per side), random headings. */
    class BoxProfile final : public Profile
    {
    public:
        float Width = 0.0f;
        float Height = 0.0f;

        void GetOffsetAndHeading(Vector2* offset, Vector2* heading) override;
        [[nodiscard]] std::string ToString() const override { return "BoxProfile"; }
    };
}
