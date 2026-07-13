// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for ParticleEffect. Fresh tests below.
#include "CNA/Extended/Particles/ParticleEffect.hpp"

#include <gtest/gtest.h>
#include <memory>

namespace CNA::Extended::Particles
{
    TEST(ParticleEffectTests, ActiveParticlesSumsAcrossEmitters)
    {
        ParticleEffect subject("TestEffect");

        auto emitter1 = std::make_unique<ParticleEmitter>(100);
        emitter1->getParametersProperty().Quantity = Data::ParticleInt32Parameter(5);
        auto emitter2 = std::make_unique<ParticleEmitter>(100);
        emitter2->getParametersProperty().Quantity = Data::ParticleInt32Parameter(7);

        emitter1->Trigger(Vector2());
        emitter2->Trigger(Vector2());

        subject.getEmittersProperty().push_back(std::move(emitter1));
        subject.getEmittersProperty().push_back(std::move(emitter2));

        EXPECT_EQ(subject.getActiveParticlesProperty(), 12);
    }

    TEST(ParticleEffectTests, TriggerPropagatesToAllEmitters)
    {
        ParticleEffect subject("TestEffect");
        subject.setAutoTriggerProperty(false);

        auto emitter = std::make_unique<ParticleEmitter>(100);
        emitter->getParametersProperty().Quantity = Data::ParticleInt32Parameter(3);
        subject.getEmittersProperty().push_back(std::move(emitter));

        subject.Trigger();
        EXPECT_EQ(subject.getActiveParticlesProperty(), 3);
    }

    TEST(ParticleEffectTests, DisposeDisposesAllEmitters)
    {
        ParticleEffect subject("TestEffect");
        auto emitter = std::make_unique<ParticleEmitter>(10);
        ParticleEmitter* emitterPtr = emitter.get();
        subject.getEmittersProperty().push_back(std::move(emitter));

        subject.Dispose();
        EXPECT_TRUE(emitterPtr->getIsDisposedProperty());
        EXPECT_TRUE(subject.getIsDisposedProperty());
    }

    TEST(ParticleEffectTests, AutoTriggerFiresOnUpdate)
    {
        ParticleEffect subject("TestEffect");
        subject.setAutoTriggerProperty(true);
        subject.setAutoTriggerFrequencyProperty(0.1f);

        auto emitter = std::make_unique<ParticleEmitter>(100);
        emitter->getParametersProperty().Quantity = Data::ParticleInt32Parameter(4);
        // Default LifeSpan is 1.0s; use a small elapsedSeconds well under that so the same Update()
        // call that triggers new particles doesn't also immediately reclaim them as expired.
        subject.getEmittersProperty().push_back(std::move(emitter));

        EXPECT_EQ(subject.getActiveParticlesProperty(), 0);
        subject.Update(0.1f);
        EXPECT_EQ(subject.getActiveParticlesProperty(), 4);
    }
}
