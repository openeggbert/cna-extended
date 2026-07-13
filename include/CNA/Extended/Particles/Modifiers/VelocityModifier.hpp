// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/VelocityModifier.cs. `List<Interpolator>` ->
// owning `std::vector<std::unique_ptr<Interpolators::Interpolator>>`, matching AgeModifier's
// established translation for the same upstream pattern.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Interpolators/Interpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::Particles::Modifiers
{
    /** @brief Applies a collection of Interpolators to particles, with intensity based on velocity magnitude relative to VelocityThreshold. */
    class VelocityModifier : public Modifier
    {
    public:
        float VelocityThreshold = 0.0f;

        VelocityModifier() : Modifier("VelocityModifier") {}

        /** @brief Gets the mutable list of interpolators applied to particles. */
        [[nodiscard]] std::vector<std::unique_ptr<Interpolators::Interpolator>>& getInterpolatorsProperty() { return interpolators_; }

    protected:
        void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) override;

    private:
        std::vector<std::unique_ptr<Interpolators::Interpolator>> interpolators_;
    };
}
