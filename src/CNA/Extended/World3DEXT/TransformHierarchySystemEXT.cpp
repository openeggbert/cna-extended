// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/TransformHierarchySystemEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"

#include <typeindex>

namespace CNA::Extended::World3DEXT
{
    using ECS::AspectBuilder;
    using ECS::ComponentManager;
    using ECS::Entity;
    using Microsoft::Xna::Framework::GameTime;

    TransformHierarchySystemEXT::TransformHierarchySystemEXT()
        : EntityUpdateSystem(AspectBuilder().All({std::type_index(typeid(Transform3ComponentEXT))}))
    {
    }

    void TransformHierarchySystemEXT::Initialize(ComponentManager& componentManager)
    {
        (void)componentManager; // no cached ComponentMapper needed: Entity::Get<T>() below is cheap
    }

    void TransformHierarchySystemEXT::Update(const GameTime& gameTime)
    {
        (void)gameTime;

        for (const int entityId : getActiveEntitiesProperty())
        {
            Entity* entity = GetEntity(entityId);
            if (entity == nullptr)
            {
                continue;
            }

            Transform3ComponentEXT* component = entity->Get<Transform3ComponentEXT>();
            if (component == nullptr)
            {
                continue;
            }

            Transform3ComponentEXT* parentComponent = nullptr;
            if (component->ParentEntityIdEXT >= 0)
            {
                Entity* parentEntity = GetEntity(component->ParentEntityIdEXT);
                if (parentEntity != nullptr)
                {
                    parentComponent = parentEntity->Get<Transform3ComponentEXT>();
                }
            }

            component->TransformEXT.setParentProperty(parentComponent != nullptr ? &parentComponent->TransformEXT : nullptr);
        }
    }
}
