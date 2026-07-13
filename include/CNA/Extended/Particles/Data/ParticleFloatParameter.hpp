// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Data/ParticleFloatParameter.cs.
#pragma once

#include "CNA/Extended/FastRandom.hpp"
#include "CNA/Extended/Particles/Data/ParticleValueKind.hpp"
#include "System/HashCode.hpp"

#include <string>

namespace CNA::Extended::Particles::Data
{
    /** @brief A float parameter that is either a constant value or randomly generated within a range. */
    struct ParticleFloatParameter
    {
        ParticleValueKind Kind = ParticleValueKind::Constant;
        float Constant = 0.0f;
        float RandomMin = 0.0f;
        float RandomMax = 0.0f;

        ParticleFloatParameter() = default;

        /** @brief Creates a constant float parameter. */
        explicit ParticleFloatParameter(float value);

        /** @brief Creates a random-range float parameter. */
        ParticleFloatParameter(float rangeStart, float rangeEnd);

        /** @brief Gets the current value: Constant if Kind is Constant, otherwise a fresh random sample in [RandomMin, RandomMax]. */
        [[nodiscard]] float getValueProperty() const;

        [[nodiscard]] bool Equals(const ParticleFloatParameter& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const ParticleFloatParameter& left, const ParticleFloatParameter& right) { return left.Equals(right); }
        friend bool operator!=(const ParticleFloatParameter& left, const ParticleFloatParameter& right) { return !left.Equals(right); }
    };
}
