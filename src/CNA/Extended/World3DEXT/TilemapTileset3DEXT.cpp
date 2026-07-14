// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/TilemapTileset3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    void TilemapTileset3DEXT::SetTileTextureEXT(int tileId, Texture2D* texture)
    {
        texturesByTileIdEXT_[tileId] = texture;
    }

    Texture2D* TilemapTileset3DEXT::GetTileTextureEXT(int tileId) const
    {
        const auto it = texturesByTileIdEXT_.find(tileId);
        return it != texturesByTileIdEXT_.end() ? it->second : nullptr;
    }
}
