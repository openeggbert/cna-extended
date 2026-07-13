// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/Systems/EntityUpdateSystem.cs.
#pragma once

#include "CNA/Extended/ECS/Systems/EntitySystem.hpp"
#include "CNA/Extended/ECS/Systems/UpdateSystem.hpp"

namespace CNA::Extended::ECS::Systems
{
    /** @brief An EntitySystem that also updates once per frame. */
    class EntityUpdateSystem : public EntitySystem, public IUpdateSystem
    {
    public:
        explicit EntityUpdateSystem(AspectBuilder aspectBuilder) : EntitySystem(std::move(aspectBuilder)) {}
        ~EntityUpdateSystem() override = default;

        // Disambiguate: both EntitySystem (via ISystem) and IUpdateSystem (via ISystem) inherit
        // Dispose()/Initialize(World&) from the same ISystem base -- diamond inheritance with a
        // shared root, resolved the same way GameComponent's own multi-interface bases are
        // resolved elsewhere in this port: forward to EntitySystem's implementation explicitly.
        void Dispose() override { EntitySystem::Dispose(); }
        void Initialize(World& world) override { EntitySystem::Initialize(world); }

        void Update(const GameTime& gameTime) override = 0;
    };
}
