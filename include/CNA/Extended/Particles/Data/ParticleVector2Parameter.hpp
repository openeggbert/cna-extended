// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Data/ParticleVector2Parameter.cs.
#pragma once

#include "CNA/Extended/FastRandom.hpp"
#include "CNA/Extended/Particles/Data/ParticleValueKind.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/HashCode.hpp"

#include <string>

namespace CNA::Extended::Particles::Data
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief A Vector2 parameter that is either a constant value or randomly generated within a range. */
    struct ParticleVector2Parameter
    {
        ParticleValueKind Kind = ParticleValueKind::Constant;
        Vector2 Constant;
        Vector2 RandomMin;
        Vector2 RandomMax;

        /**
         * @brief When true and Kind is Random, a single random value is sampled and applied to both X and Y
         * (using RandomMin.X/RandomMax.X), preserving aspect ratio. When false, X and Y are sampled independently.
         */
        bool Uniform = false;

        ParticleVector2Parameter() = default;

        /** @brief Creates a constant Vector2 parameter. */
        explicit ParticleVector2Parameter(const Vector2& value);

        /** @brief Creates a random-range Vector2 parameter. */
        ParticleVector2Parameter(const Vector2& rangeStart, const Vector2& rangeEnd);

        /** @brief Gets the current value per Kind/Uniform -- see the Uniform field for the sampling rules. */
        [[nodiscard]] Vector2 getValueProperty() const;

        [[nodiscard]] bool Equals(const ParticleVector2Parameter& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const ParticleVector2Parameter& left, const ParticleVector2Parameter& right) { return left.Equals(right); }
        friend bool operator!=(const ParticleVector2Parameter& left, const ParticleVector2Parameter& right) { return !left.Equals(right); }
    };
}
