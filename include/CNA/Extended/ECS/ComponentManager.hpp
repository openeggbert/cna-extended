// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/ComponentManager.cs.
//
// GENUINE C#/C++ REPRESENTATIONAL GAP, flagged for the orchestrating session rather than silently
// resolved (per this project's established practice for hard-to-reverse decisions):
//
// Upstream's `ComponentMapper CreateMapperForType(Type type, int componentTypeId)` uses real
// reflection (`Activator.CreateInstance(typeof(ComponentMapper<>).MakeGenericType(type), ...)`)
// to construct a `ComponentMapper<T>` for a `T` known only as a runtime `Type` value -- not known
// at that call site's compile time. This backs the non-generic `this[Type type]` indexer, and
// upstream's own `ComponentManagerTests.GetMapperForTypeByIndexer` test calls
// `componentManager[typeof(Transform2)]` as the FIRST access for that type (no prior generic
// `GetMapper<Transform2>()` call), i.e. it genuinely requires "construct T from nothing but a
// runtime type token," which C++ cannot do without either (a) a global/pre-registered factory
// keyed by type, populated in advance for every type that will ever be looked up this way, or
// (b) true reflection. Neither is available here in a way that preserves upstream's exact
// on-demand-from-nothing-but-a-Type semantics.
//
// Resolution taken (deliberately conservative, easy to revisit): `GetMapper(std::type_index)`
// below can only return a mapper that was already created via the generic `GetMapper<T>()` path
// at least once -- it does NOT construct one on first use the way upstream's indexer does. The
// ported `GetMapperForTypeByIndexerTests` test is adapted to call `GetMapper<T>()` first (matching
// how every *other* real call site in this ECS module -- `Entity::Attach<T>` etc. -- actually
// reaches a mapper), then verifies type_index-based lookup returns the same instance. This
// preserves the verifiable, non-reflective part of upstream's behavior (stable identity/lookup by
// type) and drops only the "conjure T from a bare runtime token" capability, which no other code
// in this module actually relies on.
//
// A second, smaller gap: `IComponentMapperService` (upstream interface with a generic method,
// `ComponentMapper<T> GetMapper<T>()`) has no direct translation -- C++ does not allow virtual
// member function templates, so a generic method cannot be part of a polymorphic interface.
// `ComponentManager` is the only implementer anywhere in this codebase, so the interface is not
// separately materialized; `EntitySystem::Initialize` (Systems/EntitySystem.hpp) takes a
// `ComponentManager&` directly instead of an abstracted interface type.
#pragma once

#include "CNA/Extended/Collections/Bag.hpp"
#include "CNA/Extended/ECS/ComponentBits.hpp"
#include "CNA/Extended/ECS/ComponentMapper.hpp"
#include "CNA/Extended/ECS/Systems/UpdateSystem.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "System/MulticastAction.hpp"

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace CNA::Extended::ECS
{
    using Microsoft::Xna::Framework::GameTime;

    /** @brief Owns and looks up per-component-type ComponentMapperOf<T> storage, and derives each entity's ComponentBits. */
    class ComponentManager : public Systems::UpdateSystem
    {
    public:
        static constexpr int MaxComponentTypes = 256;

        ComponentManager() = default;

        /** @brief Raised (with the entity ID) whenever a component is attached to or removed from an entity. */
        System::MulticastAction<int> ComponentsChanged;

        /** @brief Gets (creating on first use) the mapper for component type T. */
        template <typename T>
        ComponentMapperOf<T>* GetMapper()
        {
            const int componentTypeId = GetComponentTypeId(std::type_index(typeid(T)));

            ComponentMapper* existing = componentMappers_[static_cast<std::size_t>(componentTypeId)];
            if (existing != nullptr)
            {
                return static_cast<ComponentMapperOf<T>*>(existing);
            }

            return CreateMapperForType<T>(componentTypeId);
        }

        /**
         * @brief Gets the mapper previously created for @p type via GetMapper<T>(), or nullptr if none has been.
         * See this header's class comment: unlike upstream's indexer, this cannot construct a mapper on first use.
         */
        [[nodiscard]] ComponentMapper* GetMapper(std::type_index type)
        {
            const int componentTypeId = GetComponentTypeId(type);
            return componentMappers_[static_cast<std::size_t>(componentTypeId)];
        }

        /** @brief Gets the mapper previously created for the given componentTypeId, or nullptr if none has been (a direct index lookup, no reflection involved). */
        [[nodiscard]] ComponentMapper* GetMapper(int componentTypeId)
        {
            return componentMappers_[static_cast<std::size_t>(componentTypeId)];
        }

        /** @brief Gets (assigning a new ID on first use) the stable component-type ID for @p type. */
        int GetComponentTypeId(std::type_index type)
        {
            const auto it = componentTypes_.find(type);
            if (it != componentTypes_.end())
            {
                return it->second;
            }

            const auto id = static_cast<int>(componentTypes_.size());
            componentTypes_.emplace(type, id);
            return id;
        }

        /** @brief Builds the ComponentBits for entity @p entityId by querying every known mapper's Has(entityId). */
        [[nodiscard]] ComponentBits CreateComponentBits(int entityId) const
        {
            ComponentBits componentBits;

            for (std::size_t componentId = 0; componentId < componentMappers_.getCountProperty(); ++componentId)
            {
                const ComponentMapper* mapper = componentMappers_[componentId];
                componentBits[static_cast<int>(componentId)] = mapper != nullptr && mapper->Has(entityId);
            }

            return componentBits;
        }

        /** @brief Removes entity @p entityId's component from every mapper. */
        void Destroy(int entityId)
        {
            for (ComponentMapper* mapper : componentMappers_)
            {
                if (mapper != nullptr)
                {
                    mapper->Delete(entityId);
                }
            }
        }

        void Update(const GameTime& gameTime) override { (void)gameTime; }

        /** @brief Gets the underlying, componentTypeId-indexed mapper bag, for iteration (matching upstream's `foreach (var mapper in componentManager)`). */
        [[nodiscard]] const Collections::Bag<ComponentMapper*>& getComponentMappersProperty() const { return componentMappers_; }

        [[nodiscard]] auto begin() const { return componentMappers_.begin(); }
        [[nodiscard]] auto end() const { return componentMappers_.end(); }

    private:
        template <typename T>
        ComponentMapperOf<T>* CreateMapperForType(int componentTypeId)
        {
            if (componentTypeId >= MaxComponentTypes)
            {
                throw std::invalid_argument("Component type limit exceeded. Maximum of 256 component types are supported.");
            }

            auto mapper = std::make_unique<ComponentMapperOf<T>>(componentTypeId, &ComponentsChanged);
            ComponentMapperOf<T>* result = mapper.get();
            ownedMappers_.push_back(std::move(mapper));
            componentMappers_.Set(static_cast<std::size_t>(componentTypeId), result);
            return result;
        }

        std::vector<std::unique_ptr<ComponentMapper>> ownedMappers_;
        Collections::Bag<ComponentMapper*> componentMappers_;
        std::unordered_map<std::type_index, int> componentTypes_;
    };
}
