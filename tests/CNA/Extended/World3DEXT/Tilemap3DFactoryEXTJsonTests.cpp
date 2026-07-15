// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for Tilemap3DFactoryEXT::BuildFromJsonFileEXT/
// BuildFromJsonStreamEXT and Tilemap3DFileContent (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/Tilemap3DFactoryEXT.hpp"

#include "CNA/Extended/World3DEXT/Tilemap3DEXT.hpp"
#include "System/ArgumentException.hpp"
#include "System/IO/FileStream.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    namespace
    {
        const char* const kDocument = R"({
            "width": 2, "height": 1, "depth": 2,
            "tileSize": {"x": 1.0, "y": 1.0, "z": 1.0},
            "tileIds": [1, 0, 0, 2]
        })";

        std::filesystem::path MakeTempFile(const std::string& contents)
        {
            const std::filesystem::path path = std::filesystem::temp_directory_path() / "Tilemap3DFactoryEXTJsonTests_scratch.json";
            std::ofstream out(path, std::ios::binary);
            out << contents;
            out.close();
            return path;
        }
    }

    TEST(Tilemap3DFactoryEXTJsonTests, BuildFromJsonFileEXT_ParsesDocument)
    {
        const std::filesystem::path path = MakeTempFile(kDocument);

        const Tilemap3DEXT tilemap = Tilemap3DFactoryEXT::BuildFromJsonFileEXT(path.string());

        EXPECT_EQ(tilemap.GetTileCountEXT(), 2u);
        EXPECT_EQ(tilemap.GetTileEXT(0, 0, 0), 1);
        EXPECT_EQ(tilemap.GetTileEXT(1, 0, 1), 2);
        EXPECT_FALSE(tilemap.HasTileEXT(1, 0, 0));

        std::filesystem::remove(path);
    }

    TEST(Tilemap3DFactoryEXTJsonTests, BuildFromJsonStreamEXT_ParsesDocument)
    {
        const std::filesystem::path path = MakeTempFile(kDocument);

        System::IO::FileStream stream(path.string());
        const Tilemap3DEXT tilemap = Tilemap3DFactoryEXT::BuildFromJsonStreamEXT(stream);

        EXPECT_EQ(tilemap.GetTileCountEXT(), 2u);
        EXPECT_EQ(tilemap.GetTileEXT(0, 0, 0), 1);
        EXPECT_EQ(tilemap.GetTileEXT(1, 0, 1), 2);

        std::filesystem::remove(path);
    }

    TEST(Tilemap3DFactoryEXTJsonTests, BuildFromJsonFileEXT_UsesTileSizeFromDocument)
    {
        const char* const document = R"({
            "width": 1, "height": 1, "depth": 1,
            "tileSize": {"x": 2.0, "y": 3.0, "z": 4.0},
            "tileIds": [1]
        })";
        const std::filesystem::path path = MakeTempFile(document);

        const Tilemap3DEXT tilemap = Tilemap3DFactoryEXT::BuildFromJsonFileEXT(path.string());

        EXPECT_FLOAT_EQ(tilemap.getTileSizeProperty().X, 2.0f);
        EXPECT_FLOAT_EQ(tilemap.getTileSizeProperty().Y, 3.0f);
        EXPECT_FLOAT_EQ(tilemap.getTileSizeProperty().Z, 4.0f);

        std::filesystem::remove(path);
    }

    TEST(Tilemap3DFactoryEXTJsonTests, BuildFromJsonFileEXT_MismatchedTileCount_Throws)
    {
        const char* const document = R"({
            "width": 2, "height": 2, "depth": 2,
            "tileSize": {"x": 1.0, "y": 1.0, "z": 1.0},
            "tileIds": [1, 0]
        })";
        const std::filesystem::path path = MakeTempFile(document);

        EXPECT_THROW((void)Tilemap3DFactoryEXT::BuildFromJsonFileEXT(path.string()), System::ArgumentException);

        std::filesystem::remove(path);
    }
}
