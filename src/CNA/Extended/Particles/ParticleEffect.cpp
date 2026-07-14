// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/ParticleEffect.hpp"

#include "CNA/Extended/Particles/ParticleEffectSerializer.hpp"
#include "System/ObjectDisposedException.hpp"

namespace CNA::Extended::Particles
{
    ParticleEffect::ParticleEffect(std::string name)
        : name_(std::move(name)), scale_(Vector2::One)
    {
    }

    ParticleEffect::~ParticleEffect()
    {
        Dispose();
    }

    std::unique_ptr<ParticleEffect> ParticleEffect::FromFile(const std::string& path, ContentManager& content)
    {
        return ParticleEffectSerializer::Deserialize(path, content);
    }

    std::unique_ptr<ParticleEffect> ParticleEffect::FromStream(System::IO::Stream& stream, ContentManager& content, const std::string& baseDirectory)
    {
        return ParticleEffectSerializer::Deserialize(stream, content, baseDirectory);
    }

    int ParticleEffect::getActiveParticlesProperty() const
    {
        int total = 0;
        for (const std::unique_ptr<ParticleEmitter>& emitter : emitters_)
        {
            total += emitter->getActiveParticlesProperty();
        }
        return total;
    }

    void ParticleEffect::FastForward(const Vector2& position, float seconds, float triggerPeriod)
    {
        float time = 0.0f;
        while (time < seconds)
        {
            Update(triggerPeriod);
            Trigger(position);
            time += triggerPeriod;
        }
    }

    void ParticleEffect::Update(const GameTime& gameTime)
    {
        Update(static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty()));
    }

    void ParticleEffect::Update(float elapsedSeconds)
    {
        System::ObjectDisposedException::ThrowIf(isDisposed_, "ParticleBuffer");

        if (autoTrigger_)
        {
            nextAutoTrigger_ -= elapsedSeconds;

            if (nextAutoTrigger_ <= 0.0f)
            {
                Trigger();
                nextAutoTrigger_ += autoTriggerFrequency_;
            }
        }

        for (const std::unique_ptr<ParticleEmitter>& emitter : emitters_)
        {
            emitter->Update(elapsedSeconds, position_);
        }
    }

    void ParticleEffect::Trigger()
    {
        Trigger(position_);
    }

    void ParticleEffect::Trigger(const Vector2& position, float layerDepth)
    {
        System::ObjectDisposedException::ThrowIf(isDisposed_, "ParticleBuffer");

        for (const std::unique_ptr<ParticleEmitter>& emitter : emitters_)
        {
            emitter->Trigger(position, layerDepth);
        }
    }

    void ParticleEffect::Trigger(const Primitives::LineSegment& line, float layerDepth)
    {
        System::ObjectDisposedException::ThrowIf(isDisposed_, "ParticleBuffer");

        for (const std::unique_ptr<ParticleEmitter>& emitter : emitters_)
        {
            emitter->Trigger(line, layerDepth);
        }
    }

    void ParticleEffect::Dispose()
    {
        if (isDisposed_)
        {
            return;
        }

        for (const std::unique_ptr<ParticleEmitter>& emitter : emitters_)
        {
            emitter->Dispose();
        }

        isDisposed_ = true;
    }
}
