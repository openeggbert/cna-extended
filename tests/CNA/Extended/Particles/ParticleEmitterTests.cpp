// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Adapted from MonoGame.Extended's tests/MonoGame.Extended.Tests/Particles/EmitterTests.cs.
// Upstream's own file is entirely commented out and uses a ParticleEmitter constructor
// (name, capacity, TimeSpan, Profile) that no longer exists on the current ParticleEmitter.cs at
// all -- confirming the file predates the class's current shape and cannot be ported literally.
// The underlying behaviors it describes (active-particle-count tracking on trigger/expiry, release
// headroom clamping, dispose idempotency) are still real and testable against the current API, so
// they're re-created here as fresh tests rather than silently dropped.
#include "CNA/Extended/Particles/ParticleEmitter.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Particles
{
    TEST(ParticleEmitterTests, TriggerIncreasesActiveParticlesByQuantity)
    {
        ParticleEmitter subject(100);
        subject.setLifeSpanProperty(1.0f);
        subject.getParametersProperty().Quantity = Data::ParticleInt32Parameter(10);

        EXPECT_EQ(subject.getActiveParticlesProperty(), 0);
        subject.Trigger(Vector2(0.0f, 0.0f));
        EXPECT_EQ(subject.getActiveParticlesProperty(), 10);
    }

    TEST(ParticleEmitterTests, TriggerWithoutHeadroomClampsToRemainingCapacity)
    {
        ParticleEmitter subject(15);
        subject.getParametersProperty().Quantity = Data::ParticleInt32Parameter(10);

        subject.Trigger(Vector2(0.0f, 0.0f));
        EXPECT_EQ(subject.getActiveParticlesProperty(), 10);
        subject.Trigger(Vector2(0.0f, 0.0f));
        EXPECT_EQ(subject.getActiveParticlesProperty(), 15);
    }

    TEST(ParticleEmitterTests, TriggerWhenFullDoesNotIncreaseActiveParticles)
    {
        ParticleEmitter subject(10);
        subject.getParametersProperty().Quantity = Data::ParticleInt32Parameter(10);

        subject.Trigger(Vector2(0.0f, 0.0f));
        EXPECT_EQ(subject.getActiveParticlesProperty(), 10);
        subject.Trigger(Vector2(0.0f, 0.0f));
        EXPECT_EQ(subject.getActiveParticlesProperty(), 10);
    }

    TEST(ParticleEmitterTests, UpdateExpiresParticlesPastTheirLifeSpan)
    {
        ParticleEmitter subject(100);
        subject.setLifeSpanProperty(1.0f);
        subject.getParametersProperty().Quantity = Data::ParticleInt32Parameter(1);

        subject.Trigger(Vector2(0.0f, 0.0f));
        EXPECT_EQ(subject.getActiveParticlesProperty(), 1);

        subject.Update(2.0f);
        EXPECT_EQ(subject.getActiveParticlesProperty(), 0);
    }

    TEST(ParticleEmitterTests, UpdateWithNoActiveParticlesDoesNothing)
    {
        ParticleEmitter subject(100);

        subject.Update(0.5f);
        EXPECT_EQ(subject.getActiveParticlesProperty(), 0);
    }

    TEST(ParticleEmitterTests, DisposeIsIdempotent)
    {
        ParticleEmitter subject(10);

        subject.Dispose();
        subject.Dispose();
        EXPECT_TRUE(subject.getIsDisposedProperty());
    }

    TEST(ParticleEmitterTests, ChangeCapacityReplacesBuffer)
    {
        ParticleEmitter subject(10);
        subject.ChangeCapacity(50);
        EXPECT_EQ(subject.getCapacityProperty(), 50);
    }
}
