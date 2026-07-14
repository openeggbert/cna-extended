// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for ParticleEmitter3DEXT/ParticleEffect3DEXT (see
// 3d.md/plan3d.md) -- there is no upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/ParticleEmitter3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ParticleEffect3DEXT.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <memory>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Vector3;

    TEST(ParticleEmitter3DEXTTests, EmitEXT_AddsRequestedParticleCount)
    {
        ParticleEmitter3DEXT emitter;
        emitter.EmitEXT(10, Vector3::Zero);

        EXPECT_EQ(emitter.ParticlesEXT.size(), 10u);
    }

    TEST(ParticleEmitter3DEXTTests, EmitEXT_RespectsMaxParticlesEXTCap)
    {
        ParticleEmitter3DEXT emitter;
        emitter.MaxParticlesEXT = 5;
        emitter.EmitEXT(10, Vector3::Zero);

        EXPECT_EQ(emitter.ParticlesEXT.size(), 5u);
    }

    TEST(ParticleEmitter3DEXTTests, EmitEXT_PlacesNewParticlesAtOrigin)
    {
        ParticleEmitter3DEXT emitter;
        const Vector3 origin(3.0f, 4.0f, 5.0f);
        emitter.EmitEXT(1, origin);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 1u);
        EXPECT_EQ(emitter.ParticlesEXT[0].PositionEXT, origin);
    }

    TEST(ParticleEmitter3DEXTTests, UpdateEXT_AdvancesAgeAndIntegratesVelocity)
    {
        ParticleEmitter3DEXT emitter;
        emitter.IsEmittingEXT = false;
        emitter.MinSpeedEXT = 0.0f;
        emitter.MaxSpeedEXT = 0.0f;
        emitter.EmitEXT(1, Vector3::Zero);
        emitter.ParticlesEXT[0].VelocityEXT = Vector3(2.0f, 0.0f, 0.0f);

        emitter.UpdateEXT(0.5f, Vector3::Zero);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 1u);
        EXPECT_NEAR(emitter.ParticlesEXT[0].AgeEXT, 0.5f, 1e-5f);
        EXPECT_NEAR(emitter.ParticlesEXT[0].PositionEXT.X, 1.0f, 1e-5f);
    }

    TEST(ParticleEmitter3DEXTTests, UpdateEXT_GravityAccumulatesIntoVelocity)
    {
        ParticleEmitter3DEXT emitter;
        emitter.IsEmittingEXT = false;
        emitter.MinSpeedEXT = 0.0f;
        emitter.MaxSpeedEXT = 0.0f;
        emitter.MinLifetimeEXT = 100.0f;
        emitter.MaxLifetimeEXT = 100.0f;
        emitter.GravityEXT = Vector3(0.0f, -10.0f, 0.0f);
        emitter.EmitEXT(1, Vector3::Zero);

        emitter.UpdateEXT(1.0f, Vector3::Zero);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 1u);
        EXPECT_NEAR(emitter.ParticlesEXT[0].VelocityEXT.Y, -10.0f, 1e-4f);
    }

    TEST(ParticleEmitter3DEXTTests, UpdateEXT_RemovesExpiredParticles)
    {
        ParticleEmitter3DEXT emitter;
        emitter.IsEmittingEXT = false;
        emitter.MinLifetimeEXT = 1.0f;
        emitter.MaxLifetimeEXT = 1.0f;
        emitter.EmitEXT(3, Vector3::Zero);

        emitter.UpdateEXT(2.0f, Vector3::Zero);

        EXPECT_TRUE(emitter.ParticlesEXT.empty());
    }

    TEST(ParticleEmitter3DEXTTests, UpdateEXT_InterpolatesOpacityTowardEndOpacityOverLifetime)
    {
        ParticleEmitter3DEXT emitter;
        emitter.IsEmittingEXT = false;
        emitter.MinLifetimeEXT = 1.0f;
        emitter.MaxLifetimeEXT = 1.0f;
        emitter.StartOpacityEXT = 1.0f;
        emitter.EndOpacityEXT = 0.0f;
        emitter.EmitEXT(1, Vector3::Zero);

        emitter.UpdateEXT(0.5f, Vector3::Zero);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 1u);
        EXPECT_NEAR(emitter.ParticlesEXT[0].OpacityEXT, 0.5f, 1e-4f);
    }

    TEST(ParticleEmitter3DEXTTests, UpdateEXT_EmitsOverTimeAtEmissionRate)
    {
        ParticleEmitter3DEXT emitter;
        emitter.EmissionRateEXT = 10.0f; // 10/sec
        emitter.MinLifetimeEXT = 100.0f;
        emitter.MaxLifetimeEXT = 100.0f;

        emitter.UpdateEXT(1.0f, Vector3::Zero);

        EXPECT_EQ(emitter.ParticlesEXT.size(), 10u);
    }

    // A-06 regression tests (audit.md): ParticleEmitter3DEXT's public fields had no
    // validation at EmitEXT/UpdateEXT boundaries -- see ParticleEmitter3DEXT.cpp's fix
    // sites for details.

    TEST(ParticleEmitter3DEXTTests, EmitEXT_ZeroConeDirection_ProducesFiniteVelocity)
    {
        ParticleEmitter3DEXT emitter;
        emitter.ConeDirectionEXT = Vector3::Zero;
        emitter.MinSpeedEXT = 1.0f;
        emitter.MaxSpeedEXT = 1.0f;
        emitter.EmitEXT(20, Vector3::Zero);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 20u);
        for (const Particle3DEXT& particle : emitter.ParticlesEXT)
        {
            EXPECT_TRUE(std::isfinite(particle.VelocityEXT.X));
            EXPECT_TRUE(std::isfinite(particle.VelocityEXT.Y));
            EXPECT_TRUE(std::isfinite(particle.VelocityEXT.Z));
        }
    }

    TEST(ParticleEmitter3DEXTTests, EmitEXT_ClampsOutOfRangeStartOpacity)
    {
        ParticleEmitter3DEXT emitter;
        emitter.StartOpacityEXT = 5.0f;
        emitter.EmitEXT(1, Vector3::Zero);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 1u);
        EXPECT_NEAR(emitter.ParticlesEXT[0].OpacityEXT, 1.0f, 1e-6f);

        ParticleEmitter3DEXT negativeEmitter;
        negativeEmitter.StartOpacityEXT = -3.0f;
        negativeEmitter.EmitEXT(1, Vector3::Zero);

        ASSERT_EQ(negativeEmitter.ParticlesEXT.size(), 1u);
        EXPECT_NEAR(negativeEmitter.ParticlesEXT[0].OpacityEXT, 0.0f, 1e-6f);
    }

    TEST(ParticleEmitter3DEXTTests, UpdateEXT_ClampsOutOfRangeInterpolatedOpacity)
    {
        ParticleEmitter3DEXT emitter;
        emitter.IsEmittingEXT = false;
        emitter.MinLifetimeEXT = 1.0f;
        emitter.MaxLifetimeEXT = 1.0f;
        emitter.StartOpacityEXT = 2.0f;
        emitter.EndOpacityEXT = -1.0f;
        emitter.EmitEXT(1, Vector3::Zero);

        emitter.UpdateEXT(0.5f, Vector3::Zero);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 1u);
        const float opacity = emitter.ParticlesEXT[0].OpacityEXT;
        EXPECT_GE(opacity, 0.0f);
        EXPECT_LE(opacity, 1.0f);
    }

    // Follow-up robustness pass on A-06 (2026-07-14): not a new audit finding, but a
    // deliberate broadening of the same "validate at EmitEXT/UpdateEXT boundaries"
    // principle, per a follow-up review confirming NaN/reversed-range handling wasn't
    // fully nailed down yet.

    TEST(ParticleEmitter3DEXTTests, EmitEXT_ReversedLifetimeRange_StillProducesValueWithinRange)
    {
        // Min > Max is not actually a bug: min + t*(max-min) for t in [0,1) always lands
        // between min and max regardless of which is larger (a negative (max-min) just
        // reverses the interpolation direction). This locks that in as a real invariant,
        // not an assumption.
        ParticleEmitter3DEXT emitter;
        emitter.MinLifetimeEXT = 5.0f;
        emitter.MaxLifetimeEXT = 1.0f;
        emitter.EmitEXT(20, Vector3::Zero);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 20u);
        for (const Particle3DEXT& particle : emitter.ParticlesEXT)
        {
            EXPECT_GE(particle.LifetimeEXT, 1.0f);
            EXPECT_LE(particle.LifetimeEXT, 5.0f);
        }
    }

    TEST(ParticleEmitter3DEXTTests, EmitEXT_NonFiniteLifetimeRange_ProducesFiniteLifetimeThatExpiresImmediately)
    {
        // A NaN/Infinite LifetimeEXT would make IsExpiredEXT()'s `AgeEXT >= LifetimeEXT`
        // false forever (NaN compares false against everything in IEEE 754), leaking the
        // particle permanently. Guarded by falling back to 0 (== already expired).
        ParticleEmitter3DEXT emitter;
        emitter.IsEmittingEXT = false;
        emitter.MinLifetimeEXT = std::nan("");
        emitter.MaxLifetimeEXT = std::nan("");
        emitter.EmitEXT(1, Vector3::Zero);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 1u);
        EXPECT_TRUE(std::isfinite(emitter.ParticlesEXT[0].LifetimeEXT));

        emitter.UpdateEXT(0.0f, Vector3::Zero);
        EXPECT_TRUE(emitter.ParticlesEXT.empty()) << "non-finite-lifetime particle should have been treated as already expired";
    }

    TEST(ParticleEmitter3DEXTTests, EmitEXT_InfiniteLifetimeRange_ProducesFiniteLifetimeThatExpiresImmediately)
    {
        ParticleEmitter3DEXT emitter;
        emitter.IsEmittingEXT = false;
        emitter.MinLifetimeEXT = std::numeric_limits<float>::infinity();
        emitter.MaxLifetimeEXT = std::numeric_limits<float>::infinity();
        emitter.EmitEXT(1, Vector3::Zero);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 1u);
        EXPECT_TRUE(std::isfinite(emitter.ParticlesEXT[0].LifetimeEXT));

        emitter.UpdateEXT(0.0f, Vector3::Zero);
        EXPECT_TRUE(emitter.ParticlesEXT.empty());
    }

    TEST(ParticleEmitter3DEXTTests, EmitEXT_NonFiniteSpeedRange_ProducesFiniteVelocity)
    {
        ParticleEmitter3DEXT emitter;
        emitter.MinSpeedEXT = std::nan("");
        emitter.MaxSpeedEXT = std::nan("");
        emitter.EmitEXT(1, Vector3::Zero);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 1u);
        EXPECT_TRUE(std::isfinite(emitter.ParticlesEXT[0].VelocityEXT.X));
        EXPECT_TRUE(std::isfinite(emitter.ParticlesEXT[0].VelocityEXT.Y));
        EXPECT_TRUE(std::isfinite(emitter.ParticlesEXT[0].VelocityEXT.Z));
    }

    TEST(ParticleEmitter3DEXTTests, EmitEXT_NonFiniteScaleRange_ProducesFiniteScale)
    {
        ParticleEmitter3DEXT emitter;
        emitter.MinScaleEXT = std::numeric_limits<float>::infinity();
        emitter.MaxScaleEXT = std::numeric_limits<float>::infinity();
        emitter.EmitEXT(1, Vector3::Zero);

        ASSERT_EQ(emitter.ParticlesEXT.size(), 1u);
        EXPECT_TRUE(std::isfinite(emitter.ParticlesEXT[0].ScaleEXT));
    }

    TEST(ParticleEffect3DEXTTests, AddEmitterEXT_UpdateEXT_ForwardsToEveryOwnedEmitter)
    {
        ParticleEffect3DEXT effect;
        ParticleEmitter3DEXT& first = effect.AddEmitterEXT(std::make_unique<ParticleEmitter3DEXT>());
        ParticleEmitter3DEXT& second = effect.AddEmitterEXT(std::make_unique<ParticleEmitter3DEXT>());
        first.IsEmittingEXT = false;
        second.IsEmittingEXT = false;
        first.EmitEXT(2, Vector3::Zero);
        second.EmitEXT(3, Vector3::Zero);

        effect.UpdateEXT(0.1f, Vector3::Zero);

        EXPECT_EQ(effect.getEmittersProperty().size(), 2u);
        EXPECT_EQ(first.ParticlesEXT.size(), 2u);
        EXPECT_EQ(second.ParticlesEXT.size(), 3u);
        for (const Particle3DEXT& p : first.ParticlesEXT)
        {
            EXPECT_NEAR(p.AgeEXT, 0.1f, 1e-5f);
        }
    }
}
