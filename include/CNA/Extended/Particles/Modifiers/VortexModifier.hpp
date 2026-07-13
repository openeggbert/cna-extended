// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/VortexModifier.cs.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended::Particles::Modifiers
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief Applies a rotated gravitational force around a center point, creating spiral/orbital/vortex particle motion. */
    class VortexModifier final : public Modifier
    {
    public:
        Vector2 Position = Vector2::Zero;
        float Strength = 0.0f;
        float OuterRadius = 0.0f;
        float InnerRadius = 0.0f;
        float MaxVelocity = 0.0f;

        VortexModifier();

        /** @brief Gets the rotation (radians) applied to the gravitational force vector: 0 is pure attraction, larger values spiral/orbit. */
        [[nodiscard]] float getRotationAngleProperty() const { return rotationAngle_; }
        /** @brief Sets RotationAngle, recomputing the cached cos/sin used each Update() so it isn't recomputed per particle. */
        void setRotationAngleProperty(float value);

    protected:
        void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) override;

    private:
        float rotationAngle_ = 0.0f;
        float cosAngle_ = 1.0f;
        float sinAngle_ = 0.0f;
    };
}
