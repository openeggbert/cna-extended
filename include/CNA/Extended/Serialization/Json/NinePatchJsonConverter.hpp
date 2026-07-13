// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/NinePatchJsonConverter.cs. Stateful (needs an
// ITextureRegionService), so ported as an explicit class per TextureRegion2DJsonConverter.hpp's
// header-comment rationale, not an nlohmann::adl_serializer<T> specialization.
#pragma once

#include "CNA/Extended/Graphics/NinePatch.hpp"
#include "CNA/Extended/Serialization/Json/TextureRegionService.hpp"
#include "nlohmann/json.hpp"

#include <memory>

namespace CNA::Extended::Serialization::Json
{
    using CNA::Extended::Graphics::NinePatch;

    /** @brief Converts a NinePatch to/from `{"TextureRegion": name, "Padding": "l t r b"}`, resolved through an ITextureRegionService. */
    class NinePatchJsonConverter
    {
    public:
        explicit NinePatchJsonConverter(ITextureRegionService* textureRegionService);

        /** @throws std::invalid_argument @p j is not an object, or is missing "Padding"/"TextureRegion". */
        [[nodiscard]] std::shared_ptr<NinePatch> Read(const nlohmann::ordered_json& j) const;

        void Write(nlohmann::ordered_json& j, const std::shared_ptr<NinePatch>& value) const;

    private:
        ITextureRegionService* textureRegionService_;
    };
}
