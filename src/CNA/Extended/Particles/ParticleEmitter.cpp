// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/ParticleEmitter.hpp"

#include "CNA/Extended/FastRandom.hpp"
#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ObjectDisposedException.hpp"

namespace CNA::Extended::Particles
{
    using Microsoft::Xna::Framework::Vector3;

    ParticleEmitter::ParticleEmitter() : ParticleEmitter(1000)
    {
    }

    ParticleEmitter::ParticleEmitter(int initialCapacity)
        : buffer_(std::make_unique<ParticleBuffer>(initialCapacity)),
          modifierExecutionStrategy_(&Modifiers::ModifierExecutionStrategy::getSerialProperty()),
          profile_(Profiles::Profile::Point())
    {
    }

    ParticleEmitter::~ParticleEmitter()
    {
        Dispose();
    }

    void ParticleEmitter::ChangeCapacity(int size)
    {
        System::ObjectDisposedException::ThrowIf(isDisposed_, "ParticleBuffer");

        if (getCapacityProperty() == size)
        {
            return;
        }

        buffer_ = std::make_unique<ParticleBuffer>(size);
    }

    void ParticleEmitter::Update(float elapsedSeconds, const Vector2& position)
    {
        (void)position;
        System::ObjectDisposedException::ThrowIf(isDisposed_, "ParticleBuffer");

        totalSeconds_ += elapsedSeconds;
        secondsSinceLastReclaim_ += elapsedSeconds;

        if (buffer_->getCountProperty() == 0)
        {
            return;
        }

        if (secondsSinceLastReclaim_ > (1.0f / reclaimFrequency_))
        {
            ReclaimExpiredParticles();
            secondsSinceLastReclaim_ -= (1.0f / reclaimFrequency_);
        }

        if (buffer_->getCountProperty() > 0)
        {
            ParticleIterator& iterator = buffer_->getIteratorProperty();
            while (iterator.getHasNextProperty())
            {
                Data::Particle* particle = iterator.Next();
                particle->Age = (totalSeconds_ - particle->Inception) / lifeSpan_;
                particle->Position[0] += particle->Velocity[0] * elapsedSeconds;
                particle->Position[1] += particle->Velocity[1] * elapsedSeconds;
            }

            std::vector<Modifiers::Modifier*> modifierPointers;
            modifierPointers.reserve(modifiers_.size());
            for (const std::unique_ptr<Modifiers::Modifier>& modifier : modifiers_)
            {
                modifierPointers.push_back(modifier.get());
            }
            modifierExecutionStrategy_->ExecuteModifiers(modifierPointers, elapsedSeconds, iterator);
        }
    }

    void ParticleEmitter::Trigger(const Vector2& position, float layerDepth)
    {
        const int numToRelease = parameters_.Quantity.getValueProperty();
        Release(position, numToRelease, layerDepth);
    }

    void ParticleEmitter::Trigger(const Primitives::LineSegment& line, float layerDepth)
    {
        const int numToRelease = parameters_.Quantity.getValueProperty();
        const Vector2 lineVector = line.ToVector2();

        for (int i = 0; i < numToRelease; ++i)
        {
            const Vector2 offset = lineVector * FastRandom::getSharedProperty().NextSingle();
            Release(line.getOriginProperty() + offset, 1, layerDepth);
        }
    }

    void ParticleEmitter::Release(const Vector2& position, int numToRelease, float layerDepth)
    {
        ParticleIterator& iterator = buffer_->Release(numToRelease);

        while (iterator.getHasNextProperty())
        {
            Data::Particle* particle = iterator.Next();

            profile_->GetOffsetAndHeading(reinterpret_cast<Vector2*>(particle->Position), reinterpret_cast<Vector2*>(particle->Velocity));

            particle->Age = 0.0f;
            particle->Inception = totalSeconds_;

            particle->Position[0] += position.X;
            particle->Position[1] += position.Y;

            particle->TriggeredPos[0] = position.X;
            particle->TriggeredPos[1] = position.Y;

            const float speed = parameters_.Speed.getValueProperty();

            particle->Velocity[0] *= speed;
            particle->Velocity[1] *= speed;

            const Vector3 color = parameters_.Color.getValueProperty();
            particle->Color[0] = color.X;
            particle->Color[1] = color.Y;
            particle->Color[2] = color.Z;

            particle->Opacity = parameters_.Opacity.getValueProperty();

            const Vector2 scale = parameters_.Scale.getValueProperty();
            particle->Scale[0] = scale.X;
            particle->Scale[1] = scale.Y;

            particle->Rotation = parameters_.Rotation.getValueProperty();
            particle->Mass = parameters_.Mass.getValueProperty();
            particle->LayerDepth = layerDepth;
        }
    }

    void ParticleEmitter::ReclaimExpiredParticles()
    {
        int expired = 0;
        ParticleIterator& iterator = buffer_->getIteratorProperty();
        while (iterator.getHasNextProperty())
        {
            Data::Particle* particle = iterator.Next();

            if ((totalSeconds_ - particle->Inception) < lifeSpan_)
            {
                break;
            }
            ++expired;
        }

        if (expired != 0)
        {
            buffer_->Reclaim(expired);
        }
    }

    void ParticleEmitter::Dispose()
    {
        if (isDisposed_)
        {
            return;
        }

        buffer_->Dispose();
        isDisposed_ = true;
    }
}
