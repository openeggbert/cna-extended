// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tweening/EasingFunctions.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <cmath>
#include <stdexcept>

namespace CNA::Extended::Tweening
{
    using Microsoft::Xna::Framework::MathHelper;

    std::function<float(float)> EasingFunctions::Invert(std::function<float(float)> easer)
    {
        if (!easer)
        {
            throw std::invalid_argument("easer must not be null.");
        }
        return [easer = std::move(easer)](float t) { return 1.0f - easer(1.0f - t); };
    }

    std::function<float(float)> EasingFunctions::Follow(std::function<float(float)> first, std::function<float(float)> second)
    {
        if (!first)
        {
            throw std::invalid_argument("first must not be null.");
        }
        if (!second)
        {
            throw std::invalid_argument("second must not be null.");
        }
        return [first = std::move(first), second = std::move(second)](float t)
        { return (t <= 0.5f) ? first(t * 2.0f) / 2.0f : second(t * 2.0f - 1.0f) / 2.0f + 0.5f; };
    }

    float EasingFunctions::Linear(float value)
    {
        return value;
    }

    float EasingFunctions::CubicIn(float value)
    {
        return Power::In(value, 3);
    }

    float EasingFunctions::CubicOut(float value)
    {
        return Power::Out(value, 3);
    }

    float EasingFunctions::CubicInOut(float value)
    {
        return Power::InOut(value, 3);
    }

    float EasingFunctions::QuadraticIn(float value)
    {
        return Power::In(value, 2);
    }

    float EasingFunctions::QuadraticOut(float value)
    {
        return Power::Out(value, 2);
    }

    float EasingFunctions::QuadraticInOut(float value)
    {
        return Power::InOut(value, 2);
    }

    float EasingFunctions::QuarticIn(float value)
    {
        return Power::In(value, 4);
    }

    float EasingFunctions::QuarticOut(float value)
    {
        return Power::Out(value, 4);
    }

    float EasingFunctions::QuarticInOut(float value)
    {
        return Power::InOut(value, 4);
    }

    float EasingFunctions::QuinticIn(float value)
    {
        return Power::In(value, 5);
    }

    float EasingFunctions::QuinticOut(float value)
    {
        return Power::Out(value, 5);
    }

    float EasingFunctions::QuinticInOut(float value)
    {
        return Power::InOut(value, 5);
    }

    float EasingFunctions::SineIn(float value)
    {
        return static_cast<float>(std::sin(value * MathHelper::PiOver2 - MathHelper::PiOver2)) + 1.0f;
    }

    float EasingFunctions::SineOut(float value)
    {
        return static_cast<float>(std::sin(value * MathHelper::PiOver2));
    }

    float EasingFunctions::SineInOut(float value)
    {
        return static_cast<float>(std::sin(value * MathHelper::Pi - MathHelper::PiOver2) + 1.0) / 2.0f;
    }

    float EasingFunctions::ExponentialIn(float value)
    {
        return static_cast<float>(std::pow(2.0, 10.0 * (value - 1.0)));
    }

    float EasingFunctions::ExponentialOut(float value)
    {
        return Out(value, ExponentialIn);
    }

    float EasingFunctions::ExponentialInOut(float value)
    {
        return InOut(value, ExponentialIn);
    }

    float EasingFunctions::CircleIn(float value)
    {
        return static_cast<float>(-(std::sqrt(1.0 - static_cast<double>(value) * value) - 1.0));
    }

    float EasingFunctions::CircleOut(float value)
    {
        return static_cast<float>(std::sqrt(1.0 - static_cast<double>(value - 1.0f) * (value - 1.0f)));
    }

    float EasingFunctions::CircleInOut(float value)
    {
        if (value <= 0.5f)
        {
            return static_cast<float>((std::sqrt(1.0 - static_cast<double>(value) * value * 4.0) - 1.0) / -2.0);
        }
        return static_cast<float>((std::sqrt(1.0 - static_cast<double>(value * 2.0f - 2.0f) * (value * 2.0f - 2.0f)) + 1.0) / 2.0);
    }

    float EasingFunctions::ElasticIn(float value)
    {
        constexpr int oscillations = 1;
        constexpr float springiness = 3.0f;
        const double e = (std::exp(springiness * value) - 1.0) / (std::exp(springiness) - 1.0);
        return static_cast<float>(e * std::sin((MathHelper::PiOver2 + MathHelper::TwoPi * oscillations) * value));
    }

    float EasingFunctions::ElasticOut(float value)
    {
        return Out(value, ElasticIn);
    }

    float EasingFunctions::ElasticInOut(float value)
    {
        return InOut(value, ElasticIn);
    }

    float EasingFunctions::BackIn(float value)
    {
        constexpr float amplitude = 1.0f;
        return static_cast<float>(std::pow(value, 3) - value * amplitude * std::sin(value * MathHelper::Pi));
    }

    float EasingFunctions::BackOut(float value)
    {
        return Out(value, BackIn);
    }

    float EasingFunctions::BackInOut(float value)
    {
        return InOut(value, BackIn);
    }

    float EasingFunctions::BounceOut(float value)
    {
        return Out(value, BounceIn);
    }

    float EasingFunctions::BounceInOut(float value)
    {
        return InOut(value, BounceIn);
    }

    float EasingFunctions::BounceIn(float value)
    {
        constexpr float bounceConst1 = 2.75f;
        const float bounceConst2 = static_cast<float>(std::pow(bounceConst1, 2));

        value = 1.0f - value;

        if (value < 1.0f / bounceConst1)
        {
            return 1.0f - bounceConst2 * value * value;
        }

        if (value < 2.0f / bounceConst1)
        {
            return 1.0f - static_cast<float>(bounceConst2 * std::pow(value - 1.5f / bounceConst1, 2) + 0.75);
        }

        if (value < 2.5f / bounceConst1)
        {
            return 1.0f - static_cast<float>(bounceConst2 * std::pow(value - 2.25f / bounceConst1, 2) + 0.9375);
        }

        return 1.0f - static_cast<float>(bounceConst2 * std::pow(value - 2.625f / bounceConst1, 2) + 0.984375);
    }

    float EasingFunctions::Out(float value, const std::function<float(float)>& function)
    {
        return 1.0f - function(1.0f - value);
    }

    float EasingFunctions::InOut(float value, const std::function<float(float)>& function)
    {
        if (value < 0.5f)
        {
            return 0.5f * function(value * 2.0f);
        }
        return 1.0f - 0.5f * function(2.0f - value * 2.0f);
    }

    float EasingFunctions::Power::In(double value, int power)
    {
        return static_cast<float>(std::pow(value, power));
    }

    float EasingFunctions::Power::Out(double value, int power)
    {
        const int sign = (power % 2 == 0) ? -1 : 1;
        return static_cast<float>(sign * (std::pow(value - 1.0, power) + sign));
    }

    float EasingFunctions::Power::InOut(double s, int power)
    {
        s *= 2.0;

        if (s < 1.0)
        {
            return In(s, power) / 2.0f;
        }

        const int sign = (power % 2 == 0) ? -1 : 1;
        return static_cast<float>(sign / 2.0 * (std::pow(s - 2.0, power) + sign * 2));
    }
}
