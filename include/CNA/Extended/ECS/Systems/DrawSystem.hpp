// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/Systems/DrawSystem.cs.
#pragma once

#include "CNA/Extended/ECS/Systems/ISystem.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"

namespace CNA::Extended::ECS::Systems
{
    using Microsoft::Xna::Framework::GameTime;

    /** @brief An ISystem that draws once per frame. See UpdateSystem.hpp's header comment for why ISystem is inherited virtually here. */
    class IDrawSystem : public virtual ISystem
    {
    public:
        ~IDrawSystem() override = default;

        /** @brief Draws this system's visuals for the current frame. */
        virtual void Draw(const GameTime& gameTime) = 0;
    };

    /** @brief Convenience base for an IDrawSystem with no-op Dispose/Initialize defaults. */
    class DrawSystem : public IDrawSystem
    {
    public:
        ~DrawSystem() override = default;

        void Dispose() override {}
        void Initialize(World& world) override { (void)world; }
        void Draw(const GameTime& gameTime) override = 0;
    };
}
