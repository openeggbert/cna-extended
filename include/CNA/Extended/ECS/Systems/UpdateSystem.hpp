// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/Systems/UpdateSystem.cs.
//
// `ISystem` is inherited *virtually* here (and by `IDrawSystem`, and by `EntitySystem`):
// `EntityUpdateSystem : EntitySystem, IUpdateSystem` (see EntityUpdateSystem.hpp) reaches
// `ISystem` via two separate paths -- through `EntitySystem` and through `IUpdateSystem` -- the
// same way C#'s interface model always resolves a diamond-shaped interface hierarchy to one
// shared set of members regardless of how many paths reach it. C++ needs `virtual` inheritance
// spelled out explicitly to get that same single-shared-base behavior; without it,
// `EntityUpdateSystem` would have two distinct, ambiguous `ISystem` subobjects and
// `dynamic_cast<IUpdateSystem*>`/`dynamic_cast<IDrawSystem*>` (used by `World::RegisterSystem` to
// sort systems into the update/draw bags) would not behave correctly.
#pragma once

#include "CNA/Extended/ECS/Systems/ISystem.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"

namespace CNA::Extended::ECS::Systems
{
    using Microsoft::Xna::Framework::GameTime;

    /** @brief An ISystem that updates once per frame. */
    class IUpdateSystem : public virtual ISystem
    {
    public:
        ~IUpdateSystem() override = default;

        /** @brief Advances this system's logic by one frame. */
        virtual void Update(const GameTime& gameTime) = 0;
    };

    /** @brief Convenience base for an IUpdateSystem with no-op Dispose/Initialize defaults. */
    class UpdateSystem : public IUpdateSystem
    {
    public:
        ~UpdateSystem() override = default;

        void Dispose() override {}
        void Initialize(World& world) override { (void)world; }
        void Update(const GameTime& gameTime) override = 0;
    };
}
