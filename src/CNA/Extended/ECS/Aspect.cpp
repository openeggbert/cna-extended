// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ECS/Aspect.hpp"

#include "CNA/Extended/ECS/AspectBuilder.hpp"

namespace CNA::Extended::ECS
{
    AspectBuilder Aspect::All(const std::vector<std::type_index>& types)
    {
        return AspectBuilder().All(types);
    }

    AspectBuilder Aspect::One(const std::vector<std::type_index>& types)
    {
        return AspectBuilder().One(types);
    }

    AspectBuilder Aspect::Exclude(const std::vector<std::type_index>& types)
    {
        return AspectBuilder().Exclude(types);
    }
}
