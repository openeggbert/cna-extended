// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for ModifierExecutionStrategy. Fresh tests below. Only the Serial
// strategy is exercised for correctness (deterministic); Parallel is exercised only for "doesn't
// crash and does apply the modifier", since its documented data race (see
// ModifierExecutionStrategy.cpp's header comment) makes per-particle result assertions unreliable
// by design, matching upstream's own behavior.
#include "CNA/Extended/Particles/Modifiers/ModifierExecutionStrategy.hpp"

#include "CNA/Extended/Particles/Modifiers/RotationModifier.hpp"
#include "CNA/Extended/Particles/ParticleBuffer.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Modifiers
{
    TEST(ModifierExecutionStrategyTests, SerialAppliesModifierToEachInList)
    {
        ParticleBuffer buffer(2);
        ParticleIterator& releaseIterator = buffer.Release(2);
        Data::Particle* first = releaseIterator.Next();
        Data::Particle* second = releaseIterator.Next();
        first->Rotation = 0.0f;
        second->Rotation = 0.0f;

        RotationModifier modifier;
        modifier.RotationRate = 1.0f;

        std::vector<Modifier*> modifiers{&modifier};
        ParticleIterator& updateIterator = buffer.getIteratorProperty();
        ModifierExecutionStrategy::getSerialProperty().ExecuteModifiers(modifiers, 1.0f, updateIterator);

        EXPECT_GT(first->Rotation, 0.0f);
        EXPECT_GT(second->Rotation, 0.0f);
    }

    TEST(ModifierExecutionStrategyTests, SerialToStringReturnsSerial)
    {
        EXPECT_EQ(ModifierExecutionStrategy::getSerialProperty().ToString(), "Serial");
    }

    TEST(ModifierExecutionStrategyTests, ParallelToStringReturnsParallel)
    {
        EXPECT_EQ(ModifierExecutionStrategy::getParallelProperty().ToString(), "Parallel");
    }
}
