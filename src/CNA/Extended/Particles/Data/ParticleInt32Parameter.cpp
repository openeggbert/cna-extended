// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Data/ParticleInt32Parameter.hpp"

namespace CNA::Extended::Particles::Data
{
    ParticleInt32Parameter::ParticleInt32Parameter(int value)
        : Kind(ParticleValueKind::Constant), Constant(value), RandomMin(0), RandomMax(0)
    {
    }

    ParticleInt32Parameter::ParticleInt32Parameter(int rangeStart, int rangeEnd)
        : Kind(ParticleValueKind::Random), Constant(0), RandomMin(rangeStart), RandomMax(rangeEnd)
    {
    }

    int ParticleInt32Parameter::getValueProperty() const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return Constant;
        }

        return FastRandom::getSharedProperty().Next(RandomMin, RandomMax);
    }

    bool ParticleInt32Parameter::Equals(const ParticleInt32Parameter& other) const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return Constant == other.Constant;
        }

        return RandomMin == other.RandomMin && RandomMax == other.RandomMax;
    }

    int ParticleInt32Parameter::GetHashCode() const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return static_cast<int>(std::hash<int>{}(Constant));
        }

        return System::HashCode::Combine(RandomMin, RandomMax);
    }

    std::string ParticleInt32Parameter::ToString() const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return std::to_string(Constant);
        }

        return std::to_string(RandomMin) + "," + std::to_string(RandomMax);
    }
}
