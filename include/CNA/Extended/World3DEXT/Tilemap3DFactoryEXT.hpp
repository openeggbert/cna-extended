// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Tilemap3DFactoryEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Tilemaps::TilemapFactory, originally deliberately scoped
// down per this task's own wording ("whether it reads a real format... or only supports
// hand-built data is a decision for this phase, not assumed here" -- plan3d.md's Phase 8
// entry): built a Tilemap3DEXT from an in-memory array only. No 3D-extended Tiled/Ogmo JSON
// format exists to read (Tiled/Ogmo/LDtk are all 2D-only formats with no voxel/3D grid
// concept), and inventing a new file format for this project to also parse was judged out
// of scope for that phase.
//
// BuildFromJsonFileEXT/BuildFromJsonStreamEXT (Phase 12 C, 2026-07-15, user-requested
// "extend later" item) add that file-format reader: a project-defined JSON schema (see
// Tilemap3DFileContent.hpp), not an existing MonoGame.Extended format (none apply, per the
// reasoning above) -- both delegate straight to BuildFromArrayEXT after parsing, so the
// validation/grid-construction logic isn't duplicated.
#pragma once

#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <string>
#include <vector>

namespace System::IO
{
    class Stream;
}

namespace CNA::Extended::World3DEXT
{
    class Tilemap3DEXT;

    /**
     * @brief Builds a Tilemap3DEXT from an in-memory, hand-built tile ID array, or by
     * reading and parsing a Tilemap3DEXT JSON file.
     * @see Tilemap3DEXT, the type this factory constructs.
     * @see Tilemap3DFileContent, the JSON schema BuildFromJsonFileEXT/BuildFromJsonStreamEXT read.
     */
    class Tilemap3DFactoryEXT
    {
    public:
        /**
         * @brief Builds a Tilemap3DEXT from a flat, row-major (X fastest, then Y, then Z)
         * array of tile IDs (0 == empty).
         * @param tileIds Flat array of width * height * depth tile IDs.
         * @param width Grid extent along X.
         * @param height Grid extent along Y.
         * @param depth Grid extent along Z.
         * @param tileSize The world-space size of one tile cell along each axis.
         * @throws System::ArgumentException tileIds.size() != width * height * depth.
         */
        [[nodiscard]] static Tilemap3DEXT BuildFromArrayEXT(const std::vector<int>& tileIds, int width, int height, int depth,
                                                              const Microsoft::Xna::Framework::Vector3& tileSize);

        /**
         * @brief Builds a Tilemap3DEXT by reading and parsing a Tilemap3DEXT JSON file from
         * the local file system.
         * @param path The path to the JSON file (see Tilemap3DFileContent.hpp for the schema).
         * @throws System::ArgumentException tileIds.size() != width * height * depth.
         */
        [[nodiscard]] static Tilemap3DEXT BuildFromJsonFileEXT(const std::string& path);

        /**
         * @brief Builds a Tilemap3DEXT by reading and parsing Tilemap3DEXT JSON content from
         * an open stream.
         * @param stream The stream to read the JSON content from.
         * @throws System::ArgumentException tileIds.size() != width * height * depth.
         */
        [[nodiscard]] static Tilemap3DEXT BuildFromJsonStreamEXT(System::IO::Stream& stream);
    };
}
