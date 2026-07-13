// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/RandomExtensions.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <cmath>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::MathHelper;

    int Next(System::Random& random, const Interval<int>& interval)
    {
        return random.Next(interval.getMinProperty(), interval.getMaxProperty());
    }

    float NextSingle(System::Random& random, const float min, const float max)
    {
        return (max - min) * NextSingle(random) + min;
    }

    float NextSingle(System::Random& random, const float max)
    {
        return max * NextSingle(random);
    }

    float NextSingle(System::Random& random)
    {
        return static_cast<float>(random.NextDouble());
    }

    float NextSingle(System::Random& random, const Interval<float>& interval)
    {
        return NextSingle(random, interval.getMinProperty(), interval.getMaxProperty());
    }

    float NextAngle(System::Random& random)
    {
        return NextSingle(random, -MathHelper::Pi, MathHelper::Pi);
    }

    void NextUnitVector(System::Random& random, Vector2& vector)
    {
        const float angle = NextAngle(random);
        vector = Vector2(std::cos(angle), std::sin(angle));
    }
}
