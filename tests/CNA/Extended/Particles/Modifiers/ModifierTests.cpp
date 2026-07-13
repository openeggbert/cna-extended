// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for these Modifier subclasses (upstream's own EmitterTests.cs, which
// exercised modifiers indirectly through ParticleEmitter, is itself commented out / stale against
// the current API -- see ParticleBufferTests.cpp's header comment for the same situation). Fresh
// tests below, exercising each modifier the same way ParticleEmitter/AnimationController-style
// production code would: through the public Modifier::InternalUpdate() entry point (Update() itself
// is protected, only reachable via InternalUpdate's frequency-distribution logic), over a real
// ParticleBuffer/ParticleIterator. `elapsedSeconds` is chosen much larger than a modifier's cycle
// time (1/Frequency, default 60) so InternalUpdate's per-frame distribution logic updates every
// particle in a single call, keeping the tests deterministic.
#include "CNA/Extended/Particles/Modifiers/DragModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/LinearGravityModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/OpacityFastFadeModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/RotationModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/VelocityColorModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/VortexModifier.hpp"
#include "CNA/Extended/Particles/ParticleBuffer.hpp"

#include <functional>
#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Modifiers
{
    namespace
    {
        // Releases one particle, applies @p init to it, runs @p modifier over it with a large
        // elapsedSeconds (guaranteeing a full update in one call), and returns a pointer to the
        // (mutated) particle for inspection. The buffer/iterator are kept alive via out-params.
        Data::Particle* ReleaseAndUpdate(
            ParticleBuffer& buffer, Modifier& modifier, const std::function<void(Data::Particle&)>& init)
        {
            ParticleIterator& releaseIterator = buffer.Release(1);
            Data::Particle* particle = releaseIterator.Next();
            init(*particle);

            ParticleIterator& updateIterator = buffer.getIteratorProperty();
            modifier.InternalUpdate(1.0f, updateIterator);
            return particle;
        }
    }

    TEST(DragModifierTests, SlowsParticleVelocity)
    {
        ParticleBuffer buffer(1);
        DragModifier modifier;

        const Data::Particle* particle = ReleaseAndUpdate(buffer, modifier, [](Data::Particle& p) {
            p.Velocity[0] = 100.0f;
            p.Velocity[1] = 0.0f;
            p.Mass = 1.0f;
        });

        EXPECT_LT(particle->Velocity[0], 100.0f);
    }

    TEST(LinearGravityModifierTests, AddsForceProportionalToMass)
    {
        ParticleBuffer buffer(1);
        LinearGravityModifier modifier;
        modifier.Direction = Vector2(0.0f, 1.0f);
        modifier.Strength = 10.0f;

        const Data::Particle* particle = ReleaseAndUpdate(buffer, modifier, [](Data::Particle& p) {
            p.Velocity[0] = 0.0f;
            p.Velocity[1] = 0.0f;
            p.Mass = 2.0f;
        });

        EXPECT_GT(particle->Velocity[1], 0.0f);
    }

    TEST(OpacityFastFadeModifierTests, SetsOpacityToOneMinusAge)
    {
        ParticleBuffer buffer(1);
        OpacityFastFadeModifier modifier;

        const Data::Particle* particle = ReleaseAndUpdate(buffer, modifier, [](Data::Particle& p) { p.Age = 0.25f; });

        EXPECT_FLOAT_EQ(particle->Opacity, 0.75f);
    }

    TEST(RotationModifierTests, IncreasesRotationOverTime)
    {
        ParticleBuffer buffer(1);
        RotationModifier modifier;
        modifier.RotationRate = 1.0f;

        const Data::Particle* particle = ReleaseAndUpdate(buffer, modifier, [](Data::Particle& p) { p.Rotation = 0.0f; });

        EXPECT_GT(particle->Rotation, 0.0f);
    }

    TEST(VelocityColorModifierTests, FastParticleGetsVelocityColor)
    {
        ParticleBuffer buffer(1);
        VelocityColorModifier modifier;
        modifier.VelocityThreshold = 1.0f;
        modifier.VelocityColor = HslColor(0.5f, 0.5f, 0.9f);
        modifier.StationaryColor = HslColor(0.0f, 0.0f, 0.1f);

        const Data::Particle* particle = ReleaseAndUpdate(buffer, modifier, [](Data::Particle& p) {
            p.Velocity[0] = 100.0f;
            p.Velocity[1] = 0.0f;
        });

        EXPECT_FLOAT_EQ(particle->Color[2], 0.9f);
    }

    TEST(VortexModifierTests, PullsParticleTowardCenterWithinRadii)
    {
        ParticleBuffer buffer(1);
        VortexModifier modifier;
        modifier.Position = Vector2::Zero;
        modifier.Strength = 100.0f;
        modifier.InnerRadius = 0.0f;
        modifier.OuterRadius = 100.0f;
        modifier.MaxVelocity = 1000.0f;
        modifier.setRotationAngleProperty(0.0f);

        const Data::Particle* particle = ReleaseAndUpdate(buffer, modifier, [](Data::Particle& p) {
            p.TriggeredPos[0] = 0.0f;
            p.TriggeredPos[1] = 0.0f;
            p.Position[0] = 10.0f;
            p.Position[1] = 0.0f;
            p.Velocity[0] = 0.0f;
            p.Velocity[1] = 0.0f;
        });

        // Pure attraction (angle 0) should accelerate the particle toward the center, i.e. negative X velocity.
        EXPECT_LT(particle->Velocity[0], 0.0f);
    }

    TEST(VortexModifierTests, SetRotationAngleRecomputesCachedTrig)
    {
        VortexModifier modifier;
        modifier.setRotationAngleProperty(1.5f);
        EXPECT_FLOAT_EQ(modifier.getRotationAngleProperty(), 1.5f);
    }
}
