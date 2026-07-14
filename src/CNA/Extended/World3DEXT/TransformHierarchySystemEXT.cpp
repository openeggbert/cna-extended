// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/TransformHierarchySystemEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"

#include <typeindex>
#include <unordered_set>

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
            if (component->ParentEntityIdEXT >= 0 && !WouldCreateCycle(entityId, component->ParentEntityIdEXT))
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

    bool TransformHierarchySystemEXT::WouldCreateCycle(int entityId, int candidateParentId)
    {
        // Self-parenting is trivially a 1-node cycle.
        if (candidateParentId == entityId)
        {
            return true;
        }

        // Walk the candidate parent's own ancestor chain via each node's ECS
        // ParentEntityIdEXT (not yet-resolved Transform3 pointers -- this frame hasn't wired
        // them yet). If entityId reappears, wiring candidateParentId as entityId's parent
        // would close a cycle. The visited set also guards a *pre-existing* cycle elsewhere
        // in the graph that doesn't happen to pass through entityId, so a corrupt chain
        // can't walk forever.
        std::unordered_set<int> visited;
        int currentId = candidateParentId;
        while (currentId >= 0)
        {
            if (currentId == entityId || !visited.insert(currentId).second)
            {
                return true;
            }

            Entity* currentEntity = GetEntity(currentId);
            if (currentEntity == nullptr)
            {
                return false;
            }

            Transform3ComponentEXT* currentComponent = currentEntity->Get<Transform3ComponentEXT>();
            if (currentComponent == nullptr)
            {
                return false;
            }

            currentId = currentComponent->ParentEntityIdEXT;
        }

        return false;
    }
}
