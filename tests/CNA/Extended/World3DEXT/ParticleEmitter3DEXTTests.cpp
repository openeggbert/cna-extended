// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for ParticleEmitter3DEXT/ParticleEffect3DEXT (see
// 3d.md/plan3d.md) -- there is no upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/ParticleEmitter3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ParticleEffect3DEXT.hpp"

#include <gtest/gtest.h>
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
