// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/NinePatchJsonConverter.hpp"

#include "CNA/Extended/Graphics/Texture2DRegionExtensions.hpp"
#include "CNA/Extended/Thickness.hpp"

#include <stdexcept>

namespace CNA::Extended::Serialization::Json
{
    NinePatchJsonConverter::NinePatchJsonConverter(ITextureRegionService* textureRegionService)
        : textureRegionService_(textureRegionService)
    {
    }

    std::shared_ptr<NinePatch> NinePatchJsonConverter::Read(const nlohmann::ordered_json& j) const
    {
        if (!j.is_object())
        {
            throw std::invalid_argument("Expected a JSON object.");
        }

        std::string padding;
        std::string regionName;

        if (j.contains("Padding"))
        {
            padding = j.at("Padding").get<std::string>();
        }
        if (j.contains("TextureRegion"))
        {
            regionName = j.at("TextureRegion").get<std::string>();
        }

        if (padding.empty() || regionName.empty())
        {
            throw std::invalid_argument("Missing required properties \"Padding\" and \"TextureRegion\"");
        }

        const Thickness thickness = Thickness::Parse(padding);
        const std::shared_ptr<Texture2DRegion> region = textureRegionService_->GetTextureRegion(regionName);
        return CreateNinePatch(region, thickness);
    }

    void NinePatchJsonConverter::Write(nlohmann::ordered_json& j, const std::shared_ptr<NinePatch>& value) const
    {
        if (!value)
        {
            j = nullptr;
            return;
        }

        j = nlohmann::ordered_json::object();
        j["TextureRegion"] = value->getNameProperty();
        j["Padding"] = value->getPaddingProperty().ToString();
    }
}
