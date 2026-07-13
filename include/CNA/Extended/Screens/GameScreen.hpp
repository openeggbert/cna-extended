// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Screens/GameScreen.cs: a Screen with convenient access to the
// owning Game's ContentManager/GraphicsDevice/GameServiceContainer. `Game game` (a nullable
// reference-type parameter, guarded by ArgumentNullException.ThrowIfNull) -> `Game*`, since a
// C++ reference parameter couldn't represent the null case this project's tests exercise
// (Constructor_WithNullGame_ThrowsArgumentNullException).
#pragma once

#include "CNA/Extended/Screens/Screen.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameServiceContainer.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

namespace CNA::Extended::Screens
{
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::GameServiceContainer;

    /** @brief A Screen with convenient access to the owning Game's core services (Content, GraphicsDevice, Services). */
    class GameScreen : public Screen
    {
    public:
        /** @brief Gets the game instance associated with this screen. */
        [[nodiscard]] Game& getGameProperty() const { return *game_; }

        /** @brief Gets the content manager for loading game assets. */
        [[nodiscard]] Microsoft::Xna::Framework::Content::ContentManager& getContentProperty() const { return game_->getContentProperty(); }

        /** @brief Gets the graphics device for rendering operations. */
        [[nodiscard]] Microsoft::Xna::Framework::Graphics::GraphicsDevice& getGraphicsDeviceProperty() const { return game_->getGraphicsDeviceProperty(); }

        /** @brief Gets the service container for accessing registered game services. */
        [[nodiscard]] GameServiceContainer& getServicesProperty() const { return game_->getServicesProperty(); }

    protected:
        /**
         * @brief Initializes a new GameScreen bound to the specified game instance.
         * @param game The game instance that provides access to core services. Must not be null.
         * @throws std::invalid_argument game is null.
         */
        explicit GameScreen(Game* game);

    private:
        Game* game_;
    };
}
