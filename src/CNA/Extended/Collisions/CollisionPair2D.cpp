// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collisions/CollisionPair2D.hpp"

#include "CNA/Extended/Collisions/ICollisionActor.hpp"

namespace CNA::Extended::Collisions
{
    int CollisionPair2D::getFirstIdProperty() const
    {
        return first_->getIdProperty();
    }

    int CollisionPair2D::getSecondIdProperty() const
    {
        return second_->getIdProperty();
    }
}
