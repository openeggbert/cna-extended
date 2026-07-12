// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief Defines a scalable object with a 2D scale. */
    class IScalable
    {
    public:
        virtual ~IScalable() = default;

        /** @brief Gets the scale. */
        [[nodiscard]] virtual Vector2 getScaleProperty() const = 0;

        /** @brief Sets the scale. */
        virtual void setScaleProperty(const Vector2& value) = 0;
    };
}
