// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/MonoGameJsonSerializerOptionsProvider.hpp"

#include "System/Text/Json/JsonNamingPolicy.hpp"

namespace CNA::Extended::Serialization::Json
{
    System::Text::Json::JsonSerializerOptions GetOptions(ContentManager& contentManager, const std::string& contentPath)
    {
        // Kept in the signature for call-site fidelity with upstream; unused now that
        // TextureAtlasJsonConverter (the one converter upstream registered here that actually
        // needed them) is invoked directly by callers instead of through this options object --
        // see this file's header comment.
        (void)contentManager;
        (void)contentPath;

        System::Text::Json::JsonSerializerOptions options;
        options.setWriteIndentedProperty(true);
        options.setPropertyNamingPolicyProperty(System::Text::Json::JsonNamingPolicy::CamelCase());
        return options;
    }
}
