// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/Aspect.cs. Upstream's public fields (not properties) --
// `AllSet`/`ExclusionSet`/`OneSet` -- are kept as plain public data members, matching this
// project's established convention for upstream types that themselves use fields rather than
// C# properties (e.g. Interval<T>, Color).
#pragma once

#include "CNA/Extended/ECS/ComponentBits.hpp"

#include <typeindex>
#include <vector>

namespace CNA::Extended::ECS
{
    class AspectBuilder;

    /** @brief A component-composition filter: which component types an entity must have, must not have, or must have at least one of. */
    class Aspect
    {
    public:
        Aspect() = default;

        ComponentBits AllSet;
        ComponentBits ExclusionSet;
        ComponentBits OneSet;

        /** @brief Starts building an Aspect requiring all of @p types (upstream's `params Type[]` allows zero args; matched here via a default empty list). */
        [[nodiscard]] static AspectBuilder All(const std::vector<std::type_index>& types = {});

        /** @brief Starts building an Aspect requiring at least one of @p types. */
        [[nodiscard]] static AspectBuilder One(const std::vector<std::type_index>& types = {});

        /** @brief Starts building an Aspect excluding all of @p types. */
        [[nodiscard]] static AspectBuilder Exclude(const std::vector<std::type_index>& types = {});

        /** @brief Determines whether an entity with @p componentBits satisfies this Aspect's All/Exclusion/One requirements. */
        [[nodiscard]] bool IsInterested(const ComponentBits& componentBits) const
        {
            if (!AllSet.getIsEmptyProperty() && !componentBits.HasAll(AllSet))
            {
                return false;
            }

            if (!ExclusionSet.getIsEmptyProperty() && componentBits.HasAny(ExclusionSet))
            {
                return false;
            }

            if (!OneSet.getIsEmptyProperty() && !componentBits.HasAny(OneSet))
            {
                return false;
            }

            return true;
        }
    };
}
