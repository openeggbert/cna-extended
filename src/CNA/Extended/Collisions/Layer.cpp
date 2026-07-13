// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collisions/Layer.hpp"

#include "CNA/Extended/Collisions/ICollisionBroadphase2D.hpp"

#include <stdexcept>

namespace CNA::Extended::Collisions
{
    Layer::Layer(std::unique_ptr<ICollisionBroadphase2D> space) : space_(std::move(space))
    {
        if (!space_)
        {
            throw std::invalid_argument("space must not be null.");
        }
    }

    void Layer::Reset()
    {
        if (isDynamic_)
        {
            space_->Reset();
        }
    }
}
