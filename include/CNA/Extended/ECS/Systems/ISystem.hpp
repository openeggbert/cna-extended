// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/Systems/ISystem.cs.
#pragma once

#include "System/IDisposable.hpp"

namespace CNA::Extended::ECS
{
    class World;
}

namespace CNA::Extended::ECS::Systems
{
    /** @brief Base interface for anything World can register and initialize (update and/or draw systems). */
    class ISystem : public System::IDisposable
    {
    public:
        ~ISystem() override = default;

        /** @brief Called once when this system is registered with @p world. */
        virtual void Initialize(World& world) = 0;
    };
}
