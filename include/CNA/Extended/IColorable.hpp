// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Color;

    /** @brief Defines an object with a color. */
    class IColorable
    {
    public:
        virtual ~IColorable() = default;

        /** @brief Gets the color. */
        [[nodiscard]] virtual Color getColorProperty() const = 0;

        /** @brief Sets the color. */
        virtual void setColorProperty(const Color& value) = 0;
    };
}
