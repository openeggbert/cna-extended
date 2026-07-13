// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Profiles/BoxFillProfile.cs.
#pragma once

#include "CNA/Extended/Particles/Profiles/Profile.hpp"

namespace CNA::Extended::Particles::Profiles
{
    /** @brief Emits particles from anywhere within a rectangular area, random headings. */
    class BoxFillProfile final : public Profile
    {
    public:
        float Width = 0.0f;
        float Height = 0.0f;

        void GetOffsetAndHeading(Vector2* offset, Vector2* heading) override;
        [[nodiscard]] std::string ToString() const override { return "BoxFillProfile"; }
    };
}
