// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ECS/AspectBuilder.hpp"

#include "CNA/Extended/ECS/Aspect.hpp"
#include "CNA/Extended/ECS/ComponentManager.hpp"

namespace CNA::Extended::ECS
{
    namespace
    {
        void Associate(ComponentManager& componentManager, const std::vector<std::type_index>& types, ComponentBits& bits)
        {
            for (const std::type_index& type : types)
            {
                const int id = componentManager.GetComponentTypeId(type);
                bits[id] = true;
            }
        }
    }

    AspectBuilder& AspectBuilder::All(const std::vector<std::type_index>& types)
    {
        allTypes_.insert(allTypes_.end(), types.begin(), types.end());
        return *this;
    }

    AspectBuilder& AspectBuilder::One(const std::vector<std::type_index>& types)
    {
        oneTypes_.insert(oneTypes_.end(), types.begin(), types.end());
        return *this;
    }

    AspectBuilder& AspectBuilder::Exclude(const std::vector<std::type_index>& types)
    {
        exclusionTypes_.insert(exclusionTypes_.end(), types.begin(), types.end());
        return *this;
    }

    Aspect AspectBuilder::Build(ComponentManager& componentManager) const
    {
        Aspect aspect;
        Associate(componentManager, allTypes_, aspect.AllSet);
        Associate(componentManager, oneTypes_, aspect.OneSet);
        Associate(componentManager, exclusionTypes_, aspect.ExclusionSet);
        return aspect;
    }
}
