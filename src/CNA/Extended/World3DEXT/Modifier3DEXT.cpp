// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended::World3DEXT
{
    Modifier3DEXT::Modifier3DEXT(std::string name) : name_(std::move(name))
    {
        setFrequencyProperty(DefaultModifierFrequency);
    }

    void Modifier3DEXT::setFrequencyProperty(float value)
    {
        System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(value, "value");

        frequency_ = value;
        cycleTime_ = 1.0f / frequency_;
    }

    void Modifier3DEXT::InternalUpdate(float elapsedSeconds, std::vector<Particle3DEXT>& particles)
    {
        if (!enabled_ || particles.empty())
        {
            return;
        }

        const int total = static_cast<int>(particles.size());
        const int particlesRemaining = total - particlesUpdatedThisCycle_;
        const int particlesToUpdate =
            std::min(particlesRemaining, static_cast<int>(std::ceil((elapsedSeconds / cycleTime_) * static_cast<float>(total))));

        if (particlesToUpdate > 0)
        {
            // See this file's header comment: elapsedSeconds (the real per-call time), not
            // cycleTime_, is what Update() receives -- a deliberate deviation from 2D's own
            // Modifier::InternalUpdate.
            Update(elapsedSeconds, particles, particlesUpdatedThisCycle_, particlesToUpdate);
            particlesUpdatedThisCycle_ += particlesToUpdate;
        }

        if (particlesUpdatedThisCycle_ >= total)
        {
            particlesUpdatedThisCycle_ = 0;
        }
    }
}
