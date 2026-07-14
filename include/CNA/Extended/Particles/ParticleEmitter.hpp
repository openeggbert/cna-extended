// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/ParticleEmitter.cs. `Profile` (a C# reference type,
// always non-null after construction) -> owning `std::unique_ptr<Profiles::Profile>` since
// `Profile.Point()` etc. return fresh instances upstream doesn't alias elsewhere; getter returns a
// reference (never null after construction). `TextureRegion` -> `std::shared_ptr<Texture2DRegion>`,
// matching that type's established shared-ownership convention (nullable, matching upstream's
// `TextureRegion = null` default). `List<Modifier> Modifiers` -> owning
// `std::vector<std::unique_ptr<Modifiers::Modifier>>` (nothing else upstream aliases individual
// Modifier instances); `ModifierExecutionStrategy::ExecuteModifiers` needs a raw-pointer view,
// built fresh each `Update()` call. `ModifierExecutionStrategy` field -> non-owning pointer into
// the Serial/Parallel Meyer's singletons (never owned/destroyed by the emitter).
//
// `ownedTexture_`/`AdoptOwnedTexture` (added when porting `ParticleEffectSerializer.cs`, no direct
// upstream field): in C#, a `Texture2D` loaded via `ContentManager.Load<Texture2D>(path)` for this
// emitter's `TextureRegion` is a GC-managed object kept alive purely by the `Texture2DRegion`
// referencing it. `ContentManager::Load<T>()` here returns *by value*, and `Texture2DRegion` holds
// only a non-owning `Texture2D*` (matching its established externally-owned-GPU-resource
// convention) -- so something has to actually own a texture loaded specifically for this emitter's
// region. `ParticleEmitter` is that owner, mirroring `Tilemap::ownedTextures_`/
// `BitmapFont::pageTextures_`'s identical problem and solution -- at most one texture per emitter
// here (one `TextureRegion` per emitter), so a single `unique_ptr` rather than a vector.
// `Texture2D.hpp` is included in full here (not just forward-declared, unlike `Texture2DRegion.hpp`'s
// established non-owning-pointer convention): `AdoptOwnedTexture` is an ordinary (non-template)
// inline member function whose body move-assigns `unique_ptr<Texture2D>`, and unlike a template,
// its body is compiled wherever this class definition is parsed -- so every consumer of this header
// needs `Texture2D` complete right here, not just in `ParticleEmitter.cpp`.
#pragma once

