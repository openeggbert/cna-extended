// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ECS/WorldBuilder.hpp"

#include "CNA/Extended/ECS/World.hpp"

namespace CNA::Extended::ECS
{
    WorldBuilder& WorldBuilder::AddSystem(std::unique_ptr<Systems::ISystem> system)
    {
        systems_.push_back(std::move(system));
        return *this;
    }

    std::unique_ptr<World> WorldBuilder::Build()
    {
        auto world = std::make_unique<World>();

        for (std::unique_ptr<Systems::ISystem>& system : systems_)
        {
            world->RegisterSystem(std::move(system));
        }
        systems_.clear();

        return world;
    }
}
