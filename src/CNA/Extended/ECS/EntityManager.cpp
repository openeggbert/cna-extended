// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ECS/EntityManager.hpp"

#include "CNA/Extended/ECS/ComponentManager.hpp"

#include <cassert>

namespace CNA::Extended::ECS
{
    EntityManager::EntityManager(ComponentManager& componentManager)
        : componentManager_(&componentManager),
          entityPool_(
              [this]() -> Entity*
              {
                  auto entity = std::make_unique<Entity>(nextId_++, *this, *componentManager_);
                  Entity* raw = entity.get();
                  allEntities_.push_back(std::move(entity));
                  return raw;
              },
              static_cast<SharpRuntime::intcs>(DefaultBagSize)),
          entityBag_(DefaultBagSize),
          addedEntities_(DefaultBagSize),
          removedEntities_(DefaultBagSize),
          changedEntities_(DefaultBagSize),
          entityToComponentBits_(DefaultBagSize)
    {
        componentManager_->ComponentsChanged += [this](int entityId) { OnComponentsChanged(entityId); };
    }

    std::vector<int> EntityManager::getEntitiesProperty() const
    {
        std::vector<int> result;
        for (Entity* entity : entityBag_)
        {
            if (entity != nullptr)
            {
                result.push_back(entity->getIdProperty());
            }
        }
        return result;
    }

    Entity& EntityManager::Create()
    {
        Entity* entity = entityPool_.Obtain();
        const int id = entity->getIdProperty();
        assert(entityBag_[static_cast<std::size_t>(id)] == nullptr);
        entityBag_.Set(static_cast<std::size_t>(id), entity);
        addedEntities_.Add(id);
        entityToComponentBits_.Set(static_cast<std::size_t>(id), ComponentBits());
        return *entity;
    }

    void EntityManager::Destroy(int entityId)
    {
        if (!removedEntities_.Contains(entityId))
        {
            removedEntities_.Add(entityId);
        }
    }

    void EntityManager::Destroy(const Entity& entity)
    {
        Destroy(entity.getIdProperty());
    }

    Entity* EntityManager::Get(int entityId) const
    {
        return entityBag_[static_cast<std::size_t>(entityId)];
    }

    ComponentBits EntityManager::GetComponentBits(int entityId) const
    {
        return entityToComponentBits_[static_cast<std::size_t>(entityId)];
    }

    void EntityManager::OnComponentsChanged(int entityId)
    {
        changedEntities_.Add(entityId);
        entityToComponentBits_.Set(static_cast<std::size_t>(entityId), componentManager_->CreateComponentBits(entityId));
    }

    void EntityManager::Update(const GameTime& gameTime)
    {
        (void)gameTime;

        for (int entityId : addedEntities_)
        {
            entityToComponentBits_.Set(static_cast<std::size_t>(entityId), componentManager_->CreateComponentBits(entityId));
            ++activeCount_;
            EntityAdded(entityId);
        }

        for (int entityId : changedEntities_)
        {
            EntityChanged(entityId);
        }

        for (int entityId : removedEntities_)
        {
            // Notify subscribers before removing it from the pool -- otherwise an entity system
            // could still be using the entity when the same ID is obtained again.
            EntityRemoved(entityId);

            Entity* entity = entityBag_[static_cast<std::size_t>(entityId)];
            entityBag_.Set(static_cast<std::size_t>(entityId), nullptr);
            componentManager_->Destroy(entityId);
            entityToComponentBits_.Set(static_cast<std::size_t>(entityId), ComponentBits());
            --activeCount_;
            entityPool_.Free(entity);
        }

        addedEntities_.Clear();
        removedEntities_.Clear();
        changedEntities_.Clear();
    }
}
