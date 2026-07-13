// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's GameTimeExtensions.cs. C# extension method on
// Microsoft.Xna.Framework.GameTime -> a free function taking GameTime& in this namespace,
// matching the convention used throughout this project (see RandomExtensions.hpp).
#pragma once

#include "Microsoft/Xna/Framework/GameTime.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::GameTime;

    /** @brief Gets the number of seconds elapsed since the last update. */
    [[nodiscard]] float GetElapsedSeconds(const GameTime& gameTime);
}
