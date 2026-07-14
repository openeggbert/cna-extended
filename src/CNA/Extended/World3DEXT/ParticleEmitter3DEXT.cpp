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

        // Audit finding A-06 (audit.md), independently re-verified: Vector3::Normalize of a
        // zero (or near-zero) vector is undefined/NaN, and ConeDirectionEXT is a public,
        // caller-settable field with no constructor-level guarantee it stays nonzero. Fall
        // back to the field's own documented default (Vector3::Up) rather than propagate NaN
        // into every sampled particle direction.
        const Vector3 forward = ConeDirectionEXT.LengthSquared() > 1e-12f ? Vector3::Normalize(ConeDirectionEXT) : Vector3::Up;
        const Vector3 arbitrary = std::abs(forward.Y) < 0.99f ? Vector3::Up : Vector3::Right;
        const Vector3 right = Vector3::Normalize(Vector3::Cross(arbitrary, forward));
        const Vector3 up = Vector3::Cross(forward, right);

        return right * localDirection.X + up * localDirection.Y + forward * localDirection.Z;
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
            particle.PositionEXT = origin;
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
            particle.VelocityEXT = SampleConeDirectionEXT() * speed;
            particle.ScaleEXT = MinScaleEXT + randomEXT_.NextSingle() * (MaxScaleEXT - MinScaleEXT);
            if (!std::isfinite(particle.ScaleEXT))
            {
                particle.ScaleEXT = 0.0f;
            }
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
        for (Particle3DEXT& particle : ParticlesEXT)
        {
            particle.AgeEXT += deltaSeconds;
            particle.VelocityEXT = particle.VelocityEXT + GravityEXT * deltaSeconds;
            particle.PositionEXT = particle.PositionEXT + particle.VelocityEXT * deltaSeconds;

            const float ageRatio = particle.GetAgeRatioEXT();
            particle.ColorEXT = LerpColor(StartColorEXT, EndColorEXT, ageRatio);
            // A-06 (audit.md): out-of-[0,1] opacity is real UB at the render call site's
            // `static_cast<std::uint8_t>(opacity * 255.0f)` (ParticleRenderSystem3DEXT.cpp),
            // not just an implausible value -- clamp where opacity is actually computed.
            particle.OpacityEXT = std::clamp(StartOpacityEXT + (EndOpacityEXT - StartOpacityEXT) * ageRatio, 0.0f, 1.0f);
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
