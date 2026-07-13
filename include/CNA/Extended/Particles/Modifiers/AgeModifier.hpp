// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/AgeModifier.cs. `List<Interpolator>` ->
// owning `std::vector<std::unique_ptr<Interpolators::Interpolator>>` (nothing else upstream
// aliases individual Interpolator instances).
#pragma once

#include "CNA/Extended/Particles/Modifiers/Interpolators/Interpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::Particles::Modifiers
{
    /** @brief Applies a collection of Interpolators to each particle, driven by the particle's age (as a fraction of its lifespan). */
    class AgeModifier final : public Modifier
    {
    public:
        AgeModifier() : Modifier("AgeModifier") {}

        /** @brief Gets the mutable list of interpolators applied to particles. */
        [[nodiscard]] std::vector<std::unique_ptr<Interpolators::Interpolator>>& getInterpolatorsProperty() { return interpolators_; }

    protected:
        void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) override;

    private:
        std::vector<std::unique_ptr<Interpolators::Interpolator>> interpolators_;
    };
}
