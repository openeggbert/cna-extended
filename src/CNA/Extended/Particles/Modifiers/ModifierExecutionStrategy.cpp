// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/ModifierExecutionStrategy.hpp"

#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"
#include "System/Threading/Tasks/Parallel.hpp"

namespace CNA::Extended::Particles::Modifiers
{
    namespace
    {
        class SerialModifierExecutionStrategy final : public ModifierExecutionStrategy
        {
        public:
            void ExecuteModifiers(const std::vector<Modifier*>& modifiers, float elapsedSeconds, ParticleIterator& iterator) override
            {
                for (Modifier* modifier : modifiers)
                {
                    modifier->InternalUpdate(elapsedSeconds, iterator);
                }
            }

            [[nodiscard]] std::string ToString() const override { return "Serial"; }
        };

        // Upstream's ParallelModifierExecutionStrategy runs every modifier's InternalUpdate() on the
        // *same* shared ParticleIterator concurrently via Parallel.ForEach -- each modifier does call
        // iterator.Reset(offset) on entry (overwriting whatever the previous modifier left), but
        // running that concurrently on one shared mutable iterator is a genuine data race in
        // upstream's own design, not something introduced by this port. Preserved faithfully rather
        // than "fixed" by giving each thread its own iterator, per this project's port-faithfully
        // mandate -- see NEXT.md if this ever causes a real problem in practice.
        class ParallelModifierExecutionStrategy final : public ModifierExecutionStrategy
        {
        public:
            void ExecuteModifiers(const std::vector<Modifier*>& modifiers, float elapsedSeconds, ParticleIterator& iterator) override
            {
                System::Threading::Tasks::Parallel::ForEach<Modifier*>(
                    modifiers, [&iterator, elapsedSeconds](Modifier* modifier) { modifier->InternalUpdate(elapsedSeconds, iterator); });
            }

            [[nodiscard]] std::string ToString() const override { return "Parallel"; }
        };
    }

    ModifierExecutionStrategy& ModifierExecutionStrategy::getSerialProperty()
    {
        static SerialModifierExecutionStrategy instance;
        return instance;
    }

    ModifierExecutionStrategy& ModifierExecutionStrategy::getParallelProperty()
    {
        static ParallelModifierExecutionStrategy instance;
        return instance;
    }
}
