// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/CollisionEvent3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    int CollisionEvent3DEXT::getOtherIdProperty() const
    {
        return Other_->getIdProperty();
    }
}
