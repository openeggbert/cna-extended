// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/ModifierExecutionStrategy.cs. Upstream's
// static `Serial`/`Parallel` singleton fields -> function-local static Meyer's singletons (matching
// this project's established `FastRandom::getSharedProperty()` precedent), avoiding static
// initialization order fiasco. `List<Modifier>` -> `std::vector<Modifier*>` (non-owning: the
// emitter that owns the actual Modifier instances hands out pointers here, matching upstream's
// reference-type-list aliasing). Upstream's `internal` visibility kept public, matching this
// project's established convention. Its two concrete subclasses (SerialModifierExecutionStrategy,
// ParallelModifierExecutionStrategy) are also upstream-`internal`; they're implemented directly in
// the .cpp rather than given their own header/source pair, since nothing outside this file
// constructs them (only accessed via getSerialProperty()/getParallelProperty()).
#pragma once

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::Particles
{
    class ParticleIterator;
}

namespace CNA::Extended::Particles::Modifiers
{
    class Modifier;

    /** @brief Strategy for executing a list of Modifiers over a particle buffer: serial or parallel. */
    class ModifierExecutionStrategy
    {
    public:
        virtual ~ModifierExecutionStrategy() = default;

        /** @brief Gets the singleton serial (single-threaded) execution strategy. */
        [[nodiscard]] static ModifierExecutionStrategy& getSerialProperty();

        /** @brief Gets the singleton parallel (multi-threaded) execution strategy. */
        [[nodiscard]] static ModifierExecutionStrategy& getParallelProperty();

        /** @brief Executes InternalUpdate() on every modifier in @p modifiers, per this strategy's execution model. */
        virtual void ExecuteModifiers(const std::vector<Modifier*>& modifiers, float elapsedSeconds, ParticleIterator& iterator) = 0;

        [[nodiscard]] virtual std::string ToString() const = 0;
    };
}
