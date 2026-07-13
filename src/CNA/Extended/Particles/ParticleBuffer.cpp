// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/ParticleBuffer.hpp"

#include "System/ObjectDisposedException.hpp"

#include <algorithm>
#include <cstdlib>

namespace CNA::Extended::Particles
{
    ParticleBuffer::ParticleBuffer(int size)
        : size_(size),
          nativePointer_(static_cast<Data::Particle*>(std::malloc(static_cast<std::size_t>(size + 1) * Data::Particle::SizeInBytes))),
          bufferEnd_(nativePointer_ + (size_ + 1)),
          head_(nativePointer_),
          tail_(nativePointer_),
          iterator_(this)
    {
    }

    ParticleBuffer::~ParticleBuffer()
    {
        Dispose();
    }

    int ParticleBuffer::getSizeInBytesProperty() const
    {
        return static_cast<int>(Data::Particle::SizeInBytes) * (size_ + 1);
    }

    int ParticleBuffer::getActiveSizeInBytesProperty() const
    {
        return static_cast<int>(Data::Particle::SizeInBytes) * count_;
    }

    ParticleIterator& ParticleBuffer::Release(int releaseQuantity)
    {
        System::ObjectDisposedException::ThrowIf(isDisposed_, "ParticleBuffer");

        const int numToRelease = std::min(releaseQuantity, getAvailableProperty());

        const int prevCount = count_;
        count_ += numToRelease;

        tail_ += numToRelease;

        if (tail_ >= bufferEnd_)
        {
            tail_ -= size_ + 1;
        }

        return getIteratorProperty().Reset(prevCount);
    }

    void ParticleBuffer::Reclaim(int number)
    {
        System::ObjectDisposedException::ThrowIf(isDisposed_, "ParticleBuffer");

        count_ -= number;

        head_ += number;

        if (head_ >= bufferEnd_)
        {
            head_ -= size_ + 1;
        }
    }

    void ParticleBuffer::Dispose()
    {
        if (isDisposed_)
        {
            return;
        }

        std::free(nativePointer_);
        isDisposed_ = true;
    }
}
