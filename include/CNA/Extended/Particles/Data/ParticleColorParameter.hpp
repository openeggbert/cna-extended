// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Data/ParticleColorParameter.cs. Uses Vector3 to
// represent color values in the HSL color space, matching upstream. `Value` is computed on each
// access (matching upstream's C# property getter, not cached).
#pragma once

#include "CNA/Extended/FastRandom.hpp"
#include "CNA/Extended/Particles/Data/ParticleValueKind.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/HashCode.hpp"

namespace CNA::Extended::Particles::Data
{
    using Microsoft::Xna::Framework::Vector3;

    /** @brief A color parameter (HSL, via Vector3) that is either a constant value or randomly generated within a range. */
    struct ParticleColorParameter
    {
        ParticleValueKind Kind = ParticleValueKind::Constant;
        Vector3 Constant;
        Vector3 RandomMin;
        Vector3 RandomMax;

        ParticleColorParameter() = default;

        /** @brief Creates a constant color parameter. */
        explicit ParticleColorParameter(const Vector3& value);

        /** @brief Creates a random-range color parameter. */
        ParticleColorParameter(const Vector3& rangeStart, const Vector3& rangeEnd);

        /** @brief Gets the current value: Constant if Kind is Constant, otherwise a fresh random sample in [RandomMin, RandomMax]. */
        [[nodiscard]] Vector3 getValueProperty() const;

        [[nodiscard]] bool Equals(const ParticleColorParameter& other) const;
        [[nodiscard]] int GetHashCode() const;

        friend bool operator==(const ParticleColorParameter& left, const ParticleColorParameter& right) { return left.Equals(right); }
        friend bool operator!=(const ParticleColorParameter& left, const ParticleColorParameter& right) { return !left.Equals(right); }
    };
}
