// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/Tilemap3DFileContent.hpp"

namespace CNA::Extended::World3DEXT
{
    void from_json(const nlohmann::ordered_json& j, Tilemap3DFileTileSize& value)
    {
        value.x_ = j.at("x").get<float>();
        value.y_ = j.at("y").get<float>();
        value.z_ = j.at("z").get<float>();
    }

    void from_json(const nlohmann::ordered_json& j, Tilemap3DFileContent& value)
    {
        value.width_ = j.at("width").get<int>();
        value.height_ = j.at("height").get<int>();
        value.depth_ = j.at("depth").get<int>();
        value.tileSize_ = j.at("tileSize").get<Tilemap3DFileTileSize>();
        value.tileIds_ = j.at("tileIds").get<std::vector<int>>();
    }
}
