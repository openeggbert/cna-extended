// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/Entity.cs. A lightweight handle: holds non-owning back-
// pointers to the EntityManager/ComponentManager that created it (both created once and owned by
// World for its whole lifetime, so these pointers always outlive any Entity handle referencing
// them), plus its own stable integer ID. Upstream's generic `Attach<T>`/`Detach<T>`/`Get<T>`/
// `Has<T>` methods (constrained `where T : class`) become C++ templates, each forwarding to
// `ComponentManager::GetMapper<T>()` -- matching upstream's own implementation exactly.
#pragma once

#include "CNA/Extended/ECS/ComponentBits.hpp"
#include "CNA/Extended/ECS/ComponentManager.hpp"

namespace CNA::Extended::ECS
{
    class EntityManager;

    /** @brief A lightweight handle to a set of components tracked under a single stable integer ID. */
    class Entity
    {
    public:
        Entity(int id, EntityManager& entityManager, ComponentManager& componentManager)
            : id_(id), entityManager_(&entityManager), componentManager_(&componentManager)
        {
        }

        /** @brief Gets this entity's stable, unique identifier. */
        [[nodiscard]] int getIdProperty() const { return id_; }

        /** @brief Gets the ComponentBits describing which component types this entity currently has. */
        [[nodiscard]] ComponentBits getComponentBitsProperty() const;

        /** @brief Attaches @p component (of type T) to this entity, replacing any existing component of that type. */
        template <typename T>
        void Attach(T* component)
        {
            ComponentMapperOf<T>* mapper = componentManager_->GetMapper<T>();
            mapper->Put(id_, component);
        }

        /** @brief Detaches this entity's component of type T, if it has one. */
        template <typename T>
        void Detach()
        {
            ComponentMapperOf<T>* mapper = componentManager_->GetMapper<T>();
            mapper->Delete(id_);
        }

        /** @brief Gets this entity's component of type T, or nullptr if it has none. */
        template <typename T>
        [[nodiscard]] T* Get()
        {
            ComponentMapperOf<T>* mapper = componentManager_->GetMapper<T>();
            return mapper->Get(id_);
        }

        /** @brief Gets whether this entity currently has a component of type T. */
        template <typename T>
        [[nodiscard]] bool Has()
        {
            return componentManager_->GetMapper<T>()->Has(id_);
        }

        /** @brief Queues this entity for destruction (see EntityManager::Destroy). */
        void Destroy();

        [[nodiscard]] bool operator==(const Entity& other) const { return id_ == other.id_; }
        [[nodiscard]] bool operator!=(const Entity& other) const { return !(*this == other); }

    private:
        int id_;
        EntityManager* entityManager_;
        ComponentManager* componentManager_;
    };
}
