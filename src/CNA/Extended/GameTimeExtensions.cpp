// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/GameTimeExtensions.hpp"

namespace CNA::Extended
{
    float GetElapsedSeconds(const GameTime& gameTime)
    {
        return static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
    }
}
