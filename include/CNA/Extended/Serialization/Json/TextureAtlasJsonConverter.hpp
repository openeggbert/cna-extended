// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/TextureAtlasJsonConverter.cs. Stateful (needs
// a ContentManager& and a path), so ported as an explicit class per
// TextureRegion2DJsonConverter.hpp's header-comment rationale, not an
// nlohmann::adl_serializer<T> specialization.
//
// Fidelity note: upstream's `Read` has two branches. The JSON-string branch (a bare asset-name
// reference) is genuinely broken in upstream itself -- its own source carries the comment
// "TODO: (Aristurtle 05/20/2024) What is this for? It's just an if block that throws an
// exception. Need to investigate." followed by dead setup code and an unconditional
// `throw new NotImplementedException()`. Preserved as-is (throws, does not attempt to make it
// work) per this project's port-faithfully mandate -- this is a real upstream bug, not something
// to silently "fix" or skip. The JSON-object branch (an inline `{texture, regionWidth,
// regionHeight}` atlas definition) is complete and functional upstream and is ported faithfully.
//
// Ownership note (a genuine C#-GC-vs-C++ divergence, not present in the upstream source): the
// returned `Texture2DAtlas` holds a non-owning `Texture2D*` into the texture it loads (matching
// this project's established `Texture2DAtlas` ownership convention). Upstream relies on
// `ContentManager` itself keeping the loaded `Texture2D` reference alive for as long as needed;
// this port's `ContentManager::Load<T>()` returns `Texture2D` by value, so nothing outside this
// converter would otherwise keep it alive. `TextureAtlasJsonConverter` therefore owns every
// texture it loads (in `std::vector<std::unique_ptr<Texture2D>>`, matching `BitmapFont`'s
// established precedent for the same problem) -- callers must keep the converter instance alive
// for as long as any `Texture2DAtlas` it produced is still in use.
#pragma once

#include "CNA/Extended/Content/ContentManagerExtensions.hpp"
#include "CNA/Extended/Graphics/Texture2DAtlas.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "nlohmann/json.hpp"

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::Serialization::Json
{
    using CNA::Extended::Content::ContentManager;
    using CNA::Extended::Graphics::Texture2DAtlas;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    /** @brief Converts a Texture2DAtlas from either a bare asset-name string (currently unimplemented, matching upstream) or an inline `{texture, regionWidth, regionHeight}` object. */
    class TextureAtlasJsonConverter
    {
    public:
        TextureAtlasJsonConverter(ContentManager& contentManager, std::string path);

        /**
         * @brief Reads @p j, loading and taking ownership of the referenced texture.
         * @throws System::NotImplementedException @p j is a JSON string (see this file's header comment).
         */
        [[nodiscard]] Texture2DAtlas Read(const nlohmann::ordered_json& j);

    private:
        [[nodiscard]] std::string GetContentPath(const std::string& relativePath) const;

        ContentManager* contentManager_;
        std::string path_;
        std::vector<std::unique_ptr<Texture2D>> textures_;
    };
}
