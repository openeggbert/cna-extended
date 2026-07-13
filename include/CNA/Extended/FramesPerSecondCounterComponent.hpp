// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's FramesPerSecondCounterComponent.cs. A thin DrawableGameComponent
// wrapper around FramesPerSecondCounter, forwarding Update/Draw. Requires a live Game (and, once
// drawn, a GraphicsDevice) -- matching CNA's own DrawableGameComponent, which its own test suite
// does not unit test for the same reason (see cna's tests/Microsoft/Xna/Framework/
// DrawableGameComponentTests.cpp: "No tests: DrawableGameComponent requires a live Game and
// GraphicsDevice (SDL/GPU)."). All the actual counting logic lives in FramesPerSecondCounter
// (fully unit tested, no live Game needed); this wrapper itself has no independently-testable
// logic beyond what a live-Game integration test would exercise, so it is ported without a
// dedicated test file, matching the precedent CNA itself already set for this exact situation.
#pragma once

#include "CNA/Extended/FramesPerSecondCounter.hpp"
#include "Microsoft/Xna/Framework/DrawableGameComponent.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::DrawableGameComponent;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::GameTime;

    /** @brief A DrawableGameComponent that tracks and exposes the current frames-per-second count. */
    class FramesPerSecondCounterComponent : public DrawableGameComponent
    {
    public:
        /** @brief Creates the component, attaching it to the specified game. */
        explicit FramesPerSecondCounterComponent(Game& game);

        /** @brief Gets the number of frames drawn during the most recently completed one-second interval. */
        [[nodiscard]] int getFramesPerSecondProperty() const;

        void Update(GameTime& gameTime) override;
        void Draw(const GameTime& gameTime) override;

        [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        FramesPerSecondCounter fpsCounter_;
    };
}
