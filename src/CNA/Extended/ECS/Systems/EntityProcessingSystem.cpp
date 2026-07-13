// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ECS/Systems/EntityProcessingSystem.hpp"

namespace CNA::Extended::ECS::Systems
{
    void EntityProcessingSystem::Update(const GameTime& gameTime)
    {
        Begin();

        for (int entityId : getActiveEntitiesProperty())
        {
            Process(gameTime, entityId);
        }

        End();
    }
}
