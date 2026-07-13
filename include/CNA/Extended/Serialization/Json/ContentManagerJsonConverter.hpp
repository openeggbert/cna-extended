// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/ContentManagerJsonConverter.cs. Generic,
// stateful (needs a ContentManager& and an asset-name-getter function) -- ported as an explicit
// class template per TextureRegion2DJsonConverter.hpp's header-comment rationale, not an
// nlohmann::adl_serializer<T> specialization. `Microsoft::Xna::Framework::Content::ContentManager`
// is CNA's normal runtime ContentManager (not the excluded xnb-registration ExtendedContentManager).
#pragma once

#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "nlohmann/json.hpp"

#include <functional>
#include <string>

namespace CNA::Extended::Serialization::Json
{
    using Microsoft::Xna::Framework::Content::ContentManager;

    /** @brief Loads/writes a T as its content asset name, via ContentManager::Load<T>. */
    template <typename T>
    class ContentManagerJsonConverter
    {
    public:
        ContentManagerJsonConverter(ContentManager& contentManager, std::function<std::string(const T&)> getAssetName)
            : contentManager_(&contentManager), getAssetName_(std::move(getAssetName))
        {
        }

        [[nodiscard]] T Read(const nlohmann::ordered_json& j) const
        {
            return contentManager_->template Load<T>(j.template get<std::string>());
        }

        void Write(nlohmann::ordered_json& j, const T& value) const { j = getAssetName_(value); }

    private:
        ContentManager* contentManager_;
        std::function<std::string(const T&)> getAssetName_;
    };
}
