// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ParticleEmitter3DEXT.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Vector3;

    namespace
    {
        constexpr float kTwoPi = 6.28318530717958647692f;

        Color LerpColor(const Color& start, const Color& end, float t)
        {
            const auto lerpChannel = [&](std::uint8_t a, std::uint8_t b) {
                return static_cast<std::uint8_t>(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t);
            };
            return Color(lerpChannel(start.getRProperty(), end.getRProperty()), lerpChannel(start.getGProperty(), end.getGProperty()),
                         lerpChannel(start.getBProperty(), end.getBProperty()), static_cast<std::uint8_t>(255));
        }
    }

    Vector3 ParticleEmitter3DEXT::SampleConeDirectionEXT()
    {
        const float halfAngle = std::clamp(ConeHalfAngleEXT, 0.0f, 3.14159265358979323846f);
        const float minCosTheta = std::cos(halfAngle);
        const float cosTheta = minCosTheta + randomEXT_.NextSingle() * (1.0f - minCosTheta);
        const float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
        const float phi = randomEXT_.NextSingle() * kTwoPi;

        const Vector3 localDirection(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);

        const Vector3 forward = Vector3::Normalize(ConeDirectionEXT);
        const Vector3 arbitrary = std::abs(forward.Y) < 0.99f ? Vector3::Up : Vector3::Right;
        const Vector3 right = Vector3::Normalize(Vector3::Cross(arbitrary, forward));
        const Vector3 up = Vector3::Cross(forward, right);

        return right * localDirection.X + up * localDirection.Y + forward * localDirection.Z;
    }

    void ParticleEmitter3DEXT::EmitEXT(int count, const Vector3& origin)
    {
        for (int i = 0; i < count && static_cast<int>(ParticlesEXT.size()) < MaxParticlesEXT; ++i)
        {
            Particle3DEXT particle;
            particle.PositionEXT = origin;
            particle.LifetimeEXT = MinLifetimeEXT + randomEXT_.NextSingle() * (MaxLifetimeEXT - MinLifetimeEXT);
            const float speed = MinSpeedEXT + randomEXT_.NextSingle() * (MaxSpeedEXT - MinSpeedEXT);
            particle.VelocityEXT = SampleConeDirectionEXT() * speed;
            particle.ScaleEXT = MinScaleEXT + randomEXT_.NextSingle() * (MaxScaleEXT - MinScaleEXT);
            particle.ColorEXT = StartColorEXT;
            particle.OpacityEXT = StartOpacityEXT;
            ParticlesEXT.push_back(particle);
        }
    }

    void ParticleEmitter3DEXT::UpdateEXT(float deltaSeconds, const Vector3& origin)
    {
        for (Particle3DEXT& particle : ParticlesEXT)
        {
            particle.AgeEXT += deltaSeconds;
            particle.VelocityEXT = particle.VelocityEXT + GravityEXT * deltaSeconds;
            particle.PositionEXT = particle.PositionEXT + particle.VelocityEXT * deltaSeconds;

            const float ageRatio = particle.GetAgeRatioEXT();
            particle.ColorEXT = LerpColor(StartColorEXT, EndColorEXT, ageRatio);
            particle.OpacityEXT = StartOpacityEXT + (EndOpacityEXT - StartOpacityEXT) * ageRatio;
        }

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
