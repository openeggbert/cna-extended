// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/WorldBuilder.cs. Upstream's `Build()` returns a `World`
// (a GC-managed reference type, freely copyable-by-reference). `World` here owns a
// `std::vector<std::unique_ptr<Systems::ISystem>>`, making it move-only at best -- returning it by
// value would depend on the compiler eliding the move (guaranteed in some cases, not all) and adds
// no value over the more idiomatic C++ alternative: `Build()` returns `std::unique_ptr<World>`,
// giving the caller clear, singular ownership of the constructed World, matching how a `World` is
// actually used in practice (built once, owned for the rest of its lifetime, never copied).
#pragma once

#include "CNA/Extended/ECS/Systems/ISystem.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::ECS
{
    class World;

    /**
     * @brief Accumulates ISystems, then builds a World that owns and initializes all of them.
     * This is the normal way to construct a World -- prefer it over calling World's own
     * constructor directly, since a World with no registered systems can create/destroy entities
     * and attach components but never actually processes them.
     *
     * @see World, the container this builds; @see Entity, Systems::ISystem for what gets created
     * and registered, respectively.
     * @code
     * #include <CNA/Extended/ECS/Entity.hpp>
     * #include <CNA/Extended/ECS/World.hpp>
     * #include <CNA/Extended/ECS/WorldBuilder.hpp>
     * #include <Microsoft/Xna/Framework/GameTime.hpp>
     *
     * using CNA::Extended::ECS::Entity;
     * using CNA::Extended::ECS::World;
     * using CNA::Extended::ECS::WorldBuilder;
     * using Microsoft::Xna::Framework::GameTime;
     *
     * struct Position { float X = 0.0f; float Y = 0.0f; };
     *
     * void RunOneFrame()
     * {
     *     std::unique_ptr<World> world = WorldBuilder().Build();
     *
     *     Entity& entity = world->CreateEntity();
     *     Position position;
     *     entity.Attach(&position); // caller keeps position alive for the entity's lifetime
     *
     *     GameTime gameTime;
     *     world->Update(gameTime); // processes any registered IUpdateSystems
     * }
     * @endcode
     */
    class WorldBuilder
    {
    public:
        WorldBuilder() = default;

        /** @brief Adds @p system (transferring ownership) to be registered when Build() is called. Returns *this for chaining. */
        WorldBuilder& AddSystem(std::unique_ptr<Systems::ISystem> system);

        /** @brief Builds a new World, registering every system added via AddSystem in the order they were added. */
        [[nodiscard]] std::unique_ptr<World> Build();

    private:
        std::vector<std::unique_ptr<Systems::ISystem>> systems_;
    };
}
