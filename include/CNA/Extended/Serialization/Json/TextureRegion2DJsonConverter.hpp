// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/TextureRegion2DJsonConverter.cs. Unlike the
// stateless value-type converters in this module (Color/Vector2/RectangleF/etc., all
// `nlohmann::adl_serializer<T>` specializations), this needs a caller-supplied
// `ITextureRegionService&` to resolve a region name to an actual `Texture2DRegion` -- there is no
// way to inject that into nlohmann's fixed-signature ADL hooks. Ported instead as an explicit
// class the caller constructs and invokes directly, matching C#'s per-instance `JsonConverter<T>`
// object shape as closely as this project's JSON model allows.
#pragma once

#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "CNA/Extended/Serialization/Json/TextureRegionService.hpp"
#include "nlohmann/json.hpp"

#include <memory>

namespace CNA::Extended::Serialization::Json
{
    using CNA::Extended::Graphics::Texture2DRegion;

    /** @brief Converts a Texture2DRegion to/from its name, resolved through an ITextureRegionService. */
    class TextureRegion2DJsonConverter
    {
    public:
        /** @throws std::invalid_argument textureRegionService is null. */
        explicit TextureRegion2DJsonConverter(ITextureRegionService* textureRegionService);

        /** @brief Reads @p j as a region name and resolves it. Returns nullptr for an empty name. */
        [[nodiscard]] std::shared_ptr<Texture2DRegion> Read(const nlohmann::ordered_json& j) const;

        /** @brief Writes @p value's Name. @throws std::invalid_argument value is null. */
        void Write(nlohmann::ordered_json& j, const std::shared_ptr<Texture2DRegion>& value) const;

    private:
        ITextureRegionService* textureRegionService_;
    };
}
