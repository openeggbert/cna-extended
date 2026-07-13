// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    class MathExtended
    {
    public:
        MathExtended() = delete;

        /**
         * @brief The smallest positive value that can be added to 1.0 to produce a
         * distinguishable result. Approximately 1.19209290e-7; useful for floating-point
         * comparisons.
         */
        static constexpr float MachineEpsilon = 1.19209290e-7f;

        /**
         * @brief Calculates a new Vector2 with the component-wise minimum values from two given
         * Vector2 values.
         */
        [[nodiscard]] static Vector2 CalculateMinimumVector2(const Vector2& first, const Vector2& second);

        /**
         * @brief Calculates a new Vector2 with the component-wise minimum values from two given
         * Vector2 values.
         * @param result Receives the calculated Vector2 value with the component-wise minimum
         * values.
         */
        static void CalculateMinimumVector2(const Vector2& first, const Vector2& second, Vector2& result);

        /**
         * @brief Calculates a new Vector2 with the component-wise maximum values from two given
         * Vector2 values.
         */
        [[nodiscard]] static Vector2 CalculateMaximumVector2(const Vector2& first, const Vector2& second);

        /**
         * @brief Calculates a new Vector2 with the component-wise maximum values from two given
         * Vector2 values.
         * @param result Receives the calculated Vector2 value with the component-wise maximum
         * values.
         */
        static void CalculateMaximumVector2(const Vector2& first, const Vector2& second, Vector2& result);
    };
}
