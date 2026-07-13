// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for Content/TexturePacker/*. Fresh tests below.
#include "CNA/Extended/Content/TexturePacker/TexturePackerFileReader.hpp"

#include "System/IO/FileStream.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace CNA::Extended::Content::TexturePacker
{
    namespace
    {
        const char* const kDocument = R"({
            "frames": [
                {
                    "filename": "a.png",
                    "frame": {"x": 0, "y": 0, "w": 4, "h": 4},
                    "rotated": false,
                    "trimmed": false,
                    "spriteSourceSize": {"x": 0, "y": 0, "w": 4, "h": 4},
                    "sourceSize": {"w": 4, "h": 4},
                    "pivot": {"x": 0.5, "y": 0.5}
                }
            ],
            "textures": [],
            "meta": {"app": "app", "version": "1", "image": "a.png", "dataformat": "json", "smartupdate": ""}
        })";

        std::filesystem::path MakeTempFile(const std::string& contents)
        {
            const std::filesystem::path path = std::filesystem::temp_directory_path() / "TexturePackerFileReaderTests_scratch.json";
            std::ofstream out(path, std::ios::binary);
            out << contents;
            out.close();
            return path;
        }
    }

    TEST(TexturePackerFileReaderTests, ReadFromPathParsesDocument)
    {
        const std::filesystem::path path = MakeTempFile(kDocument);

        const TexturePackerFileContent content = Read(path.string());

        ASSERT_EQ(content.getRegionsProperty().size(), 1u);
        EXPECT_EQ(content.getRegionsProperty()[0].getFileNameProperty(), "a.png");
        EXPECT_EQ(content.getMetaProperty().getDataFormatProperty(), "json");

        std::filesystem::remove(path);
    }

    TEST(TexturePackerFileReaderTests, ReadFromStreamParsesDocument)
    {
        const std::filesystem::path path = MakeTempFile(kDocument);

        System::IO::FileStream stream(path.string());
        const TexturePackerFileContent content = Read(stream);

        ASSERT_EQ(content.getRegionsProperty().size(), 1u);
        EXPECT_EQ(content.getRegionsProperty()[0].getFileNameProperty(), "a.png");

        std::filesystem::remove(path);
    }
}
