// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/ComponentMapper.cs. Upstream's non-generic `ComponentMapper`
// base class stores a C# `Type` (`ComponentType` property); translated as `std::type_index`
// (`<typeindex>`, wrapping `typeid(T)`) -- a direct, standard C++ mechanism for "map a compile-time
// type to a runtime-comparable identity," not a reflection substitute needing a design decision
// (unlike e.g. Tweening's property-path reflection, which had no such direct equivalent).
//
// `Components` is a `Bag<T>` of raw component pointers/values -- upstream's `T` is constrained to
// `class` (a C# reference type), so component instances are heap-allocated reference-type objects
// whose lifetime is managed by whoever calls `Attach`. This project's `Bag<T*>` (non-owning
// pointers) mirrors that: `Put`/`Delete` never allocate or free the component itself, matching
// upstream (`Components[entityId] = component;` / `= null;` never call `new`/dispose either --
// the caller passed in an already-constructed component and remains responsible for its
// lifetime).
//
// Naming: upstream has both a non-generic `ComponentMapper` class and a generic `ComponentMapper
// <T>` class in the same namespace -- C# allows two types to share an identifier when their
// arity differs (arity-based overloading of type names). C++ does not allow a class template and
// a non-template class to share a name in the same namespace. The generic one is therefore named
// `ComponentMapperOf<T>` here, a forced rename, not a stylistic choice.
//
// Upstream's `Get(Entity entity)`/`TryGet(Entity entity, ...)` convenience overloads (which just
// forward to the `int entityId` overload via `entity.Id`) are not ported: including `Entity.hpp`
// here to give `ComponentMapperOf<T>` a definition for them would create a circular include
// (`Entity.hpp` needs `ComponentManager.hpp`, which needs this file). Nothing internal to this
// module ever calls the `Entity`-keyed overloads (`Entity` itself always uses its own `Id` with
// the `int`-keyed overloads directly); confirmed via the upstream test suite too -- only the
// `int entityId` overloads are exercised anywhere. External callers can call
// `mapper.Get(entity.getIdProperty())` instead.
#pragma once

#include "CNA/Extended/Collections/Bag.hpp"
#include "System/MulticastAction.hpp"

#include <typeindex>

namespace CNA::Extended::ECS
{
    /** @brief Non-generic base for a per-component-type storage mapper (component-type identity, presence/removal). */
    class ComponentMapper
    {
    public:
        ComponentMapper(int id, std::type_index componentType) : id_(id), componentType_(componentType) {}
        virtual ~ComponentMapper() = default;

        /** @brief Gets the component-type ID assigned to this mapper by ComponentManager. */
        [[nodiscard]] int getIdProperty() const { return id_; }

        /** @brief Gets the identity of the component type this mapper stores. */
        [[nodiscard]] std::type_index getComponentTypeProperty() const { return componentType_; }

        /** @brief Gets whether entity @p entityId currently has a component of this mapper's type. */
        [[nodiscard]] virtual bool Has(int entityId) const = 0;

        /** @brief Removes entity @p entityId's component of this mapper's type, if present. */
        virtual void Delete(int entityId) = 0;

    private:
        int id_;
        std::type_index componentType_;
    };

    /** @brief Per-component-type storage: attaches/retrieves/removes components of type T, keyed by entity ID. */
    template <typename T>
    class ComponentMapperOf : public ComponentMapper
    {
    public:
        /** @brief Raised (with the entity ID) after a component is attached via Put. */
        System::MulticastAction<int> OnPut;

        /** @brief Raised (with the entity ID) after a component is removed via Delete. */
        System::MulticastAction<int> OnDelete;

        ComponentMapperOf(int id, System::MulticastAction<int>* onCompositionChanged)
            : ComponentMapper(id, std::type_index(typeid(T))), onCompositionChanged_(onCompositionChanged)
        {
        }

        /** @brief Gets the Bag backing this mapper's per-entity component storage. */
        [[nodiscard]] Collections::Bag<T*>& getComponentsProperty() { return components_; }
        /** @copydoc getComponentsProperty() */
        [[nodiscard]] const Collections::Bag<T*>& getComponentsProperty() const { return components_; }

        /** @brief Attaches @p component to entity @p entityId, raising OnPut and the owning ComponentManager's composition-changed notification. */
        void Put(int entityId, T* component)
        {
            components_.Set(static_cast<std::size_t>(entityId), component);
            (*onCompositionChanged_)(entityId);
            OnPut(entityId);
        }

        /** @brief Gets entity @p entityId's component, or nullptr if it has none of this type. */
        [[nodiscard]] T* Get(int entityId) const { return components_[static_cast<std::size_t>(entityId)]; }

        /** @brief Attempts to get entity @p entityId's component. Returns true and sets @p result if present. */
        [[nodiscard]] bool TryGet(int entityId, T*& result) const
        {
            result = Get(entityId);
            return result != nullptr;
        }

        [[nodiscard]] bool Has(int entityId) const override
        {
            if (static_cast<std::size_t>(entityId) >= components_.getCountProperty())
            {
                return false;
            }
            return components_[static_cast<std::size_t>(entityId)] != nullptr;
        }

        void Delete(int entityId) override
        {
            OnDelete(entityId);
            components_.Set(static_cast<std::size_t>(entityId), nullptr);
            (*onCompositionChanged_)(entityId);
        }

    private:
        System::MulticastAction<int>* onCompositionChanged_;
        Collections::Bag<T*> components_;
    };
}
