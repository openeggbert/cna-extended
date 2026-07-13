// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for these container modifiers. Fresh tests below, using the same
// InternalUpdate() harness pattern established in ../ModifierTests.cpp.
#include "CNA/Extended/Particles/Modifiers/Containers/CircleContainerModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/Containers/RectangleContainerModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/Containers/RectangleLoopContainerModifier.hpp"
#include "CNA/Extended/Particles/ParticleBuffer.hpp"

#include <functional>
#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Modifiers::Containers
{
    namespace
    {
        Data::Particle* ReleaseAndUpdate(ParticleBuffer& buffer, Modifier& modifier, const std::function<void(Data::Particle&)>& init)
        {
            ParticleIterator& releaseIterator = buffer.Release(1);
            Data::Particle* particle = releaseIterator.Next();
            init(*particle);

            ParticleIterator& updateIterator = buffer.getIteratorProperty();
            modifier.InternalUpdate(1.0f, updateIterator);
            return particle;
        }
    }

    TEST(CircleContainerModifierTests, ReflectsParticleThatExitsInsideBoundary)
    {
        ParticleBuffer buffer(1);
        CircleContainerModifier modifier;
        modifier.Radius = 10.0f;
        modifier.Inside = true;
        modifier.RestitutionCoefficient = 1.0f;

        const Data::Particle* particle = ReleaseAndUpdate(buffer, modifier, [](Data::Particle& p) {
            p.TriggeredPos[0] = 0.0f;
            p.TriggeredPos[1] = 0.0f;
            p.Position[0] = 20.0f; // well outside the radius-10 circle
            p.Position[1] = 0.0f;
            p.Velocity[0] = 5.0f;
            p.Velocity[1] = 0.0f;
        });

        // Reflected back toward the center: velocity along X should have flipped sign.
        EXPECT_LT(particle->Velocity[0], 0.0f);
    }

    TEST(RectangleContainerModifierTests, ReflectsParticleThatExitsLeftEdge)
    {
        ParticleBuffer buffer(1);
        RectangleContainerModifier modifier;
        modifier.Width = 20;
        modifier.Height = 20;
        modifier.RestitutionCoefficient = 1.0f;

        const Data::Particle* particle = ReleaseAndUpdate(buffer, modifier, [](Data::Particle& p) {
            p.TriggeredPos[0] = 0.0f;
            p.TriggeredPos[1] = 0.0f;
            p.Position[0] = -15.0f; // past the left edge at -10
            p.Position[1] = 0.0f;
            p.Velocity[0] = -5.0f;
            p.Velocity[1] = 0.0f;
        });

        EXPECT_GT(particle->Position[0], -15.0f);
        EXPECT_GT(particle->Velocity[0], 0.0f);
    }

    TEST(RectangleLoopContainerModifierTests, WrapsParticleToOppositeEdge)
    {
        ParticleBuffer buffer(1);
        RectangleLoopContainerModifier modifier;
        modifier.Width = 20;
        modifier.Height = 20;

        const Data::Particle* particle = ReleaseAndUpdate(buffer, modifier, [](Data::Particle& p) {
            p.TriggeredPos[0] = 0.0f;
            p.TriggeredPos[1] = 0.0f;
            p.Position[0] = -15.0f; // past the left edge at -10
            p.Position[1] = 0.0f;
        });

        EXPECT_FLOAT_EQ(particle->Position[0], -15.0f + 20.0f);
    }
}
