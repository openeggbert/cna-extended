// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/TextureRegion2DJsonConverter.hpp"

#include <stdexcept>

namespace CNA::Extended::Serialization::Json
{
    TextureRegion2DJsonConverter::TextureRegion2DJsonConverter(ITextureRegionService* textureRegionService)
        : textureRegionService_(textureRegionService)
    {
        if (textureRegionService_ == nullptr)
        {
            throw std::invalid_argument("textureRegionService must not be null.");
        }
    }

    std::shared_ptr<Texture2DRegion> TextureRegion2DJsonConverter::Read(const nlohmann::ordered_json& j) const
    {
        const std::string regionName = j.get<std::string>();
        return regionName.empty() ? nullptr : textureRegionService_->GetTextureRegion(regionName);
    }

    void TextureRegion2DJsonConverter::Write(nlohmann::ordered_json& j, const std::shared_ptr<Texture2DRegion>& value) const
    {
        if (!value)
        {
            throw std::invalid_argument("value must not be null.");
        }
        j = value->getNameProperty();
    }
}
