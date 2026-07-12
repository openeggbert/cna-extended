// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#pragma once

namespace CNA::Extended
{
    /** @brief Defines a rotatable object with a rotation angle, in radians. */
    class IRotatable
    {
    public:
        virtual ~IRotatable() = default;

        /** @brief Gets the rotation, in radians. */
        [[nodiscard]] virtual float getRotationProperty() const = 0;

        /** @brief Sets the rotation, in radians. */
        virtual void setRotationProperty(float value) = 0;
    };
}
