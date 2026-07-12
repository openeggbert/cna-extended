// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#pragma once

#include "Microsoft/Xna/Framework/Rectangle.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Rectangle;

    // Defined in CNA/Extended/RectangleF.hpp (Phase 1, ported later). Forward-declared
    // here to avoid a circular include.
    class RectangleF;

    /** @brief Defines an object with an integer bounding rectangle. */
    class IRectangular
    {
    public:
        virtual ~IRectangular() = default;

        /** @brief Gets the bounding rectangle. */
        [[nodiscard]] virtual Rectangle getBoundingRectangleProperty() const = 0;
    };

    /** @brief Defines an object with a floating-point bounding rectangle. */
    class IRectangularF
    {
    public:
        virtual ~IRectangularF() = default;

        /** @brief Gets the bounding rectangle. */
        [[nodiscard]] virtual RectangleF getBoundingRectangleProperty() const = 0;
    };
}
