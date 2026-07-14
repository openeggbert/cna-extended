// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/CollisionPair3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    int CollisionPair3DEXT::getFirstIdProperty() const
    {
        return first_->getIdProperty();
    }

    int CollisionPair3DEXT::getSecondIdProperty() const
    {
        return second_->getIdProperty();
    }
}
