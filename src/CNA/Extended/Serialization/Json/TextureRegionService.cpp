// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/TextureRegionService.hpp"

namespace CNA::Extended::Serialization::Json
{
    std::shared_ptr<Texture2DRegion> TextureRegionService::GetTextureRegion(const std::string& name)
    {
        for (Texture2DAtlas& atlas : textureAtlases_)
        {
            std::shared_ptr<Texture2DRegion> region;
            if (atlas.TryGetRegion(name, region))
            {
                return region;
            }
        }
        return nullptr;
    }
}
