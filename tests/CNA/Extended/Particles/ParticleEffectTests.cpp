// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for ParticleEffect. Fresh tests below.
#include "CNA/Extended/Particles/ParticleEffect.hpp"

#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/IO/MemoryStream.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <string_view>

namespace CNA::Extended::Particles
{
    using Microsoft::Xna::Framework::Content::ContentManager;

    namespace
    {
        constexpr const char* kEmptyEffectXml =
            R"(<?xml version="1.0" encoding="utf-8"?><ParticleEffect Name="EmptyEffect" Position="0,0" Rotation="0" Scale="1,1" AutoTrigger="True" AutoTriggerFrequency="1" />)";
    }

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

    // Regression tests: FromFile/FromStream were previously not ported at all -- the header
    // comment deferred them behind ParticleEffectSerializer.cs being out of scope, but that file
    // was fully ported in the same phase and the follow-up was never done (found via a
    // member-level audit against upstream). Both just delegate to
    // ParticleEffectSerializer::Deserialize, already exhaustively tested in
    // ParticleEffectSerializerTests.cpp -- these two tests only need to confirm the delegation
    // itself works, not re-verify parsing correctness.
    TEST(ParticleEffectTests, FromStreamDelegatesToSerializer)
    {
        ContentManager content;
        System::IO::MemoryStream stream(reinterpret_cast<const SharpRuntime::bytecs*>(kEmptyEffectXml), static_cast<SharpRuntime::intcs>(std::string_view(kEmptyEffectXml).size()));

        const std::unique_ptr<ParticleEffect> effect = ParticleEffect::FromStream(stream, content);

        ASSERT_NE(effect, nullptr);
        EXPECT_EQ(effect->getNameProperty(), "EmptyEffect");
    }

    TEST(ParticleEffectTests, FromFileDelegatesToSerializer)
    {
        const std::filesystem::path path = std::filesystem::temp_directory_path() / "cna_extended_particle_effect_from_file_test.xml";
        {
            std::ofstream file(path);
            file << kEmptyEffectXml;
        }

        ContentManager content;
        const std::unique_ptr<ParticleEffect> effect = ParticleEffect::FromFile(path.string(), content);

        std::filesystem::remove(path);

        ASSERT_NE(effect, nullptr);
        EXPECT_EQ(effect->getNameProperty(), "EmptyEffect");
    }
}
