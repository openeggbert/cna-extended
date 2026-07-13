// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Properties/TilemapPropertyType.cs. Despite living
// under the Properties/ folder upstream, its declared namespace is the root
// `MonoGame.Extended.Tilemaps` -- ported into the matching root `CNA::Extended::Tilemaps`
// namespace and a flat file path, per this project's "namespace follows the actual C#
// declaration, not the source folder" convention (see Shapes::Polygon's own header comment for
// the established precedent).
#pragma once

namespace CNA::Extended::Tilemaps
{
    /** @brief Identifies the kind of value stored in a TilemapPropertyValue. */
    enum class TilemapPropertyType
    {
        String,
        Int,
        Float,
        Bool,
        Color,
        File,
        Object
    };
}
