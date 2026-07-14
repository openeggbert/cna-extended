// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ParticleEmitter3DEXT.hpp"

#include "CNA/Extended/World3DEXT/AgeModifier3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ColorInterpolator3DEXT.hpp"
#include "CNA/Extended/World3DEXT/LinearGravityModifier3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ModifierExecutionStrategy3DEXT.hpp"
#include "CNA/Extended/World3DEXT/OpacityInterpolator3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;

    ParticleEmitter3DEXT::ParticleEmitter3DEXT()
        : profileEXT_(Profile3DEXT::Cone(Vector3::Up, 3.14159265358979323846f)),
          modifierExecutionStrategyEXT_(&ModifierExecutionStrategy3DEXT::getSerialProperty())
    {
        // Default pipeline: matches the exact behavior ParticleEmitter3DEXT had before this
        // architecture existed (cone/full-sphere emission, linear gravity, color+opacity
        // interpolation over the particle's lifetime) -- see this file's header comment for
        // why StartColorEXT/EndColorEXT/StartOpacityEXT/EndOpacityEXT/GravityEXT stayed on
        // this class instead of moving onto these owned objects.
        //
        // kUnthrottledFrequencyEXT (Modifier3DEXT.hpp's header comment has the full
        // rationale): these two built-in modifiers deliberately override Frequency to an
        // effectively-unthrottled value so every particle is touched every UpdateEXT call,
        // regardless of deltaSeconds -- preserving this emitter's pre-existing "every
        // particle, every call, real elapsed time" behavior exactly, rather than 2D's own
        // default 60Hz pacing (which would silently apply a stale, much smaller time delta
        // to a large single UpdateEXT(dt) call).
        constexpr float kUnthrottledFrequencyEXT = 1.0e6f;

        auto ageModifier = std::make_unique<AgeModifier3DEXT>();
        defaultAgeModifierEXT_ = ageModifier.get();
        ageModifier->setFrequencyProperty(kUnthrottledFrequencyEXT);
        ageModifier->getInterpolatorsProperty().push_back(std::make_unique<ColorInterpolator3DEXT>());
        ageModifier->getInterpolatorsProperty().push_back(std::make_unique<OpacityInterpolator3DEXT>());

        auto gravityModifier = std::make_unique<LinearGravityModifier3DEXT>();
        defaultGravityModifierEXT_ = gravityModifier.get();
        gravityModifier->setFrequencyProperty(kUnthrottledFrequencyEXT);

        modifiersEXT_.push_back(std::move(ageModifier));
        modifiersEXT_.push_back(std::move(gravityModifier));
    }

    ParticleEmitter3DEXT::~ParticleEmitter3DEXT() = default;
    ParticleEmitter3DEXT::ParticleEmitter3DEXT(ParticleEmitter3DEXT&&) noexcept = default;
    ParticleEmitter3DEXT& ParticleEmitter3DEXT::operator=(ParticleEmitter3DEXT&&) noexcept = default;

    void ParticleEmitter3DEXT::setProfileEXTProperty(std::unique_ptr<Profile3DEXT> profile)
    {
        if (profile)
        {
            profileEXT_ = std::move(profile);
        }
    }

    void ParticleEmitter3DEXT::SyncBuiltInModifiersEXT()
    {
        for (const std::unique_ptr<Interpolator3DEXT>& interpolator : defaultAgeModifierEXT_->getInterpolatorsProperty())
        {
            if (auto* colorInterpolator = dynamic_cast<ColorInterpolator3DEXT*>(interpolator.get()))
            {
                colorInterpolator->StartValue = StartColorEXT;
                colorInterpolator->EndValue = EndColorEXT;
            }
            else if (auto* opacityInterpolator = dynamic_cast<OpacityInterpolator3DEXT*>(interpolator.get()))
            {
                opacityInterpolator->StartValue = StartOpacityEXT;
                opacityInterpolator->EndValue = EndOpacityEXT;
            }
        }

        defaultGravityModifierEXT_->GravityEXT = GravityEXT;
    }

    void ParticleEmitter3DEXT::EmitEXT(int count, const Vector3& origin)
    {
        for (int i = 0; i < count && static_cast<int>(ParticlesEXT.size()) < MaxParticlesEXT; ++i)
        {
            // Note on "reversed" ranges (Min > Max): not a bug -- `min + t*(max-min)` for t in
            // [0,1) already lands in [min,max] (inclusive/exclusive as appropriate) regardless
            // of which of the two is larger, since a negative (max-min) just reverses the
            // interpolation direction. The real hazard is non-finite (NaN/Infinity) inputs, not
            // reversed order: an infinite or NaN LifetimeEXT makes IsExpiredEXT()'s
            // `AgeEXT >= LifetimeEXT` false forever (NaN compares false against everything in
            // IEEE 754), leaking the particle permanently instead of ever removing it. Guarded
            // below by falling back to the "already expired" value (0) for the actually-broken
            // (non-finite) case only, not by forcing Min <= Max.
            Particle3DEXT particle;

            Vector3 offset;
            Vector3 heading;
            profileEXT_->GetOffsetAndHeading(&offset, &heading, randomEXT_);
            particle.PositionEXT = origin + offset;
            particle.TriggeredPositionEXT = origin;

            particle.LifetimeEXT = MinLifetimeEXT + randomEXT_.NextSingle() * (MaxLifetimeEXT - MinLifetimeEXT);
            if (!std::isfinite(particle.LifetimeEXT))
            {
                particle.LifetimeEXT = 0.0f;
            }
            float speed = MinSpeedEXT + randomEXT_.NextSingle() * (MaxSpeedEXT - MinSpeedEXT);
            if (!std::isfinite(speed))
            {
                speed = 0.0f;
            }
            particle.VelocityEXT = heading * speed;

            float scale = MinScaleEXT + randomEXT_.NextSingle() * (MaxScaleEXT - MinScaleEXT);
            if (!std::isfinite(scale))
            {
                scale = 0.0f;
            }
            particle.ScaleEXT = Vector2(scale, scale);

            particle.ColorEXT = StartColorEXT;
            // A-06: clamp here too, not just in UpdateEXT's per-frame recompute below --
            // EmitEXT runs at the end of this same UpdateEXT call, so a newly-emitted
            // particle wouldn't otherwise get its opacity clamped until next frame.
            particle.OpacityEXT = std::clamp(StartOpacityEXT, 0.0f, 1.0f);
            ParticlesEXT.push_back(particle);
        }
    }

    void ParticleEmitter3DEXT::UpdateEXT(float deltaSeconds, const Vector3& origin)
    {
        // Baked directly, matching 2D's own ParticleEmitter::Update precedent (see
        // AgeModifier3DEXT.hpp) -- not modifier-driven.
        for (Particle3DEXT& particle : ParticlesEXT)
        {
            particle.AgeEXT += deltaSeconds;
            particle.PositionEXT = particle.PositionEXT + particle.VelocityEXT * deltaSeconds;
        }

        SyncBuiltInModifiersEXT();

        std::vector<Modifier3DEXT*> modifierPointers;
        modifierPointers.reserve(modifiersEXT_.size());
        for (const std::unique_ptr<Modifier3DEXT>& modifier : modifiersEXT_)
        {
            modifierPointers.push_back(modifier.get());
        }
        modifierExecutionStrategyEXT_->ExecuteModifiers(modifierPointers, deltaSeconds, ParticlesEXT);

        ParticlesEXT.erase(std::remove_if(ParticlesEXT.begin(), ParticlesEXT.end(),
                                           [](const Particle3DEXT& particle) { return particle.IsExpiredEXT(); }),
                            ParticlesEXT.end());

        if (IsEmittingEXT && EmissionRateEXT > 0.0f)
        {
            emitAccumulatorEXT_ += EmissionRateEXT * deltaSeconds;
            const int count = static_cast<int>(emitAccumulatorEXT_);
            if (count > 0)
            {
                emitAccumulatorEXT_ -= static_cast<float>(count);
                EmitEXT(count, origin);
            }
        }
    }
}
