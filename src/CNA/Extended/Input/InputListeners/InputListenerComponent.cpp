// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/InputListenerComponent.hpp"

#include "CNA/Extended/Input/InputListeners/GamePadListener.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::GameComponent;

    InputListenerComponent::InputListenerComponent(Game& game) : GameComponent(game)
    {
    }

    InputListenerComponent::InputListenerComponent(Game& game, std::vector<std::unique_ptr<InputListener>> listeners)
        : GameComponent(game), listeners_(std::move(listeners))
    {
    }

    void InputListenerComponent::Update(GameTime& gameTime)
    {
        GameComponent::Update(gameTime);

        if (getGameProperty().getIsActiveProperty())
        {
            for (auto& listener : listeners_)
            {
                listener->Update(gameTime);
            }
        }

        GamePadListener::CheckConnections();
    }
}
