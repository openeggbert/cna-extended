// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#pragma once

#include "CNA/Extended/SizeF.hpp"

namespace CNA::Extended
{
    /** @brief Defines a sizable object with a 2D size. */
    class ISizable
    {
    public:
        virtual ~ISizable() = default;

        /** @brief Gets the size. */
        [[nodiscard]] virtual SizeF getSizeProperty() const = 0;

        /** @brief Sets the size. */
        virtual void setSizeProperty(const SizeF& value) = 0;
    };
}
