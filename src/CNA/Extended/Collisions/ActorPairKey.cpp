// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collisions/ActorPairKey.hpp"

#include "CNA/Extended/Collisions/ICollisionActor.hpp"
#include "System/HashCode.hpp"

namespace CNA::Extended::Collisions
{
    ActorPairKey::ActorPairKey(const ICollisionActor& first, const ICollisionActor& second)
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

    int ActorPairKey::GetHashCode() const
    {
        return System::HashCode::Combine(FirstId, SecondId);
    }
}
