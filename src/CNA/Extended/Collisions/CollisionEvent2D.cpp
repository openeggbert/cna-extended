// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collisions/CollisionEvent2D.hpp"

#include "CNA/Extended/Collisions/ICollisionActor.hpp"

namespace CNA::Extended::Collisions
{
    int CollisionEvent2D::getOtherIdProperty() const
    {
        return Other_->getIdProperty();
    }
}
