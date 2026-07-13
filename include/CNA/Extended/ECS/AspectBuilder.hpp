// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/AspectBuilder.cs. Upstream's `params Type[] types` variadic
// arrays are translated as `const std::vector<std::type_index>&` throughout (a runtime-collected
// list, not a compile-time type pack), matching how callers actually build these lists dynamically.
// Upstream stores its accumulated type lists in `Bag<Type>`; ported as plain `std::vector
// <std::type_index>` instead of this project's `Bag<T>` -- `std::type_index` has no default
// constructor, which `Bag<T>` requires (its own doc comment: "must support default construction"),
// and none of `Bag<T>`'s distinguishing behavior (O(1) swap-remove, index-addressed growth) is
// actually used here; only `Add` and iteration are, which `std::vector` provides equally faithfully.
#pragma once

#include <typeindex>
#include <vector>

namespace CNA::Extended::ECS
{
    class Aspect;
    class ComponentManager;

    /** @brief Accumulates the All/One/Exclusion component-type requirements used to build an Aspect. */
    class AspectBuilder
    {
    public:
        AspectBuilder() = default;

        /** @brief Gets the component types every matching entity must have. */
        [[nodiscard]] const std::vector<std::type_index>& getAllTypesProperty() const { return allTypes_; }
        /** @brief Gets the component types no matching entity may have. */
        [[nodiscard]] const std::vector<std::type_index>& getExclusionTypesProperty() const { return exclusionTypes_; }
        /** @brief Gets the component types a matching entity must have at least one of. */
        [[nodiscard]] const std::vector<std::type_index>& getOneTypesProperty() const { return oneTypes_; }

        /** @brief Adds to the set of component types every matching entity must have. Returns *this for chaining. */
        AspectBuilder& All(const std::vector<std::type_index>& types = {});

        /** @brief Adds to the set of component types a matching entity must have at least one of. Returns *this for chaining. */
        AspectBuilder& One(const std::vector<std::type_index>& types = {});

        /** @brief Adds to the set of component types no matching entity may have. Returns *this for chaining. */
        AspectBuilder& Exclude(const std::vector<std::type_index>& types = {});

        /** @brief Resolves every accumulated type against @p componentManager, producing the final Aspect. */
        [[nodiscard]] Aspect Build(ComponentManager& componentManager) const;

    private:
        std::vector<std::type_index> allTypes_;
        std::vector<std::type_index> exclusionTypes_;
        std::vector<std::type_index> oneTypes_;
    };
}
