// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/ParticleEffect.cs. `List<ParticleEmitter> Emitters` ->
// owning `std::vector<std::unique_ptr<ParticleEmitter>>` (this effect disposes every emitter it
// owns in its own Dispose(), matching upstream's ownership).
//
// `FromFile`/`FromStream` both delegate entirely to `ParticleEffectSerializer::Deserialize(...)`.
// Declared here (matching upstream's static-factory-method placement on ParticleEffect itself)
// but implemented in ParticleEffect.cpp, not inline: `ParticleEffectSerializer.hpp` itself
// includes this header, so including it back here would be circular -- the .cpp has no such
// restriction. Return type is `std::unique_ptr<ParticleEffect>`, matching `Deserialize`'s own
// return type and this project's established "non-copyable/non-movable factory returns
// unique_ptr" convention (see ParticleEffectSerializer.hpp's own header comment).
#pragma once

#include "CNA/Extended/Particles/ParticleEmitter.hpp"
#include "CNA/Extended/Particles/Primitives/LineSegment.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/IDisposable.hpp"
#include "System/IO/Stream.hpp"

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::Particles
{
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Content::ContentManager;

    /**
     * @brief A complete particle effect: a positioned/rotated/scaled container of one or more
     * ParticleEmitters (e.g. an "explosion" effect might combine a fireball emitter, a smoke
     * emitter, and a spark emitter). Move/rotate/scale the whole effect via Position/Rotation/
     * Scale, Trigger() it to release particles, and call Update() once per frame to advance every
     * emitter's simulation.
     *
     * @see ParticleEmitter, the individual emission source this effect owns one or more of via
     * getEmittersProperty().
     * @see Particles::ParticleEffectSerializer::Deserialize for loading a ParticleEffect from an
     * XML particle-effect definition file instead of constructing one by hand.
     * @code
     * #include <CNA/Extended/Particles/ParticleEffect.hpp>
     * #include <CNA/Extended/Particles/ParticleEmitter.hpp>
     * #include <memory>
     *
     * using CNA::Extended::Particles::ParticleEffect;
     * using CNA::Extended::Particles::ParticleEmitter;
     * using Microsoft::Xna::Framework::Vector2;
     *
     * void SpawnExplosion(ParticleEffect& reusableEffect)
     * {
     *     if (reusableEffect.getEmittersProperty().empty())
     *     {
     *         reusableEffect.getEmittersProperty().push_back(std::make_unique<ParticleEmitter>(200));
     *     }
     *
     *     reusableEffect.Trigger(Vector2(100.0f, 100.0f));
     *     reusableEffect.Update(1.0f / 60.0f); // call once per frame from your game's Update()
     * }
     * @endcode
     */
    class ParticleEffect : public System::IDisposable
    {
    public:
        /** @param name The name of the effect, used for identification and debugging. */
        explicit ParticleEffect(std::string name);

        ~ParticleEffect() override;

        ParticleEffect(const ParticleEffect&) = delete;
        ParticleEffect& operator=(const ParticleEffect&) = delete;

        /**
         * @brief Creates a new ParticleEffect from an XML particle-effect definition file.
         * @throws System::ArgumentException @p path is empty.
         * @throws System::Xml::XmlException the XML format is invalid.
         */
        [[nodiscard]] static std::unique_ptr<ParticleEffect> FromFile(const std::string& path, ContentManager& content);

        /**
         * @brief Creates a new ParticleEffect from a stream containing an XML particle-effect definition.
         * @param stream The stream to read from.
         * @param content The ContentManager used to load referenced textures.
         * @param baseDirectory Base directory for resolving relative texture paths; if empty, uses @p content's RootDirectory.
         * @throws System::Xml::XmlException the XML format is invalid.
         */
        [[nodiscard]] static std::unique_ptr<ParticleEffect> FromStream(System::IO::Stream& stream, ContentManager& content, const std::string& baseDirectory = "");

        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        void setNameProperty(const std::string& value) { name_ = value; }

        [[nodiscard]] const Vector2& getPositionProperty() const { return position_; }
        void setPositionProperty(const Vector2& value) { position_ = value; }

        [[nodiscard]] float getRotationProperty() const { return rotation_; }
        void setRotationProperty(float value) { rotation_ = value; }

        [[nodiscard]] const Vector2& getScaleProperty() const { return scale_; }
        void setScaleProperty(const Vector2& value) { scale_ = value; }

        /** @brief Gets or sets whether all emitters are automatically triggered every AutoTriggerFrequency seconds. */
        [[nodiscard]] bool getAutoTriggerProperty() const { return autoTrigger_; }
        void setAutoTriggerProperty(bool value) { autoTrigger_ = value; }

        [[nodiscard]] float getAutoTriggerFrequencyProperty() const { return autoTriggerFrequency_; }
        void setAutoTriggerFrequencyProperty(float value) { autoTriggerFrequency_ = value; }

        /** @brief Gets the mutable list of emitters composing this effect. */
        [[nodiscard]] std::vector<std::unique_ptr<ParticleEmitter>>& getEmittersProperty() { return emitters_; }

        [[nodiscard]] bool getIsDisposedProperty() const { return isDisposed_; }

        /** @brief Gets the sum of ActiveParticles across every emitter in this effect. */
        [[nodiscard]] int getActiveParticlesProperty() const;

        /** @brief Repeatedly Update()s (by triggerPeriod) then Trigger()s at @p position until @p seconds have been simulated. */
        void FastForward(const Vector2& position, float seconds, float triggerPeriod);

        /** @throws System::ObjectDisposedException this effect was already disposed. */
        void Update(const GameTime& gameTime);
        /** @throws System::ObjectDisposedException this effect was already disposed. */
        void Update(float elapsedSeconds);

        /** @brief Triggers all emitters at this effect's current Position. */
        void Trigger();

        /** @throws System::ObjectDisposedException this effect was already disposed. */
        void Trigger(const Vector2& position, float layerDepth = 0.0f);

        /** @throws System::ObjectDisposedException this effect was already disposed. */
        void Trigger(const Primitives::LineSegment& line, float layerDepth);

        void Dispose() override;

        [[nodiscard]] std::string ToString() const { return name_; }

    private:
        std::string name_;
        Vector2 position_;
        float rotation_ = 0.0f;
        Vector2 scale_;
        bool autoTrigger_ = true;
        float autoTriggerFrequency_ = 1.0f;
        std::vector<std::unique_ptr<ParticleEmitter>> emitters_;
        bool isDisposed_ = false;
        float nextAutoTrigger_ = 0.0f;
    };
}
