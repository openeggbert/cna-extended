// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Data/ParticleColorParameter.hpp"

namespace CNA::Extended::Particles::Data
{
    ParticleColorParameter::ParticleColorParameter(const Vector3& value)
        : Kind(ParticleValueKind::Constant), Constant(value), RandomMin(), RandomMax()
    {
    }

    ParticleColorParameter::ParticleColorParameter(const Vector3& rangeStart, const Vector3& rangeEnd)
        : Kind(ParticleValueKind::Random), Constant(), RandomMin(rangeStart), RandomMax(rangeEnd)
    {
    }

    Vector3 ParticleColorParameter::getValueProperty() const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return Constant;
        }

        Vector3 hsl;
        hsl.X = FastRandom::getSharedProperty().NextSingle(RandomMin.X, RandomMax.X);
        hsl.Y = FastRandom::getSharedProperty().NextSingle(RandomMin.Y, RandomMax.Y);
        hsl.Z = FastRandom::getSharedProperty().NextSingle(RandomMin.Z, RandomMax.Z);
        return hsl;
    }

    bool ParticleColorParameter::Equals(const ParticleColorParameter& other) const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return Constant.Equals(other.Constant);
        }

        return RandomMin.Equals(other.RandomMin) && RandomMax.Equals(other.RandomMax);
    }

    int ParticleColorParameter::GetHashCode() const
    {
        if (Kind == ParticleValueKind::Constant)
        {
            return Constant.GetHashCode();
        }

        return System::HashCode::Combine(RandomMin.GetHashCode(), RandomMax.GetHashCode());
    }
}
