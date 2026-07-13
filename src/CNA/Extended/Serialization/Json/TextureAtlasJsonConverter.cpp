// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/TextureAtlasJsonConverter.hpp"

#include "CNA/Extended/Content/TexturePacker/TexturePackerFileContent.hpp"
#include "CNA/Extended/Serialization/Json/JsonContentLoader.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/IO/Path.hpp"
#include "System/NotImplementedException.hpp"

namespace CNA::Extended::Serialization::Json
{
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    namespace
    {
        struct InlineTextureAtlas
        {
            std::string texture;
            int regionWidth = 0;
            int regionHeight = 0;
        };

        void from_json(const nlohmann::ordered_json& j, InlineTextureAtlas& value)
        {
            value.texture = j.at("Texture").get<std::string>();
            value.regionWidth = j.at("RegionWidth").get<int>();
            value.regionHeight = j.at("RegionHeight").get<int>();
        }
    }

    TextureAtlasJsonConverter::TextureAtlasJsonConverter(ContentManager& contentManager, std::string path)
        : contentManager_(&contentManager), path_(std::move(path))
    {
    }

    std::string TextureAtlasJsonConverter::GetContentPath(const std::string& relativePath) const
    {
        const std::string directory = System::IO::Path::GetDirectoryName(path_);
        return System::IO::Path::Combine(directory, relativePath);
    }

    Texture2DAtlas TextureAtlasJsonConverter::Read(const nlohmann::ordered_json& j)
    {
        if (j.is_string())
        {
            // Genuinely unimplemented upstream -- see this file's header comment. Preserved as-is.
            const std::string textureAtlasAssetName = j.get<std::string>();
            const std::string contentPath = GetContentPath(textureAtlasAssetName);
            const CNA::Extended::Content::TexturePacker::TexturePackerFileContent texturePackerFile =
                CNA::Extended::Content::Load<CNA::Extended::Content::TexturePacker::TexturePackerFileContent>(
                    *contentManager_, contentPath, *std::make_unique<JsonContentLoader>());
            (void)texturePackerFile;
            throw System::NotImplementedException();
        }

        const InlineTextureAtlas metadata = j.get<InlineTextureAtlas>();

        const std::string textureName = System::IO::Path::GetFileNameWithoutExtension(metadata.texture);
        const std::string textureDirectory = System::IO::Path::GetDirectoryName(metadata.texture);
        const std::string directory = System::IO::Path::GetDirectoryName(path_);
        const std::string relativePath =
            System::IO::Path::Combine(contentManager_->getRootDirectoryProperty(), System::IO::Path::Combine(directory, System::IO::Path::Combine(textureDirectory, textureName)));
        const std::string resolvedAssetName = System::IO::Path::GetFullPath(relativePath);

        Texture2D loaded = [&]() {
            try
            {
                return contentManager_->Load<Texture2D>(resolvedAssetName);
            }
            catch (const std::exception&)
            {
                return textureDirectory.empty() ? contentManager_->Load<Texture2D>(textureName)
                                                 : contentManager_->Load<Texture2D>(textureDirectory + "/" + textureName);
            }
        }();

        textures_.push_back(std::make_unique<Texture2D>(std::move(loaded)));
        return Texture2DAtlas::Create(resolvedAssetName, textures_.back().get(), metadata.regionWidth, metadata.regionHeight);
    }
}
