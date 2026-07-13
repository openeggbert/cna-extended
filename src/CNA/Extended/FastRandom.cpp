// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/FastRandom.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <cmath>
#include <stdexcept>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::MathHelper;

    // ---------------------------------------------------------------------------------------
    // FastRandom
    // ---------------------------------------------------------------------------------------

    FastRandom& FastRandom::getSharedProperty()
    {
        static FastRandom instance(std::make_unique<ThreadSafeFastRandomImpl>());
        return instance;
    }

    FastRandom::FastRandom() : FastRandom(1)
    {
    }

    FastRandom::FastRandom(const int seed) : impl_(std::make_unique<LinearCongruentialGeneratorImpl>(seed))
    {
    }

    FastRandom::FastRandom(std::unique_ptr<IFastRandomImpl> impl) : impl_(std::move(impl))
    {
        if (!impl_)
        {
            throw std::invalid_argument("impl");
        }
    }

    FastRandom::~FastRandom() = default;
    FastRandom::FastRandom(FastRandom&&) noexcept = default;
    FastRandom& FastRandom::operator=(FastRandom&&) noexcept = default;

    int FastRandom::Next()
    {
        return impl_->Next();
    }

    int FastRandom::Next(const int max)
    {
        return impl_->Next(max);
    }

    int FastRandom::Next(const int min, const int max)
    {
        return impl_->Next(min, max);
    }

    int FastRandom::Next(const Interval<int>& interval)
    {
        return impl_->Next(interval);
    }

    float FastRandom::NextSingle()
    {
        return impl_->NextSingle();
    }

    float FastRandom::NextSingle(const float max)
    {
        return impl_->NextSingle(max);
    }

    float FastRandom::NextSingle(const float min, const float max)
    {
        return impl_->NextSingle(min, max);
    }

    float FastRandom::NextSingle(const Interval<float>& interval)
    {
        return impl_->NextSingle(interval);
    }

    float FastRandom::NextAngle()
    {
        return impl_->NextAngle();
    }

    void FastRandom::NextUnitVector(Vector2& vector)
    {
        impl_->NextUnitVector(vector);
    }

    void FastRandom::NextUnitVector(Vector2* vector)
    {
        impl_->NextUnitVector(vector);
    }

    // ---------------------------------------------------------------------------------------
    // LinearCongruentialGeneratorImpl
    // ---------------------------------------------------------------------------------------

    FastRandom::LinearCongruentialGeneratorImpl::LinearCongruentialGeneratorImpl() : LinearCongruentialGeneratorImpl(1)
    {
    }

    FastRandom::LinearCongruentialGeneratorImpl::LinearCongruentialGeneratorImpl(const int seed)
    {
        if (seed <= 0)
        {
            throw std::out_of_range("seed");
        }
        state_ = seed;
    }

    int FastRandom::LinearCongruentialGeneratorImpl::Next()
    {
        state_ = kMultiplier * state_ + kIncrement;
        return (state_ >> 16) & 0x7FFF;
    }

    int FastRandom::LinearCongruentialGeneratorImpl::Next(const int max)
    {
        return static_cast<int>(static_cast<float>(max) * NextSingle() + 0.5f);
    }

    int FastRandom::LinearCongruentialGeneratorImpl::Next(const int min, const int max)
    {
        return static_cast<int>(static_cast<float>(max - min) * NextSingle() + 0.5f) + min;
    }

    int FastRandom::LinearCongruentialGeneratorImpl::Next(const Interval<int>& interval)
    {
        return Next(interval.getMinProperty(), interval.getMaxProperty());
    }

    float FastRandom::LinearCongruentialGeneratorImpl::NextSingle()
    {
        return static_cast<float>(Next()) / 32767.0f; // short.MaxValue
    }

    float FastRandom::LinearCongruentialGeneratorImpl::NextSingle(const float max)
    {
        return max * NextSingle();
    }

    float FastRandom::LinearCongruentialGeneratorImpl::NextSingle(const float min, const float max)
    {
        return (max - min) * NextSingle() + min;
    }

    float FastRandom::LinearCongruentialGeneratorImpl::NextSingle(const Interval<float>& interval)
    {
        return NextSingle(interval.getMinProperty(), interval.getMaxProperty());
    }

    float FastRandom::LinearCongruentialGeneratorImpl::NextAngle()
    {
        return NextSingle(-MathHelper::Pi, MathHelper::Pi);
    }

    void FastRandom::LinearCongruentialGeneratorImpl::NextUnitVector(Vector2& vector)
    {
        const float angle = NextAngle();
        vector.X = std::cos(angle);
        vector.Y = std::sin(angle);
    }

    void FastRandom::LinearCongruentialGeneratorImpl::NextUnitVector(Vector2* vector)
    {
        const float angle = NextAngle();
        vector->X = std::cos(angle);
        vector->Y = std::sin(angle);
    }

    // ---------------------------------------------------------------------------------------
    // ThreadSafeFastRandomImpl
    // ---------------------------------------------------------------------------------------

    FastRandom::LinearCongruentialGeneratorImpl& FastRandom::ThreadSafeFastRandomImpl::LocalRandom()
    {
        thread_local LinearCongruentialGeneratorImpl instance;
        return instance;
    }

    int FastRandom::ThreadSafeFastRandomImpl::Next()
    {
        return LocalRandom().Next();
    }

    int FastRandom::ThreadSafeFastRandomImpl::Next(const int max)
    {
        return LocalRandom().Next(max);
    }

    int FastRandom::ThreadSafeFastRandomImpl::Next(const int min, const int max)
    {
        return LocalRandom().Next(min, max);
    }

    int FastRandom::ThreadSafeFastRandomImpl::Next(const Interval<int>& interval)
    {
        return LocalRandom().Next(interval.getMinProperty(), interval.getMaxProperty());
    }

    float FastRandom::ThreadSafeFastRandomImpl::NextSingle()
    {
        return LocalRandom().NextSingle();
    }

    float FastRandom::ThreadSafeFastRandomImpl::NextSingle(const float max)
    {
        return LocalRandom().NextSingle(max);
    }

    float FastRandom::ThreadSafeFastRandomImpl::NextSingle(const float min, const float max)
    {
        return LocalRandom().NextSingle(min, max);
    }

    float FastRandom::ThreadSafeFastRandomImpl::NextSingle(const Interval<float>& interval)
    {
        return LocalRandom().NextSingle(interval.getMinProperty(), interval.getMaxProperty());
    }

    float FastRandom::ThreadSafeFastRandomImpl::NextAngle()
    {
        return LocalRandom().NextAngle();
    }

    void FastRandom::ThreadSafeFastRandomImpl::NextUnitVector(Vector2& vector)
    {
        LocalRandom().NextUnitVector(vector);
    }

    void FastRandom::ThreadSafeFastRandomImpl::NextUnitVector(Vector2* vector)
    {
        LocalRandom().NextUnitVector(vector);
    }
}
