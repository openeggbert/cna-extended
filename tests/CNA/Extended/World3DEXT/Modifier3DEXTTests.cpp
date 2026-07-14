// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for Modifier3DEXT (base class), ModifierExecutionStrategy3DEXT,
// AgeModifier3DEXT, and LinearGravityModifier3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/AgeModifier3DEXT.hpp"
#include "CNA/Extended/World3DEXT/LinearGravityModifier3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ModifierExecutionStrategy3DEXT.hpp"
#include "CNA/Extended/World3DEXT/OpacityInterpolator3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    namespace
    {
        // Test double exercising Modifier3DEXT's protected surface: records which particles
        // its Update() touched (startIndex/particleCount) each call, without altering them.
        class RecordingModifier3DEXT final : public Modifier3DEXT
        {
        public:
            RecordingModifier3DEXT() : Modifier3DEXT("RecordingModifier3DEXT") {}

            int lastStartIndex = -1;
            int lastParticleCount = -1;
            int updateCallCount = 0;

        protected:
            void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override
            {
                (void)elapsedSeconds;
                (void)particles;
                lastStartIndex = startIndex;
                lastParticleCount = particleCount;
                ++updateCallCount;
            }
        };
    }

    TEST(Modifier3DEXTTests, DefaultFrequencyIsSixty)
    {
        RecordingModifier3DEXT modifier;
        EXPECT_FLOAT_EQ(modifier.getFrequencyProperty(), 60.0f);
    }

    TEST(Modifier3DEXTTests, SetFrequency_Positive_UpdatesValue)
    {
        RecordingModifier3DEXT modifier;
        modifier.setFrequencyProperty(30.0f);
        EXPECT_FLOAT_EQ(modifier.getFrequencyProperty(), 30.0f);
    }

    TEST(Modifier3DEXTTests, SetFrequency_Zero_Throws)
    {
        RecordingModifier3DEXT modifier;
        EXPECT_THROW(modifier.setFrequencyProperty(0.0f), System::ArgumentOutOfRangeException);
    }

    TEST(Modifier3DEXTTests, SetFrequency_Negative_Throws)
    {
        RecordingModifier3DEXT modifier;
        EXPECT_THROW(modifier.setFrequencyProperty(-1.0f), System::ArgumentOutOfRangeException);
    }

    TEST(Modifier3DEXTTests, NameDefaultsToConstructorArgument)
    {
        RecordingModifier3DEXT modifier;
        EXPECT_EQ(modifier.getNameProperty(), "RecordingModifier3DEXT");
    }

    TEST(Modifier3DEXTTests, SetName_UpdatesValue)
    {
        RecordingModifier3DEXT modifier;
        modifier.setNameProperty("Renamed");
        EXPECT_EQ(modifier.getNameProperty(), "Renamed");
    }

    TEST(Modifier3DEXTTests, EnabledDefaultsToTrue)
    {
        RecordingModifier3DEXT modifier;
        EXPECT_TRUE(modifier.getEnabledProperty());
    }

    TEST(Modifier3DEXTTests, Disabled_InternalUpdate_DoesNothing)
    {
        RecordingModifier3DEXT modifier;
        modifier.setEnabledProperty(false);

        std::vector<Particle3DEXT> particles(3);
        modifier.InternalUpdate(1.0f, particles);

        EXPECT_EQ(modifier.updateCallCount, 0);
    }

    TEST(Modifier3DEXTTests, EmptyParticleList_InternalUpdate_DoesNothing)
    {
        RecordingModifier3DEXT modifier;
        std::vector<Particle3DEXT> particles;

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_EQ(modifier.updateCallCount, 0);
    }

    TEST(Modifier3DEXTTests, LargeElapsedSeconds_UpdatesEveryParticleInOneCall)
    {
        // Frequency defaults to 60 (cycleTime_ ~= 0.0167s); an elapsedSeconds far larger than
        // that forces InternalUpdate's ceil(...) pacing formula to cover the whole list in a
        // single call, matching how ParticleEmitter3DEXT's own built-in modifiers guarantee
        // full-list coverage every UpdateEXT (see Modifier3DEXT.hpp's header comment).
        RecordingModifier3DEXT modifier;
        std::vector<Particle3DEXT> particles(5);

        modifier.InternalUpdate(10.0f, particles);

        EXPECT_EQ(modifier.updateCallCount, 1);
        EXPECT_EQ(modifier.lastStartIndex, 0);
        EXPECT_EQ(modifier.lastParticleCount, 5);
    }

    TEST(ModifierExecutionStrategy3DEXTTests, SerialAppliesModifierToEachInList)
    {
        std::vector<Particle3DEXT> particles(2);
        RecordingModifier3DEXT first;
        RecordingModifier3DEXT second;
        std::vector<Modifier3DEXT*> modifiers{&first, &second};

        ModifierExecutionStrategy3DEXT::getSerialProperty().ExecuteModifiers(modifiers, 10.0f, particles);

        EXPECT_EQ(first.updateCallCount, 1);
        EXPECT_EQ(second.updateCallCount, 1);
    }

    TEST(ModifierExecutionStrategy3DEXTTests, SerialSingletonIsStable)
    {
        EXPECT_EQ(&ModifierExecutionStrategy3DEXT::getSerialProperty(), &ModifierExecutionStrategy3DEXT::getSerialProperty());
    }

    TEST(AgeModifier3DEXTTests, AppliesInterpolatorsBasedOnParticleAge)
    {
        AgeModifier3DEXT modifier;
        auto opacityInterpolator = std::make_unique<OpacityInterpolator3DEXT>();
        opacityInterpolator->StartValue = 1.0f;
        opacityInterpolator->EndValue = 0.0f;
        modifier.getInterpolatorsProperty().push_back(std::move(opacityInterpolator));

        std::vector<Particle3DEXT> particles(1);
        particles[0].AgeEXT = 0.5f;
        particles[0].LifetimeEXT = 1.0f;

        modifier.InternalUpdate(10.0f, particles);

        EXPECT_FLOAT_EQ(particles[0].OpacityEXT, 0.5f);
    }

    TEST(AgeModifier3DEXTTests, Disabled_DoesNotApplyInterpolators)
    {
        AgeModifier3DEXT modifier;
        modifier.setEnabledProperty(false);
        auto opacityInterpolator = std::make_unique<OpacityInterpolator3DEXT>();
        opacityInterpolator->StartValue = 1.0f;
        opacityInterpolator->EndValue = 0.0f;
        modifier.getInterpolatorsProperty().push_back(std::move(opacityInterpolator));

        std::vector<Particle3DEXT> particles(1);
        particles[0].AgeEXT = 0.5f;
        particles[0].LifetimeEXT = 1.0f;
        particles[0].OpacityEXT = 0.75f;

        modifier.InternalUpdate(10.0f, particles);

        EXPECT_FLOAT_EQ(particles[0].OpacityEXT, 0.75f);
    }

    TEST(LinearGravityModifier3DEXTTests, AddsGravityScaledByMassAndElapsedSeconds)
    {
        LinearGravityModifier3DEXT modifier;
        modifier.GravityEXT = Vector3(0.0f, -10.0f, 0.0f);

        std::vector<Particle3DEXT> particles(1);
        particles[0].MassEXT = 2.0f;
        particles[0].VelocityEXT = Vector3::Zero;

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_NEAR(particles[0].VelocityEXT.Y, -20.0f, 1e-4f);
    }

    TEST(LinearGravityModifier3DEXTTests, Disabled_DoesNotAlterVelocity)
    {
        LinearGravityModifier3DEXT modifier;
        modifier.setEnabledProperty(false);
        modifier.GravityEXT = Vector3(0.0f, -10.0f, 0.0f);

        std::vector<Particle3DEXT> particles(1);
        particles[0].VelocityEXT = Vector3::Zero;

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_FLOAT_EQ(particles[0].VelocityEXT.Y, 0.0f);
    }
}
