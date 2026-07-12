// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#pragma once

namespace CNA::Extended
{
    // Defined in CNA/Extended/SizeF.hpp (Phase 1, ported after the marker interfaces).
    // Forward-declared here to avoid a circular include; a pure virtual declaration
    // only needs a complete type where it is called/defined, not where it is declared.
    class SizeF;

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
