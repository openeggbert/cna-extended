// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/Systems/EntityDrawSystem.cs. See EntityUpdateSystem.hpp
// (and UpdateSystem.hpp's header comment) for the virtual-inheritance/diamond note that applies
// here identically.
#pragma once

#include "CNA/Extended/ECS/Systems/DrawSystem.hpp"
#include "CNA/Extended/ECS/Systems/EntitySystem.hpp"

namespace CNA::Extended::ECS::Systems
{
    /** @brief An EntitySystem that also draws once per frame. */
    class EntityDrawSystem : public EntitySystem, public IDrawSystem
    {
    public:
        explicit EntityDrawSystem(AspectBuilder aspectBuilder) : EntitySystem(std::move(aspectBuilder)) {}
        ~EntityDrawSystem() override = default;

        void Dispose() override { EntitySystem::Dispose(); }
        void Initialize(World& world) override { EntitySystem::Initialize(world); }

        void Draw(const GameTime& gameTime) override = 0;
    };
}
