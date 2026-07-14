// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ModifierExecutionStrategy3DEXT -- new, non-upstream addition.
// See 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::ModifierExecutionStrategy, scoped
// down deliberately (2026-07-14, per the approved parity plan): only the Serial
// (single-threaded) strategy exists here, not upstream's Parallel one. 2D's
// ParallelModifierExecutionStrategy is a documented, deliberately-preserved *upstream* data
// race (every modifier task shares one ParticleIterator with no synchronization -- see that
// class's own header comment) kept faithfully as a 1:1 port of a known upstream quirk. Adding
// an equivalent to genuinely-new (non-upstream) 3D code would mean either reproducing that
// same real data race on purpose, or independently designing new synchronization upstream
// never had -- neither is "start with the simplest correct version." Serial-only for now;
// revisit if a real large-particle-count performance need appears.
//
// getSerialProperty() is a function-local static Meyer's singleton (matching
// ModifierExecutionStrategy.hpp's own established precedent for avoiding static
// initialization order issues).
#pragma once

#include <vector>

namespace CNA::Extended::World3DEXT
{
    class Modifier3DEXT;
    struct Particle3DEXT;

    /** @brief Strategy for executing a list of Modifier3DEXT instances over a particle list. */
    class ModifierExecutionStrategy3DEXT
    {
    public:
        virtual ~ModifierExecutionStrategy3DEXT() = default;

        /** @brief Gets the singleton serial (single-threaded) execution strategy. */
        [[nodiscard]] static ModifierExecutionStrategy3DEXT& getSerialProperty();

        /** @brief Executes InternalUpdate() on every modifier in @p modifiers, per this strategy's execution model. */
        virtual void ExecuteModifiers(const std::vector<Modifier3DEXT*>& modifiers, float elapsedSeconds, std::vector<Particle3DEXT>& particles) = 0;
    };
}
