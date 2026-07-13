// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"

#include "CNA/Extended/Particles/ParticleIterator.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended::Particles::Modifiers
{
    Modifier::Modifier(std::string name) : name_(std::move(name))
    {
        setFrequencyProperty(DefaultModifierFrequency);
    }

    void Modifier::setFrequencyProperty(float value)
    {
        System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(value, "value");

        frequency_ = value;
        cycleTime_ = 1.0f / frequency_;
    }

    void Modifier::InternalUpdate(float elapsedSeconds, ParticleIterator& iterator)
    {
        if (!enabled_ || iterator.getTotalProperty() == 0)
        {
            return;
        }

        const int particlesRemaining = iterator.getTotalProperty() - particlesUpdatedThisCycle_;
        const int particlesToUpdate = std::min(
            particlesRemaining, static_cast<int>(std::ceil((elapsedSeconds / cycleTime_) * static_cast<float>(iterator.getTotalProperty()))));

        if (particlesToUpdate > 0)
        {
            ParticleIterator& offsetIterator = iterator.Reset(particlesUpdatedThisCycle_);

            Update(cycleTime_, offsetIterator, particlesToUpdate);

            particlesUpdatedThisCycle_ += particlesToUpdate;
        }

        if (particlesUpdatedThisCycle_ >= iterator.getTotalProperty())
        {
            particlesUpdatedThisCycle_ = 0;
        }
    }
}
