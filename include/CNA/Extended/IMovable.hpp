// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief Defines a movable object with a 2D position. */
    class IMovable
    {
    public:
        virtual ~IMovable() = default;

        /** @brief Gets the position. */
        [[nodiscard]] virtual Vector2 getPositionProperty() const = 0;

        /** @brief Sets the position. */
        virtual void setPositionProperty(const Vector2& value) = 0;
    };
}
