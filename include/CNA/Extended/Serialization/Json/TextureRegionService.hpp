// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/TextureRegionService.cs.
// `IList<Texture2DAtlas>` -> `std::vector<Texture2DAtlas>`; `Texture2DRegion`/`Texture2DAtlas`
// stay `std::shared_ptr`-managed per this project's established ownership convention.
#pragma once

#include "CNA/Extended/Graphics/Texture2DAtlas.hpp"
#include "CNA/Extended/Graphics/Texture2DRegion.hpp"

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::Serialization::Json
{
    using CNA::Extended::Graphics::Texture2DAtlas;
    using CNA::Extended::Graphics::Texture2DRegion;

    /** @brief Locates a Texture2DRegion by name across a set of registered Texture2DAtlas instances. */
    class ITextureRegionService
    {
    public:
        virtual ~ITextureRegionService() = default;

        /** @brief Returns the named region, or nullptr if no registered atlas contains it. */
        [[nodiscard]] virtual std::shared_ptr<Texture2DRegion> GetTextureRegion(const std::string& name) = 0;
    };

    /** @brief Default ITextureRegionService: a flat, ordered list of Texture2DAtlas instances searched in order. */
    class TextureRegionService : public ITextureRegionService
    {
    public:
        TextureRegionService() = default;

        /** @brief The atlases searched, in order, by GetTextureRegion. */
        [[nodiscard]] std::vector<Texture2DAtlas>& getTextureAtlasesProperty() { return textureAtlases_; }
        /** @copydoc getTextureAtlasesProperty() */
        [[nodiscard]] const std::vector<Texture2DAtlas>& getTextureAtlasesProperty() const { return textureAtlases_; }

        [[nodiscard]] std::shared_ptr<Texture2DRegion> GetTextureRegion(const std::string& name) override;

    private:
        std::vector<Texture2DAtlas> textureAtlases_;
    };
}
