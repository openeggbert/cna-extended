// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Screens/GameScreen.hpp"

#include <stdexcept>

namespace CNA::Extended::Screens
{
    GameScreen::GameScreen(Game* game) : game_(game)
    {
        if (game_ == nullptr)
        {
            throw std::invalid_argument("game must not be null.");
        }
    }
}
