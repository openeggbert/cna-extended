// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Data/ParticleInt32Parameter.cs.
#pragma once

#include "CNA/Extended/FastRandom.hpp"
#include "CNA/Extended/Particles/Data/ParticleValueKind.hpp"
#include "System/HashCode.hpp"

#include <string>

namespace CNA::Extended::Particles::Data
{
    /** @brief An int parameter that is either a constant value or randomly generated within a range. */
    struct ParticleInt32Parameter
    {
        ParticleValueKind Kind = ParticleValueKind::Constant;
        int Constant = 0;
        int RandomMin = 0;
        int RandomMax = 0;

        ParticleInt32Parameter() = default;

        /** @brief Creates a constant int parameter. */
        explicit ParticleInt32Parameter(int value);

        /** @brief Creates a random-range int parameter. */
        ParticleInt32Parameter(int rangeStart, int rangeEnd);

        /** @brief Gets the current value: Constant if Kind is Constant, otherwise a fresh random sample in [RandomMin, RandomMax]. */
        [[nodiscard]] int getValueProperty() const;

        [[nodiscard]] bool Equals(const ParticleInt32Parameter& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const ParticleInt32Parameter& left, const ParticleInt32Parameter& right) { return left.Equals(right); }
        friend bool operator!=(const ParticleInt32Parameter& left, const ParticleInt32Parameter& right) { return !left.Equals(right); }
    };
}
