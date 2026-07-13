// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Data/ParticleVector2Parameter.hpp"

namespace CNA::Extended::Particles::Data
{
    ParticleVector2Parameter::ParticleVector2Parameter(const Vector2& value)
        : Kind(ParticleValueKind::Constant), Constant(value), RandomMin(), RandomMax(), Uniform(false)
    {
    }

    ParticleVector2Parameter::ParticleVector2Parameter(const Vector2& rangeStart, const Vector2& rangeEnd)
        : Kind(ParticleValueKind::Random), Constant(), RandomMin(rangeStart), RandomMax(rangeEnd), Uniform(false)
    {
    }

    Vector2 ParticleVector2Parameter::getValueProperty() const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return Constant;
        }

        if (Uniform)
        {
            const float s = FastRandom::getSharedProperty().NextSingle(RandomMin.X, RandomMax.X);
            return Vector2(s, s);
        }

        Vector2 v;
        v.X = FastRandom::getSharedProperty().NextSingle(RandomMin.X, RandomMax.X);
        v.Y = FastRandom::getSharedProperty().NextSingle(RandomMin.Y, RandomMax.Y);
        return v;
    }

    bool ParticleVector2Parameter::Equals(const ParticleVector2Parameter& other) const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return Constant.Equals(other.Constant);
        }

        return RandomMin.Equals(other.RandomMin) && RandomMax.Equals(other.RandomMax);
    }

    int ParticleVector2Parameter::GetHashCode() const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return Constant.GetHashCode();
        }

        return System::HashCode::Combine(RandomMin.GetHashCode(), RandomMax.GetHashCode());
    }

    std::string ParticleVector2Parameter::ToString() const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return Constant.ToString();
        }

        return RandomMin.ToString() + "," + RandomMax.ToString();
    }
}
