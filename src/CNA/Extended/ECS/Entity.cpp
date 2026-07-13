// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ECS/Entity.hpp"

#include "CNA/Extended/ECS/EntityManager.hpp"

namespace CNA::Extended::ECS
{
    ComponentBits Entity::getComponentBitsProperty() const
    {
        return entityManager_->GetComponentBits(id_);
    }

    void Entity::Destroy()
    {
        entityManager_->Destroy(id_);
    }
}
