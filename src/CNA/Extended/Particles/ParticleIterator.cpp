// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/ParticleIterator.hpp"

#include "CNA/Extended/Particles/ParticleBuffer.hpp"
#include "System/ObjectDisposedException.hpp"

#include <stdexcept>

namespace CNA::Extended::Particles
{
    ParticleIterator::ParticleIterator(ParticleBuffer* buffer) : buffer_(buffer)
    {
        if (buffer_ == nullptr)
        {
            throw std::invalid_argument("buffer must not be null.");
        }
        System::ObjectDisposedException::ThrowIf(buffer_->getIsDisposedProperty(), "buffer");
    }

    bool ParticleIterator::getHasNextProperty() const
    {
        return current_ != buffer_->getTailProperty();
    }

    ParticleIterator& ParticleIterator::Reset()
    {
        current_ = buffer_->getHeadProperty();
        total_ = buffer_->getCountProperty();
        return *this;
    }

    ParticleIterator& ParticleIterator::Reset(int offset)
    {
        total_ = buffer_->getCountProperty();

        current_ = buffer_->getHeadProperty() + offset;

        if (current_ >= buffer_->getBufferEndProperty())
        {
            current_ -= buffer_->getSizeProperty() + 1;
        }

        return *this;
    }

    Data::Particle* ParticleIterator::Next()
    {
        Data::Particle* particle = current_;

        ++current_;

        if (current_ == buffer_->getBufferEndProperty())
        {
            current_ = buffer_->getNativePointerProperty();
        }

        return particle;
    }
}
