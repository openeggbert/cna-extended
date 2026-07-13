// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/RandomExtensions.cs. C# extension methods on
// System.Random -> free functions taking System::Random& in this namespace, matching the
// convention used throughout this project. Ported the exact algorithms as written upstream
// (e.g. NextSingle as a (float)NextDouble() cast) rather than delegating to sharp-runtime's own
// System::Random::NextSingle() (which exists and is used elsewhere, but implements a different,
// more modern algorithm matching real .NET 6+ Random.NextSingle() -- using it here would
// silently change behavior from what this specific upstream file actually does).
#pragma once

#include "CNA/Extended/Interval.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/Random.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    /**
     * @brief Returns a random integer that is within a closed interval.
     * @param random The random number generator.
     * @param interval A closed interval representing the lower and upper bounds of the random
     * number to return.
     */
    [[nodiscard]] int Next(System::Random& random, const Interval<int>& interval);

    /**
     * @brief Returns a random floating-point number that is within a specified range.
     * @param random The random number generator.
     * @param min The inclusive lower bound of the random number returned.
     * @param max The exclusive upper bound of the random number returned.
     */
    [[nodiscard]] float NextSingle(System::Random& random, float min, float max);

    /**
     * @brief Returns a random floating-point number that is greater than or equal to 0.0 and
     * less than the specified maximum.
     * @param random The random number generator.
     * @param max The exclusive upper bound of the random number generated.
     */
    [[nodiscard]] float NextSingle(System::Random& random, float max);

    /**
     * @brief Returns a random floating-point number that is greater than or equal to 0.0 and
     * less than 1.0.
     * @param random The random number generator.
     */
    [[nodiscard]] float NextSingle(System::Random& random);

    /**
     * @brief Returns a random floating-point number that is within a closed interval.
     * @param random The random number generator.
     * @param interval A closed interval representing the lower and upper bounds of the random
     * number to return.
     */
    [[nodiscard]] float NextSingle(System::Random& random, const Interval<float>& interval);

    /**
     * @brief Returns a random angle between -pi and pi, in radians.
     * @param random The random number generator.
     */
    [[nodiscard]] float NextAngle(System::Random& random);

    /**
     * @brief Gets a random unit vector.
     * @param random The random number generator.
     * @param vector Receives a unit vector with a random direction.
     */
    void NextUnitVector(System::Random& random, Vector2& vector);
}
