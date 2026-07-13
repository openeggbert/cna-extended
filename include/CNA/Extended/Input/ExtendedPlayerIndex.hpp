// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/ExtendedPlayerIndex.cs. Upstream's first 4 values are
// assigned from Microsoft.Xna.Framework.PlayerIndex (One-Four); CNA's own
// Microsoft::Xna::Framework::PlayerIndex enum has exactly those same 4 values (0-3), so the
// numeric values below match both upstream and CNA's PlayerIndex exactly.
#pragma once

namespace CNA::Extended::Input
{
    /** @brief Player index enumeration with slots for 8 players. */
    enum class ExtendedPlayerIndex
    {
        /** @brief First player. */
        One = 0,

        /** @brief Second player. */
        Two = 1,

        /** @brief Third player. */
        Three = 2,

        /** @brief Fourth player. */
        Four = 3,

        /** @brief Fifth player. */
        Five,

        /** @brief Sixth player. */
        Six,

        /** @brief Seventh player. */
        Seven,

        /** @brief Eighth player. */
        Eight
    };
}
