// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Profiles/SprayProfile.cs.
#pragma once

#include "CNA/Extended/Particles/Profiles/Profile.hpp"

namespace CNA::Extended::Particles::Profiles
{
    /** @brief Emits particles from a single point in a directional cone (central Direction +/- Spread/2 radians). */
    class SprayProfile final : public Profile
    {
    public:
        Vector2 Direction;
        float Spread = 0.0f;

        void GetOffsetAndHeading(Vector2* offset, Vector2* heading) override;
        [[nodiscard]] std::string ToString() const override { return "SprayProfile"; }
    };
}
