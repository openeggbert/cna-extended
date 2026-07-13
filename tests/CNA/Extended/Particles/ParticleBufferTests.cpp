// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/Particles/ParticleBufferTests.cs.
// Upstream's own file is entirely commented out (dead/stale) -- its CopyTo/CopyToReverse sections
// test methods that no longer exist on the current ParticleBuffer.cs at all, confirming the file
// predates the class's current shape. The Available/Count/Release/Reclaim/Dispose sections still
// match the current API exactly, so those are ported (adapted to GoogleTest); CopyTo/CopyToReverse
// are not, since there is nothing current to port them against.
#include "CNA/Extended/Particles/ParticleBuffer.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Particles
{
    TEST(ParticleBufferTests, AvailableWhenNoParticlesReleasedReturnsBufferSize)
    {
        ParticleBuffer subject(100);
        EXPECT_EQ(subject.getAvailableProperty(), 100);
    }

    TEST(ParticleBufferTests, AvailableWhenSomeParticlesReleasedReturnsAvailableCount)
    {
        ParticleBuffer subject(100);
        subject.Release(10);
        EXPECT_EQ(subject.getAvailableProperty(), 90);
    }

    TEST(ParticleBufferTests, AvailableWhenAllParticlesReleasedReturnsZero)
    {
        ParticleBuffer subject(100);
        subject.Release(100);
        EXPECT_EQ(subject.getAvailableProperty(), 0);
    }

    TEST(ParticleBufferTests, CountWhenNoParticlesReleasedReturnsZero)
    {
        ParticleBuffer subject(100);
        EXPECT_EQ(subject.getCountProperty(), 0);
    }

    TEST(ParticleBufferTests, CountWhenSomeParticlesReleasedReturnsCount)
    {
        ParticleBuffer subject(100);
        subject.Release(10);
        EXPECT_EQ(subject.getCountProperty(), 10);
    }

    TEST(ParticleBufferTests, CountWhenAllParticlesReleasedReturnsCapacity)
    {
        ParticleBuffer subject(100);
        subject.Release(100);
        EXPECT_EQ(subject.getCountProperty(), 100);
    }

    TEST(ParticleBufferTests, ReleaseWhenPassedReasonableQuantityReturnsNumberReleased)
    {
        ParticleBuffer subject(100);
        const ParticleIterator& iterator = subject.Release(50);
        EXPECT_EQ(iterator.getTotalProperty(), 50);
    }

    TEST(ParticleBufferTests, ReleaseWhenPassedImpossibleQuantityReturnsNumberActuallyReleased)
    {
        ParticleBuffer subject(100);
        const ParticleIterator& iterator = subject.Release(200);
        EXPECT_EQ(iterator.getTotalProperty(), 100);
    }

    TEST(ParticleBufferTests, ReclaimWhenPassedReasonableNumberReclaimsParticles)
    {
        ParticleBuffer subject(100);
        subject.Release(100);
        EXPECT_EQ(subject.getCountProperty(), 100);

        subject.Reclaim(50);
        EXPECT_EQ(subject.getCountProperty(), 50);
    }

    TEST(ParticleBufferTests, DisposeIsIdempotent)
    {
        ParticleBuffer subject(100);
        subject.Dispose();
        subject.Dispose();
        EXPECT_TRUE(subject.getIsDisposedProperty());
    }

    // Fresh tests: no upstream coverage for wraparound or SizeInBytes.
    TEST(ParticleBufferTests, ReleaseAndReclaimWrapsAroundCorrectly)
    {
        ParticleBuffer subject(10);
        subject.Release(8);
        subject.Reclaim(8);
        EXPECT_EQ(subject.getCountProperty(), 0);

        // Release again, past the point where the tail would wrap around the end of the buffer.
        const ParticleIterator& iterator = subject.Release(8);
        EXPECT_EQ(iterator.getTotalProperty(), 8);
        EXPECT_EQ(subject.getCountProperty(), 8);
    }

    TEST(ParticleBufferTests, SizeInBytesAccountsForOneExtraSlot)
    {
        ParticleBuffer subject(10);
        EXPECT_EQ(subject.getSizeInBytesProperty(), static_cast<int>(Data::Particle::SizeInBytes) * 11);
    }
}
