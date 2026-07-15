// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Tilemap3DFileContent -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design (Phase 12 C, 2026-07-15,
// user-requested "extend later" item: a real file-format reader for Tilemap3DFactoryEXT).
//
// Tilemap3DFactoryEXT.hpp's own header comment explains why no existing MonoGame.Extended
// map format (Tiled/Ogmo/LDtk) applies here -- all three are 2D-only grid formats with no
// voxel/3D-grid concept. This is a project-defined JSON schema instead, using the exact same
// dense flat row-major (X fastest, then Y, then Z) tile-ID array shape
// Tilemap3DFactoryEXT::BuildFromArrayEXT already takes as in-memory data (user-confirmed
// 2026-07-15, chosen over a sparse {x,y,z,id} tile-list schema for consistency with the
// existing in-memory shape and lower risk), so BuildFromJsonFileEXT/BuildFromJsonStreamEXT
// can delegate straight to BuildFromArrayEXT with zero duplicated validation logic.
//
// Follows TexturePackerFileContent.hpp's established pattern for this project's own JSON
// file formats: get-only getXProperty() classes, each with a `friend from_json` free
// function providing the nlohmann-json ADL customization point (see that file's own header
// comment for why sharp-runtime's JsonSerializer::Deserialize<T> uses this instead of
// reflection).
//
// Example file:
//   {
//     "width": 4, "height": 2, "depth": 4,
//     "tileSize": { "x": 1.0, "y": 1.0, "z": 1.0 },
//     "tileIds": [1, 1, 0, 0, 1, 1, 0, 0, ...]
//   }
#pragma once

#include "nlohmann/json.hpp"

#include <vector>

namespace CNA::Extended::World3DEXT
{
    /** @brief A (x, y, z) world-space tile size, as found in a Tilemap3DEXT JSON file's "tileSize" field. */
    class Tilemap3DFileTileSize
    {
    public:
        Tilemap3DFileTileSize() = default;
        Tilemap3DFileTileSize(float x, float y, float z) : x_(x), y_(y), z_(z) {}

        [[nodiscard]] float getXProperty() const { return x_; }
        [[nodiscard]] float getYProperty() const { return y_; }
        [[nodiscard]] float getZProperty() const { return z_; }

    private:
        float x_ = 1.0f;
        float y_ = 1.0f;
        float z_ = 1.0f;

        friend void from_json(const nlohmann::ordered_json& j, Tilemap3DFileTileSize& value);
    };

    /**
     * @brief The parsed content of a Tilemap3DEXT JSON file: dimensions, tile size, and a
     * flat row-major tile-ID array -- the same shape Tilemap3DFactoryEXT::BuildFromArrayEXT
     * already takes as in-memory data.
     */
    class Tilemap3DFileContent
    {
    public:
        Tilemap3DFileContent() = default;

        [[nodiscard]] int getWidthProperty() const { return width_; }
        [[nodiscard]] int getHeightProperty() const { return height_; }
        [[nodiscard]] int getDepthProperty() const { return depth_; }
        [[nodiscard]] const Tilemap3DFileTileSize& getTileSizeProperty() const { return tileSize_; }
        [[nodiscard]] const std::vector<int>& getTileIdsProperty() const { return tileIds_; }

    private:
        int width_ = 0;
        int height_ = 0;
        int depth_ = 0;
        Tilemap3DFileTileSize tileSize_;
        std::vector<int> tileIds_;

        friend void from_json(const nlohmann::ordered_json& j, Tilemap3DFileContent& value);
    };
}