#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/Data/ParticleReleaseParameters.hpp"
#include "CNA/Extended/Particles/Modifiers/ModifierExecutionStrategy.hpp"
#include "CNA/Extended/Particles/ParticleBuffer.hpp"
#include "CNA/Extended/Particles/ParticleRenderingOrder.hpp"
#include "CNA/Extended/Particles/Primitives/LineSegment.hpp"
#include "CNA/Extended/Particles/Profiles/Profile.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/IDisposable.hpp"

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::Particles
{
    using Microsoft::Xna::Framework::Vector2;

    /**
     * @brief Creates, updates, and manages the particles emitted by one emission source within a
     * particle effect. An emitter's Profile controls where new particles start (point, line,
     * circle, box, ...), Parameters controls their initial values (velocity, color, scale,
     * lifetime, ...), and Modifiers apply per-frame forces/interpolation (gravity, drag, color-
     * over-time, ...) to particles already in flight.
     *
     * @see ParticleEffect, which owns and drives one or more emitters as a single logical effect.
     * @see Profiles::Profile for the available emission-shape factories (Profile::Point,
     * Profile::Line, Profile::Circle, ...).
     * @see Modifiers::Modifier, the base type for entries in getModifiersProperty()'s list.
     */
    class ParticleEmitter : public System::IDisposable
    {
    public:
        /** @brief Creates an emitter with a capacity of 1000 particles and default settings. */
        ParticleEmitter();

        /** @param initialCapacity The maximum number of particles this emitter can manage. */
        explicit ParticleEmitter(int initialCapacity);

        ~ParticleEmitter() override;

        ParticleEmitter(const ParticleEmitter&) = delete;
        ParticleEmitter& operator=(const ParticleEmitter&) = delete;

        /** @brief Gets the buffer storing this emitter's particles. */
        [[nodiscard]] ParticleBuffer& getBufferProperty() { return *buffer_; }
        [[nodiscard]] const ParticleBuffer& getBufferProperty() const { return *buffer_; }

        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        void setNameProperty(const std::string& value) { name_ = value; }

        /** @brief Gets the maximum number of particles this emitter can manage. */
        [[nodiscard]] int getCapacityProperty() const { return buffer_->getSizeProperty(); }

        /** @brief Gets the current number of active particles. */
        [[nodiscard]] int getActiveParticlesProperty() const { return buffer_->getCountProperty(); }

        [[nodiscard]] float getLifeSpanProperty() const { return lifeSpan_; }
        void setLifeSpanProperty(float value) { lifeSpan_ = value; }

        [[nodiscard]] const Vector2& getOffsetProperty() const { return offset_; }
        void setOffsetProperty(const Vector2& value) { offset_ = value; }

        [[nodiscard]] float getLayerDepthProperty() const { return layerDepth_; }
        void setLayerDepthProperty(float value) { layerDepth_ = value; }

        [[nodiscard]] float getReclaimFrequencyProperty() const { return reclaimFrequency_; }
        void setReclaimFrequencyProperty(float value) { reclaimFrequency_ = value; }

        [[nodiscard]] Data::ParticleReleaseParameters& getParametersProperty() { return parameters_; }
        void setParametersProperty(Data::ParticleReleaseParameters value) { parameters_ = std::move(value); }

        [[nodiscard]] Modifiers::ModifierExecutionStrategy& getModifierExecutionStrategyProperty() const { return *modifierExecutionStrategy_; }
        void setModifierExecutionStrategyProperty(Modifiers::ModifierExecutionStrategy& value) { modifierExecutionStrategy_ = &value; }

        /** @brief Gets the mutable list of modifiers applied to this emitter's particles. */
        [[nodiscard]] std::vector<std::unique_ptr<Modifiers::Modifier>>& getModifiersProperty() { return modifiers_; }

        [[nodiscard]] Profiles::Profile& getProfileProperty() { return *profile_; }
        void setProfileProperty(std::unique_ptr<Profiles::Profile> value) { profile_ = std::move(value); }

        [[nodiscard]] const std::shared_ptr<Graphics::Texture2DRegion>& getTextureRegionProperty() const { return textureRegion_; }
        void setTextureRegionProperty(std::shared_ptr<Graphics::Texture2DRegion> value) { textureRegion_ = std::move(value); }

        /** @brief Gives this emitter ownership of @p texture, keeping it alive for as long as the emitter is (see header comment). */
        void AdoptOwnedTexture(std::unique_ptr<Graphics::Texture2D> texture) { ownedTexture_ = std::move(texture); }

        [[nodiscard]] ParticleRenderingOrder getRenderingOrderProperty() const { return renderingOrder_; }
        void setRenderingOrderProperty(ParticleRenderingOrder value) { renderingOrder_ = value; }

        [[nodiscard]] bool getVisibleProperty() const { return visible_; }
        void setVisibleProperty(bool value) { visible_ = value; }

        [[nodiscard]] bool getIsDisposedProperty() const { return isDisposed_; }

        /** @brief Disposes the old buffer and replaces it with a new one of @p size capacity. Existing particles are lost. */
        void ChangeCapacity(int size);

        /** @brief Advances particle ages/positions, applies modifiers, and reclaims expired particles. */
        void Update(float elapsedSeconds, const Vector2& position = Vector2());

        /** @brief Releases Parameters.Quantity particles at @p position. */
        void Trigger(const Vector2& position, float layerDepth = 0.0f);

        /** @brief Releases Parameters.Quantity particles at random positions along @p line. */
        void Trigger(const Primitives::LineSegment& line, float layerDepth = 0.0f);

        [[nodiscard]] std::string ToString() const { return name_; }

        void Dispose() override;

    private:
        void Release(const Vector2& position, int numToRelease, float layerDepth);
        void ReclaimExpiredParticles();

        float totalSeconds_ = 0.0f;
        float secondsSinceLastReclaim_ = 0.0f;
        std::unique_ptr<ParticleBuffer> buffer_;
        std::string name_ = "ParticleEmitter";
        float lifeSpan_ = 1.0f;
        Vector2 offset_;
        float layerDepth_ = 0.0f;
        float reclaimFrequency_ = 60.0f;
        Data::ParticleReleaseParameters parameters_;
        Modifiers::ModifierExecutionStrategy* modifierExecutionStrategy_;
        std::vector<std::unique_ptr<Modifiers::Modifier>> modifiers_;
        std::unique_ptr<Profiles::Profile> profile_;
        std::shared_ptr<Graphics::Texture2DRegion> textureRegion_;
        std::unique_ptr<Graphics::Texture2D> ownedTexture_;
        ParticleRenderingOrder renderingOrder_ = ParticleRenderingOrder::FrontToBack;
        bool visible_ = true;
        bool isDisposed_ = false;
    };
}
