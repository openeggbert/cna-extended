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

    /** @brief Accumulates ISystems, then builds a World that owns and initializes all of them. */
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
