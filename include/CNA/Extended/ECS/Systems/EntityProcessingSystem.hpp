// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/Systems/EntityProcessingSystem.cs.
#pragma once

#include "CNA/Extended/ECS/Systems/EntityUpdateSystem.hpp"

namespace CNA::Extended::ECS::Systems
{
    /** @brief An EntityUpdateSystem that calls Process() once per active entity each Update, bracketed by Begin()/End(). */
    class EntityProcessingSystem : public EntityUpdateSystem
    {
    public:
        explicit EntityProcessingSystem(AspectBuilder aspectBuilder) : EntityUpdateSystem(std::move(aspectBuilder)) {}
        ~EntityProcessingSystem() override = default;

        void Update(const GameTime& gameTime) override;

        /** @brief Called once before processing entities each Update. */
        virtual void Begin() {}
        /** @brief Called once for each active entity, in Update order. */
        virtual void Process(const GameTime& gameTime, int entityId) = 0;
        /** @brief Called once after processing all entities each Update. */
        virtual void End() {}
    };
}
