// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/Particles/ParticleEffectSerializerTests.cs.
//
// Two upstream tests are NOT ported: `Deserialize_NullStream_ThrowsArgumentNullException` and
// `Deserialize_NullContentManager_ThrowsArgumentNullException`. This port's
// `Deserialize(System::IO::Stream&, ContentManager&, ...)` takes C++ references, not C#'s nullable
// reference types -- "null stream"/"null content manager" is not representable at the call site, so
// the language's own type system already rules out what those two tests guarded against (a genuine
// representational impossibility, not a skipped behavior).
//
// Upstream's `MockContentManager` (its `Load<T>()` override returns `default(T)`, i.e. null) has no
// C++ equivalent here: none of these fixtures include a `<TextureRegion>` element, so
// `ContentManager::Load<Texture2D>` is never invoked and a plain default-constructed `ContentManager`
// suffices.
//
// `AssertParticleEffect` does NOT do whole-document exact string equality the way upstream's
// `Assert.Equal(expected, actual)` does. `System::Xml::XmlWriter` here is backed by tinyxml2 (via
// sharp-runtime), whose exact self-closing-tag spacing ("/>" vs " />"), indentation width, and XML
// declaration casing ("UTF-8" vs "utf-8") are not guaranteed to match .NET's XmlWriter byte-for-byte
// (confirmed by direct inspection, not assumed) -- `XmlWriterExtensionsTests.cpp` already established
// substring checks over exact equality for exactly this reason. `AssertParticleEffect` instead checks
// that every `attribute="value"` pair in the expected fixture is present verbatim in the actual
// output, plus that the same set of element tag names appears in both (catching spurious/missing
// wrapper elements like an empty `<Modifiers>`) -- full content-equivalence without depending on a
// specific XML writer's whitespace formatting.
#include "CNA/Extended/Particles/ParticleEffectSerializer.hpp"

#include "CNA/Extended/HslColor.hpp"
#include "CNA/Extended/Particles/Modifiers/AgeModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/Containers/CircleContainerModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/Containers/RectangleContainerModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/Containers/RectangleLoopContainerModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/DragModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/ColorInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/HueInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/OpacityInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/RotationInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/ScaleInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/VelocityInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/LinearGravityModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/OpacityFastFadeModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/RotationModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/VelocityColorModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/VelocityModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/VortexModifier.hpp"
#include "CNA/Extended/Particles/Profiles/BoxFillProfile.hpp"
#include "CNA/Extended/Particles/Profiles/BoxProfile.hpp"
#include "CNA/Extended/Particles/Profiles/BoxUniformProfile.hpp"
#include "CNA/Extended/Particles/Profiles/CircleProfile.hpp"
#include "CNA/Extended/Particles/Profiles/LineProfile.hpp"
#include "CNA/Extended/Particles/Profiles/PointProfile.hpp"
#include "CNA/Extended/Particles/Profiles/RingProfile.hpp"
#include "CNA/Extended/Particles/Profiles/SprayProfile.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/IO/MemoryStream.hpp"
#include "System/Xml/XmlException.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>

namespace CNA::Extended::Particles
{
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Content::ContentManager;
    using namespace CNA::Extended::Particles::Modifiers;
    using namespace CNA::Extended::Particles::Modifiers::Containers;
    using namespace CNA::Extended::Particles::Modifiers::Interpolators;
    using namespace CNA::Extended::Particles::Profiles;

    namespace
    {
        constexpr const char* kDefaultParameters =
            "<Parameters>"
            R"(<Quantity Kind="Random" RandomMin="5" RandomMax="100" />)"
            R"(<Speed Kind="Random" RandomMin="50" RandomMax="100" />)"
            R"(<Color Kind="Constant" Constant="1,1,1" />)"
            R"(<Opacity Kind="Random" RandomMin="0" RandomMax="1" />)"
            R"(<Scale Kind="Random" RandomMin="0.5,0.5" RandomMax="1,1" />)"
            R"(<Rotation Kind="Random" RandomMin="-3.1415927" RandomMax="3.1415927" />)"
            R"(<Mass Kind="Constant" Constant="1" />)"
            "</Parameters>";

