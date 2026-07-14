// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ActorPairKey3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "System/HashCode.hpp"

namespace CNA::Extended::World3DEXT
{
    ActorPairKey3DEXT::ActorPairKey3DEXT(const ICollisionActor3DEXT& first, const ICollisionActor3DEXT& second)
    {
        if (first.getIdProperty() <= second.getIdProperty())
        {
            FirstId = first.getIdProperty();
            SecondId = second.getIdProperty();
        }
        else
        {
            FirstId = second.getIdProperty();
            SecondId = first.getIdProperty();
        }
    }

    int ActorPairKey3DEXT::GetHashCode() const
    {
        return System::HashCode::Combine(FirstId, SecondId);
    }
}
