// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Content/TexturePacker/TexturePackerFileContent.hpp"

namespace CNA::Extended::Content::TexturePacker
{
    void from_json(const nlohmann::ordered_json& j, TexturePackerPoint& value)
    {
        value.x_ = j.at("x").get<int>();
        value.y_ = j.at("y").get<int>();
    }

    void from_json(const nlohmann::ordered_json& j, TexturePackerPointF& value)
    {
        value.x_ = j.at("x").get<double>();
        value.y_ = j.at("y").get<double>();
    }

    void from_json(const nlohmann::ordered_json& j, TexturePackerSize& value)
    {
        value.width_ = j.at("w").get<int>();
        value.height_ = j.at("h").get<int>();
    }

    void from_json(const nlohmann::ordered_json& j, TexturePackerRectangle& value)
    {
        value.x_ = j.at("x").get<int>();
        value.y_ = j.at("y").get<int>();
        value.width_ = j.at("w").get<int>();
        value.height_ = j.at("h").get<int>();
    }

    void from_json(const nlohmann::ordered_json& j, TexturePackerTextureFrame& value)
    {
        value.frame_ = j.at("frame").get<TexturePackerRectangle>();
        value.rotated_ = j.value("rotated", 0);
        if (j.contains("size") && !j.at("size").is_null())
        {
            value.size_ = j.at("size").get<TexturePackerSize>();
        }
        if (j.contains("offset") && !j.at("offset").is_null())
        {
            value.offset_ = j.at("offset").get<TexturePackerPoint>();
        }
        if (j.contains("pivot") && !j.at("pivot").is_null())
        {
            value.pivot_ = j.at("pivot").get<TexturePackerPointF>();
        }
        if (j.contains("scale9") && !j.at("scale9").is_null())
        {
            value.scale9_ = j.at("scale9").get<TexturePackerRectangle>();
        }
    }

    void from_json(const nlohmann::ordered_json& j, TexturePackerTexture& value)
    {
        value.fileName_ = j.at("filename").get<std::string>();
        if (j.contains("format") && !j.at("format").is_null())
        {
            value.format_ = j.at("format").get<std::string>();
        }
        if (j.contains("size") && !j.at("size").is_null())
        {
            value.size_ = j.at("size").get<TexturePackerSize>();
        }
        if (j.contains("scale") && !j.at("scale").is_null())
        {
            value.scale_ = j.at("scale").get<std::string>();
        }
        value.premultiplied_ = j.value("premultiplied", false);
        if (j.contains("frames") && !j.at("frames").is_null())
        {
            System::Collections::Generic::Dictionary<std::string, TexturePackerTextureFrame> frames;
            for (const auto& [key, frameJson] : j.at("frames").items())
            {
                frames.Add(key, frameJson.get<TexturePackerTextureFrame>());
            }
            value.frames_ = std::move(frames);
        }
    }

    void from_json(const nlohmann::ordered_json& j, TexturePackerFrame& value)
    {
        value.fileName_ = j.at("filename").get<std::string>();
        value.frame_ = j.at("frame").get<TexturePackerRectangle>();
        value.rotated_ = j.at("rotated").get<bool>();
        value.trimmed_ = j.at("trimmed").get<bool>();
        value.spriteSourceSize_ = j.at("spriteSourceSize").get<TexturePackerRectangle>();
        value.sourceSize_ = j.at("sourceSize").get<TexturePackerSize>();
        value.pivotPoint_ = j.at("pivot").get<TexturePackerPointF>();
    }

    void from_json(const nlohmann::ordered_json& j, TexturePackerMeta& value)
    {
        value.app_ = j.at("app").get<std::string>();
        value.version_ = j.at("version").get<std::string>();
        value.image_ = j.at("image").get<std::string>();
        value.dataFormat_ = j.at("dataformat").get<std::string>();
        value.smartUpdate_ = j.at("smartupdate").get<std::string>();
    }

    void from_json(const nlohmann::ordered_json& j, TexturePackerFileContent& value)
    {
        value.regions_ = j.at("frames").get<std::vector<TexturePackerFrame>>();
        value.textures_ = j.at("textures").get<std::vector<TexturePackerTexture>>();
        value.meta_ = j.at("meta").get<TexturePackerMeta>();
    }
}