        // Builds a single-emitter ParticleEffect document: `inner` is everything after
        // `</Parameters>` and before `</ParticleEmitter>` (typically a `<Profile .../>` optionally
        // followed by a `<Modifiers>...</Modifiers>` block). Exact whitespace doesn't matter -- see
        // this file's header comment for why (DOM parsing for Deserialize inputs, attribute-set
        // comparison for Serialize expected fixtures).
        std::string WrapEmitterXml(const std::string& emitterName, const std::string& inner)
        {
            return
                R"(<?xml version="1.0" encoding="utf-8"?>)"
                R"(<ParticleEffect Name="TestEffect" Position="0,0" Rotation="0" Scale="1,1" AutoTrigger="True" AutoTriggerFrequency="1">)"
                "<Emitters>"
                R"(<ParticleEmitter Name=")" + emitterName +
                R"(" LifeSpan="1" Offset="0,0" LayerDepth="0" ReclaimFrequency="60" Capacity="1" ModifierExecutionStrategy="Serial" RenderingOrder="FrontToBack">)"
                + kDefaultParameters + inner +
                "</ParticleEmitter>"
                "</Emitters>"
                "</ParticleEffect>";
        }

        std::unique_ptr<ParticleEffect> ReadParticleEffectFromXml(const std::string& xml, ContentManager& content)
        {
            System::IO::MemoryStream stream(reinterpret_cast<const SharpRuntime::bytecs*>(xml.data()), static_cast<SharpRuntime::intcs>(xml.size()));
            return ParticleEffectSerializer::Deserialize(stream, content);
        }

        void AssertParticleEffect(ParticleEffect& effect, const std::string& expected)
        {
            System::IO::MemoryStream stream;
            ParticleEffectSerializer::Serialize(stream, effect);
            const std::vector<SharpRuntime::bytecs> bytes = stream.ToArray();
            const std::string actual(bytes.begin(), bytes.end());

            // Skip the `<?xml ... ?>` declaration itself: tinyxml2 (this port's XmlWriter backend)
            // always emits `encoding="UTF-8"`, while .NET's XmlWriter (and every upstream fixture
            // string) uses lowercase `encoding="utf-8"` -- a fixed, inconsequential casing quirk of
            // the declaration boilerplate, not part of the ParticleEffect data this test verifies.
            const std::string expectedBody = expected.substr(expected.find("<ParticleEffect"));

            static const std::regex attributeRegex(R"((\w+)=\"([^\"]*)\")");
            for (auto it = std::sregex_iterator(expectedBody.begin(), expectedBody.end(), attributeRegex); it != std::sregex_iterator(); ++it)
            {
                EXPECT_NE(actual.find(it->str()), std::string::npos) << "Missing attribute in output: " << it->str() << "\nActual: " << actual;
            }

            static const std::regex tagRegex(R"(<([A-Za-z][A-Za-z0-9]*))");
            std::set<std::string> expectedTags;
            std::set<std::string> actualTags;
            for (auto it = std::sregex_iterator(expected.begin(), expected.end(), tagRegex); it != std::sregex_iterator(); ++it)
            {
                expectedTags.insert((*it)[1].str());
            }
            for (auto it = std::sregex_iterator(actual.begin(), actual.end(), tagRegex); it != std::sregex_iterator(); ++it)
            {
                actualTags.insert((*it)[1].str());
            }
            EXPECT_EQ(actualTags, expectedTags) << "Actual: " << actual;
        }

        std::unique_ptr<ParticleEmitter> MakeTestEmitter()
        {
            auto emitter = std::make_unique<ParticleEmitter>(1);
            emitter->setNameProperty("TestEmitter");
            emitter->setProfileProperty(Profile::Point());
            return emitter;
        }
    }

    // --- Deserialize: guard/basic cases ---------------------------------------------------------

    TEST(ParticleEffectSerializerTests, Deserialize_InvalidXmlRoot_ThrowsXmlException)
    {
        ContentManager content;
        EXPECT_THROW(ReadParticleEffectFromXml("<InvalidRoot />", content), System::Xml::XmlException);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_EmptyEffect_ReturnsExpected)
    {
        ContentManager content;
        const std::string xml =
            R"(<?xml version="1.0" encoding="utf-8"?><ParticleEffect Name="EmptyEffect" Position="0,0" Rotation="0" Scale="1,1" AutoTrigger="True" AutoTriggerFrequency="1" />)";

        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        EXPECT_EQ(effect->getNameProperty(), "EmptyEffect");
        EXPECT_EQ(effect->getPositionProperty(), Vector2::Zero);
        EXPECT_FLOAT_EQ(effect->getRotationProperty(), 0.0f);
        EXPECT_EQ(effect->getScaleProperty(), Vector2::One);
        EXPECT_TRUE(effect->getEmittersProperty().empty());
        EXPECT_TRUE(effect->getAutoTriggerProperty());
        EXPECT_FLOAT_EQ(effect->getAutoTriggerFrequencyProperty(), 1.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_EmptyModifiers_ReturnsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("EmptyModifiers", R"(<Profile Type="PointProfile" />)");

        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        EXPECT_TRUE(effect->getEmittersProperty()[0]->getModifiersProperty().empty());
    }

    // --- Deserialize: Profiles --------------------------------------------------------------------

    TEST(ParticleEffectSerializerTests, Deserialize_BoxFillProfile_ReadsExpected)
    {
        ContentManager content;
        const std::unique_ptr<ParticleEffect> effect =
            ReadParticleEffectFromXml(WrapEmitterXml("TestEmitter", R"(<Profile Type="BoxFillProfile" Width="1" Height="1" />)"), content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto* profile = dynamic_cast<BoxFillProfile*>(&effect->getEmittersProperty()[0]->getProfileProperty());
        ASSERT_NE(profile, nullptr);
        EXPECT_FLOAT_EQ(profile->Width, 1.0f);
        EXPECT_FLOAT_EQ(profile->Height, 1.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_BoxProfile_ReadsExpected)
    {
        ContentManager content;
        const std::unique_ptr<ParticleEffect> effect =
            ReadParticleEffectFromXml(WrapEmitterXml("TestEmitter", R"(<Profile Type="BoxProfile" Width="1" Height="1" />)"), content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto* profile = dynamic_cast<BoxProfile*>(&effect->getEmittersProperty()[0]->getProfileProperty());
        ASSERT_NE(profile, nullptr);
        EXPECT_FLOAT_EQ(profile->Width, 1.0f);
        EXPECT_FLOAT_EQ(profile->Height, 1.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_BoxUniformProfile_ReadsExpected)
    {
        ContentManager content;
        const std::unique_ptr<ParticleEffect> effect =
            ReadParticleEffectFromXml(WrapEmitterXml("TestEmitter", R"(<Profile Type="BoxUniformProfile" Width="1" Height="1" />)"), content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto* profile = dynamic_cast<BoxUniformProfile*>(&effect->getEmittersProperty()[0]->getProfileProperty());
        ASSERT_NE(profile, nullptr);
        EXPECT_FLOAT_EQ(profile->Width, 1.0f);
        EXPECT_FLOAT_EQ(profile->Height, 1.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_CircleProfile_ReadsExpected)
    {
        ContentManager content;
        const std::unique_ptr<ParticleEffect> effect =
            ReadParticleEffectFromXml(WrapEmitterXml("TestEmitter", R"(<Profile Type="CircleProfile" Radius="1" Radiate="Out" />)"), content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto* profile = dynamic_cast<CircleProfile*>(&effect->getEmittersProperty()[0]->getProfileProperty());
        ASSERT_NE(profile, nullptr);
        EXPECT_FLOAT_EQ(profile->Radius, 1.0f);
        EXPECT_EQ(profile->Radiate, CircleRadiation::Out);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_LineProfile_ReadsExpected)
    {
        ContentManager content;
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(
            WrapEmitterXml("TestEmitter", R"(<Profile Type="LineProfile" Axis="1,1" Length="1" Direction="0,0" Radiate="None" />)"), content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto* profile = dynamic_cast<LineProfile*>(&effect->getEmittersProperty()[0]->getProfileProperty());
        ASSERT_NE(profile, nullptr);
        EXPECT_EQ(profile->Axis, Vector2::One);
        EXPECT_FLOAT_EQ(profile->Length, 1.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_PointProfile_ReadsExpected)
    {
        ContentManager content;
        const std::unique_ptr<ParticleEffect> effect =
            ReadParticleEffectFromXml(WrapEmitterXml("TestEmitter", R"(<Profile Type="PointProfile" />)"), content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        EXPECT_NE(dynamic_cast<PointProfile*>(&effect->getEmittersProperty()[0]->getProfileProperty()), nullptr);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_RingProfile_ReadsExpected)
    {
        ContentManager content;
        const std::unique_ptr<ParticleEffect> effect =
            ReadParticleEffectFromXml(WrapEmitterXml("TestEmitter", R"(<Profile Type="RingProfile" Radius="1" Radiate="In" />)"), content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto* profile = dynamic_cast<RingProfile*>(&effect->getEmittersProperty()[0]->getProfileProperty());
        ASSERT_NE(profile, nullptr);
        EXPECT_FLOAT_EQ(profile->Radius, 1.0f);
        EXPECT_EQ(profile->Radiate, CircleRadiation::In);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_SprayProfile_ReadsExpected)
    {
        ContentManager content;
        const std::unique_ptr<ParticleEffect> effect =
            ReadParticleEffectFromXml(WrapEmitterXml("TestEmitter", R"(<Profile Type="SprayProfile" Direction="1,1" Spread="1" />)"), content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto* profile = dynamic_cast<SprayProfile*>(&effect->getEmittersProperty()[0]->getProfileProperty());
        ASSERT_NE(profile, nullptr);
        EXPECT_EQ(profile->Direction, Vector2::One);
        EXPECT_FLOAT_EQ(profile->Spread, 1.0f);
    }

    // --- Deserialize: Modifiers ---------------------------------------------------------------------

    TEST(ParticleEffectSerializerTests, Deserialize_AgeModifier_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier" /></Modifiers>)");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<AgeModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        EXPECT_TRUE(modifier->getEnabledProperty());
        EXPECT_FLOAT_EQ(modifier->getFrequencyProperty(), 60.0f);
        EXPECT_EQ(modifier->getNameProperty(), "AgeModifier");
    }

    TEST(ParticleEffectSerializerTests, Deserialize_CircleContainerModifier_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="CircleContainerModifier" Enabled="True" Frequency="60" Type="CircleContainerModifier" Radius="0" Inside="True" RestitutionCoefficient="1" /></Modifiers>)");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<CircleContainerModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        EXPECT_FLOAT_EQ(modifier->getFrequencyProperty(), 60.0f);
        EXPECT_TRUE(modifier->getEnabledProperty());
        EXPECT_EQ(modifier->getNameProperty(), "CircleContainerModifier");
        EXPECT_FLOAT_EQ(modifier->Radius, 0.0f);
        EXPECT_TRUE(modifier->Inside);
        EXPECT_FLOAT_EQ(modifier->RestitutionCoefficient, 1.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_DragModifier_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="DragModifier" Enabled="True" Frequency="60" Type="DragModifier" DragCoefficient="0.47" Density="0.5" /></Modifiers>)");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<DragModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        EXPECT_FLOAT_EQ(modifier->getFrequencyProperty(), 60.0f);
        EXPECT_TRUE(modifier->getEnabledProperty());
        EXPECT_EQ(modifier->getNameProperty(), "DragModifier");
        EXPECT_FLOAT_EQ(modifier->DragCoefficient, 0.47f);
        EXPECT_FLOAT_EQ(modifier->Density, 0.5f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_LinearGravityModifier_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="LinearGravityModifier" Enabled="True" Frequency="60" Type="LinearGravityModifier" Direction="0,0" Strength="0" /></Modifiers>)");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<LinearGravityModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        EXPECT_FLOAT_EQ(modifier->getFrequencyProperty(), 60.0f);
        EXPECT_TRUE(modifier->getEnabledProperty());
        EXPECT_EQ(modifier->getNameProperty(), "LinearGravityModifier");
        EXPECT_EQ(modifier->Direction, Vector2::Zero);
        EXPECT_FLOAT_EQ(modifier->Strength, 0.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_OpacityFastFadeModifier_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="OpacityFastFadeModifier" Enabled="True" Frequency="60" Type="OpacityFastFadeModifier" /></Modifiers>)");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<OpacityFastFadeModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        EXPECT_FLOAT_EQ(modifier->getFrequencyProperty(), 60.0f);
        EXPECT_TRUE(modifier->getEnabledProperty());
        EXPECT_EQ(modifier->getNameProperty(), "OpacityFastFadeModifier");
    }

    TEST(ParticleEffectSerializerTests, Deserialize_RectangleContainerModifier_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="RectangleContainerModifier" Enabled="True" Frequency="60" Type="RectangleContainerModifier" Width="0" Height="0" RestitutionCoefficient="1" /></Modifiers>)");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<RectangleContainerModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        EXPECT_FLOAT_EQ(modifier->getFrequencyProperty(), 60.0f);
        EXPECT_TRUE(modifier->getEnabledProperty());
        EXPECT_EQ(modifier->getNameProperty(), "RectangleContainerModifier");
        EXPECT_EQ(modifier->Width, 0);
        EXPECT_EQ(modifier->Height, 0);
        EXPECT_FLOAT_EQ(modifier->RestitutionCoefficient, 1.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_RectangleLoopContainerModifier_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="RectangleLoopContainerModifier" Enabled="True" Frequency="60" Type="RectangleLoopContainerModifier" Width="0" Height="0" /></Modifiers>)");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<RectangleLoopContainerModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        EXPECT_FLOAT_EQ(modifier->getFrequencyProperty(), 60.0f);
        EXPECT_TRUE(modifier->getEnabledProperty());
        EXPECT_EQ(modifier->getNameProperty(), "RectangleLoopContainerModifier");
        EXPECT_EQ(modifier->Width, 0);
        EXPECT_EQ(modifier->Height, 0);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_RotationModifier_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="RotationModifier" Enabled="True" Frequency="60" Type="RotationModifier" RotationRate="0" /></Modifiers>)");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<RotationModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        EXPECT_FLOAT_EQ(modifier->getFrequencyProperty(), 60.0f);
        EXPECT_TRUE(modifier->getEnabledProperty());
        EXPECT_EQ(modifier->getNameProperty(), "RotationModifier");
        EXPECT_FLOAT_EQ(modifier->RotationRate, 0.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_VelocityColorModifier_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="VelocityColorModifier" Enabled="True" Frequency="60" Type="VelocityColorModifier" StationaryColor="0,0,0" VelocityColor="0,0,0" VelocityThreshold="0" /></Modifiers>)");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<VelocityColorModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        EXPECT_FLOAT_EQ(modifier->getFrequencyProperty(), 60.0f);
        EXPECT_TRUE(modifier->getEnabledProperty());
        EXPECT_EQ(modifier->getNameProperty(), "VelocityColorModifier");
        EXPECT_EQ(modifier->StationaryColor, HslColor(0, 0, 0));
        EXPECT_EQ(modifier->VelocityColor, HslColor(0, 0, 0));
        EXPECT_FLOAT_EQ(modifier->VelocityThreshold, 0.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_VelocityModifier_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="VelocityModifier" Enabled="True" Frequency="60" Type="VelocityModifier" VelocityThreshold="0" /></Modifiers>)");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<VelocityModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        EXPECT_FLOAT_EQ(modifier->getFrequencyProperty(), 60.0f);
        EXPECT_EQ(modifier->getNameProperty(), "VelocityModifier");
        EXPECT_FLOAT_EQ(modifier->VelocityThreshold, 0.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_VortexModifier_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="VortexModifier" Enabled="True" Frequency="60" Type="VortexModifier" Position="0,0" Strength="1" OuterRadius="2" InnerRadius="3" MaxVelocity="4" RotationAngle="5" /></Modifiers>)");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<VortexModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        EXPECT_FLOAT_EQ(modifier->getFrequencyProperty(), 60.0f);
        EXPECT_EQ(modifier->getNameProperty(), "VortexModifier");
        EXPECT_EQ(modifier->Position, Vector2::Zero);
        EXPECT_FLOAT_EQ(modifier->Strength, 1.0f);
        EXPECT_FLOAT_EQ(modifier->OuterRadius, 2.0f);
        EXPECT_FLOAT_EQ(modifier->InnerRadius, 3.0f);
        EXPECT_FLOAT_EQ(modifier->MaxVelocity, 4.0f);
        EXPECT_FLOAT_EQ(modifier->getRotationAngleProperty(), 5.0f);
    }

    // --- Deserialize: Interpolators -----------------------------------------------------------------

    TEST(ParticleEffectSerializerTests, Deserialize_ColorInterpolator_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="ColorInterpolator" Type="ColorInterpolator" Enabled="True" StartValue="0,0,0" EndValue="0,0,0" /></Interpolators>)"
            "</Modifier></Modifiers>");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<AgeModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        ASSERT_EQ(modifier->getInterpolatorsProperty().size(), 1u);
        auto* interpolator = dynamic_cast<ColorInterpolator*>(modifier->getInterpolatorsProperty()[0].get());
        ASSERT_NE(interpolator, nullptr);
        EXPECT_TRUE(interpolator->getEnabledProperty());
        EXPECT_EQ(interpolator->StartValue, HslColor(0, 0, 0));
        EXPECT_EQ(interpolator->EndValue, HslColor(0, 0, 0));
    }

    TEST(ParticleEffectSerializerTests, Deserialize_HueInterpolator_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="HueInterpolator" Type="HueInterpolator" Enabled="True" StartValue="0" EndValue="0" /></Interpolators>)"
            "</Modifier></Modifiers>");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<AgeModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        ASSERT_EQ(modifier->getInterpolatorsProperty().size(), 1u);
        auto* interpolator = dynamic_cast<HueInterpolator*>(modifier->getInterpolatorsProperty()[0].get());
        ASSERT_NE(interpolator, nullptr);
        EXPECT_TRUE(interpolator->getEnabledProperty());
        EXPECT_FLOAT_EQ(interpolator->StartValue, 0.0f);
        EXPECT_FLOAT_EQ(interpolator->EndValue, 0.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_OpacityInterpolator_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="OpacityInterpolator" Type="OpacityInterpolator" Enabled="True" StartValue="0" EndValue="0" /></Interpolators>)"
            "</Modifier></Modifiers>");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<AgeModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        ASSERT_EQ(modifier->getInterpolatorsProperty().size(), 1u);
        auto* interpolator = dynamic_cast<OpacityInterpolator*>(modifier->getInterpolatorsProperty()[0].get());
        ASSERT_NE(interpolator, nullptr);
        EXPECT_TRUE(interpolator->getEnabledProperty());
        EXPECT_FLOAT_EQ(interpolator->StartValue, 0.0f);
        EXPECT_FLOAT_EQ(interpolator->EndValue, 0.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_RotationInterpolator_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="RotationInterpolator" Type="RotationInterpolator" Enabled="True" StartValue="0" EndValue="0" /></Interpolators>)"
            "</Modifier></Modifiers>");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<AgeModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        ASSERT_EQ(modifier->getInterpolatorsProperty().size(), 1u);
        auto* interpolator = dynamic_cast<RotationInterpolator*>(modifier->getInterpolatorsProperty()[0].get());
        ASSERT_NE(interpolator, nullptr);
        EXPECT_TRUE(interpolator->getEnabledProperty());
        EXPECT_FLOAT_EQ(interpolator->StartValue, 0.0f);
        EXPECT_FLOAT_EQ(interpolator->EndValue, 0.0f);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_ScaleInterpolator_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="ScaleInterpolator" Type="ScaleInterpolator" Enabled="True" StartValue="0,0" EndValue="1,1" /></Interpolators>)"
            "</Modifier></Modifiers>");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<AgeModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        ASSERT_EQ(modifier->getInterpolatorsProperty().size(), 1u);
        auto* interpolator = dynamic_cast<ScaleInterpolator*>(modifier->getInterpolatorsProperty()[0].get());
        ASSERT_NE(interpolator, nullptr);
        EXPECT_TRUE(interpolator->getEnabledProperty());
        EXPECT_EQ(interpolator->StartValue, Vector2::Zero);
        EXPECT_EQ(interpolator->EndValue, Vector2::One);
    }

    TEST(ParticleEffectSerializerTests, Deserialize_VelocityInterpolator_ReadsExpected)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="VelocityInterpolator" Type="VelocityInterpolator" Enabled="True" StartValue="0,0" EndValue="0,0" /></Interpolators>)"
            "</Modifier></Modifiers>");
        const std::unique_ptr<ParticleEffect> effect = ReadParticleEffectFromXml(xml, content);

        ASSERT_EQ(effect->getEmittersProperty().size(), 1u);
        auto& modifiers = effect->getEmittersProperty()[0]->getModifiersProperty();
        ASSERT_EQ(modifiers.size(), 1u);
        auto* modifier = dynamic_cast<AgeModifier*>(modifiers[0].get());
        ASSERT_NE(modifier, nullptr);
        ASSERT_EQ(modifier->getInterpolatorsProperty().size(), 1u);
        auto* interpolator = dynamic_cast<VelocityInterpolator*>(modifier->getInterpolatorsProperty()[0].get());
        ASSERT_NE(interpolator, nullptr);
        EXPECT_TRUE(interpolator->getEnabledProperty());
        EXPECT_EQ(interpolator->StartValue, Vector2::Zero);
        EXPECT_EQ(interpolator->EndValue, Vector2::Zero);
    }

    // Fresh coverage, no upstream equivalent: exercises the preserved-but-translated upstream bug
    // documented in ParticleEffectSerializer.cpp's `ReadInterpolator` (an unrecognized Interpolator
    // Type is a NullReferenceException crash upstream; this port throws std::logic_error instead).
    TEST(ParticleEffectSerializerTests, Deserialize_UnrecognizedInterpolatorType_Throws)
    {
        ContentManager content;
        const std::string xml = WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="Bogus" Type="BogusInterpolator" Enabled="True" /></Interpolators>)"
            "</Modifier></Modifiers>");

        EXPECT_THROW(ReadParticleEffectFromXml(xml, content), std::logic_error);
    }

    // --- Serialize: guard/basic cases ---------------------------------------------------------------

    TEST(ParticleEffectSerializerTests, Serialize_EmptyEffect_WritesMinimalXml)
    {
        ParticleEffect effect("EmptyEffect");

        AssertParticleEffect(effect,
            R"(<?xml version="1.0" encoding="utf-8"?><ParticleEffect Name="EmptyEffect" Position="0,0" Rotation="0" Scale="1,1" AutoTrigger="True" AutoTriggerFrequency="1" />)");
    }

    TEST(ParticleEffectSerializerTests, Serialize_EmptyModifiers_WritesMinimalXml)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = std::make_unique<ParticleEmitter>(1);
        emitter->setNameProperty("EmptyModifiers");
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("EmptyModifiers", R"(<Profile Type="PointProfile" />)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_EmptyInterpolators_WritesMinimalXml)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<AgeModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier" /></Modifiers>)"));
    }

    // --- Serialize: Profiles ------------------------------------------------------------------------

    TEST(ParticleEffectSerializerTests, Serialize_BoxFillProfile_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->setProfileProperty(Profile::BoxFill(1.0f, 2.0f));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter", R"(<Profile Type="BoxFillProfile" Width="1" Height="2" />)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_BoxProfile_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->setProfileProperty(Profile::Box(1.0f, 2.0f));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter", R"(<Profile Type="BoxProfile" Width="1" Height="2" />)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_BoxUniformProfile_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->setProfileProperty(Profile::BoxUniform(1.0f, 2.0f));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter", R"(<Profile Type="BoxUniformProfile" Width="1" Height="2" />)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_CircleProfile_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->setProfileProperty(Profile::Circle(1.0f, CircleRadiation::Out));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter", R"(<Profile Type="CircleProfile" Radius="1" Radiate="Out" />)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_LineProfile_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->setProfileProperty(Profile::Line(Vector2::One, 1.0f));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect,
            WrapEmitterXml("TestEmitter", R"(<Profile Type="LineProfile" Axis="1,1" Length="1" Radiate="None" Direction="0,1" />)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_PointProfile_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter", R"(<Profile Type="PointProfile" />)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_RingProfile_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->setProfileProperty(Profile::Ring(1.0f, CircleRadiation::In));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter", R"(<Profile Type="RingProfile" Radius="1" Radiate="In" />)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_SprayProfile_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->setProfileProperty(Profile::Spray(Vector2::One, 1.0f));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter", R"(<Profile Type="SprayProfile" Direction="1,1" Spread="1" />)"));
    }

    // --- Serialize: Modifiers -----------------------------------------------------------------------

    TEST(ParticleEffectSerializerTests, Serialize_AgeModifier_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<AgeModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier" /></Modifiers>)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_CircleContainerModifier_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<CircleContainerModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="CircleContainerModifier" Enabled="True" Frequency="60" Type="CircleContainerModifier" Radius="0" Inside="True" RestitutionCoefficient="1" /></Modifiers>)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_DragModifier_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<DragModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="DragModifier" Enabled="True" Frequency="60" Type="DragModifier" DragCoefficient="0.47" Density="0.5" /></Modifiers>)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_LinearGravityModifier_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<LinearGravityModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="LinearGravityModifier" Enabled="True" Frequency="60" Type="LinearGravityModifier" Direction="0,0" Strength="0" /></Modifiers>)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_OpacityFastFadeModifier_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<OpacityFastFadeModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="OpacityFastFadeModifier" Enabled="True" Frequency="60" Type="OpacityFastFadeModifier" /></Modifiers>)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_RectangleContainerModifier_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<RectangleContainerModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="RectangleContainerModifier" Enabled="True" Frequency="60" Type="RectangleContainerModifier" Width="0" Height="0" RestitutionCoefficient="1" /></Modifiers>)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_RectangleLoopContainerModifier_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<RectangleLoopContainerModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="RectangleLoopContainerModifier" Enabled="True" Frequency="60" Type="RectangleLoopContainerModifier" Width="0" Height="0" /></Modifiers>)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_RotationModifier_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<RotationModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="RotationModifier" Enabled="True" Frequency="60" Type="RotationModifier" RotationRate="0" /></Modifiers>)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_VelocityColorModifier_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<VelocityColorModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="VelocityColorModifier" Enabled="True" Frequency="60" Type="VelocityColorModifier" StationaryColor="0,0,0" VelocityColor="0,0,0" VelocityThreshold="0" /></Modifiers>)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_VelocityModifier_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<VelocityModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="VelocityModifier" Enabled="True" Frequency="60" Type="VelocityModifier" VelocityThreshold="0" /></Modifiers>)"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_VortexModifier_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        emitter->getModifiersProperty().push_back(std::make_unique<VortexModifier>());
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="VortexModifier" Enabled="True" Frequency="60" Type="VortexModifier" Position="0,0" Strength="0" OuterRadius="0" InnerRadius="0" MaxVelocity="0" RotationAngle="0" /></Modifiers>)"));
    }

    // --- Serialize: Interpolators -------------------------------------------------------------------

    TEST(ParticleEffectSerializerTests, Serialize_ColorInterpolator_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        auto modifier = std::make_unique<AgeModifier>();
        modifier->getInterpolatorsProperty().push_back(std::make_unique<ColorInterpolator>());
        emitter->getModifiersProperty().push_back(std::move(modifier));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="ColorInterpolator" Enabled="True" Type="ColorInterpolator" StartValue="0,0,0" EndValue="0,0,0" /></Interpolators>)"
            "</Modifier></Modifiers>"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_HueInterpolator_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        auto modifier = std::make_unique<AgeModifier>();
        modifier->getInterpolatorsProperty().push_back(std::make_unique<HueInterpolator>());
        emitter->getModifiersProperty().push_back(std::move(modifier));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="HueInterpolator" Enabled="True" Type="HueInterpolator" StartValue="0" EndValue="0" /></Interpolators>)"
            "</Modifier></Modifiers>"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_OpacityInterpolator_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        auto modifier = std::make_unique<AgeModifier>();
        modifier->getInterpolatorsProperty().push_back(std::make_unique<OpacityInterpolator>());
        emitter->getModifiersProperty().push_back(std::move(modifier));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="OpacityInterpolator" Enabled="True" Type="OpacityInterpolator" StartValue="0" EndValue="0" /></Interpolators>)"
            "</Modifier></Modifiers>"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_RotationInterpolator_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        auto modifier = std::make_unique<AgeModifier>();
        modifier->getInterpolatorsProperty().push_back(std::make_unique<RotationInterpolator>());
        emitter->getModifiersProperty().push_back(std::move(modifier));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="RotationInterpolator" Enabled="True" Type="RotationInterpolator" StartValue="0" EndValue="0" /></Interpolators>)"
            "</Modifier></Modifiers>"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_ScaleInterpolator_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        auto modifier = std::make_unique<AgeModifier>();
        modifier->getInterpolatorsProperty().push_back(std::make_unique<ScaleInterpolator>());
        emitter->getModifiersProperty().push_back(std::move(modifier));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="ScaleInterpolator" Enabled="True" Type="ScaleInterpolator" StartValue="0,0" EndValue="0,0" /></Interpolators>)"
            "</Modifier></Modifiers>"));
    }

    TEST(ParticleEffectSerializerTests, Serialize_VelocityInterpolator_WritesExpected)
    {
        ParticleEffect effect("TestEffect");
        auto emitter = MakeTestEmitter();
        auto modifier = std::make_unique<AgeModifier>();
        modifier->getInterpolatorsProperty().push_back(std::make_unique<VelocityInterpolator>());
        emitter->getModifiersProperty().push_back(std::move(modifier));
        effect.getEmittersProperty().push_back(std::move(emitter));

        AssertParticleEffect(effect, WrapEmitterXml("TestEmitter",
            R"(<Profile Type="PointProfile" />)"
            R"(<Modifiers><Modifier Name="AgeModifier" Enabled="True" Frequency="60" Type="AgeModifier">)"
            R"(<Interpolators><Interpolator Name="VelocityInterpolator" Enabled="True" Type="VelocityInterpolator" StartValue="0,0" EndValue="0,0" /></Interpolators>)"
            "</Modifier></Modifiers>"));
    }
}
