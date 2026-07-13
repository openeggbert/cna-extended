// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Data/ParticleFloatParameter.hpp"

namespace CNA::Extended::Particles::Data
{
    ParticleFloatParameter::ParticleFloatParameter(float value)
        : Kind(ParticleValueKind::Constant), Constant(value), RandomMin(0.0f), RandomMax(0.0f)
    {
    }

    ParticleFloatParameter::ParticleFloatParameter(float rangeStart, float rangeEnd)
        : Kind(ParticleValueKind::Random), Constant(0.0f), RandomMin(rangeStart), RandomMax(rangeEnd)
    {
    }

    float ParticleFloatParameter::getValueProperty() const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return Constant;
        }

        return FastRandom::getSharedProperty().NextSingle(RandomMin, RandomMax);
    }

    bool ParticleFloatParameter::Equals(const ParticleFloatParameter& other) const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return Constant == other.Constant;
        }

        return RandomMin == other.RandomMin && RandomMax == other.RandomMax;
    }

    int ParticleFloatParameter::GetHashCode() const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return static_cast<int>(std::hash<float>{}(Constant));
        }

        return System::HashCode::Combine(RandomMin, RandomMax);
    }

    std::string ParticleFloatParameter::ToString() const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return std::to_string(Constant);
        }

        return std::to_string(RandomMin) + "," + std::to_string(RandomMax);
    }
}
