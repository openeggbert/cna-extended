// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for ExternalResourceResolvers. Fresh tests below. OpenTitleContainerStream
// is not covered here -- it needs a real title-content directory this test binary doesn't set up
// (matching the "no GPU/no live-resource-context" testing gaps already documented for
// ViewportAdapters/VectorDraw/Graphics::Effects elsewhere in this project).
#include "CNA/Extended/Content/ExternalResourceResolvers.hpp"

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/IO/Stream.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

namespace CNA::Extended::Content
{
    namespace
    {
        std::filesystem::path MakeTempFile(const std::string& contents)
        {
            const std::filesystem::path path = std::filesystem::temp_directory_path() / "ExternalResourceResolversTests_scratch.txt";
            std::ofstream out(path, std::ios::binary);
            out << contents;
            out.close();
            return path;
        }
    }

    TEST(ExternalResourceResolversTests, OpenFileReadsFileContents)
    {
        const std::filesystem::path path = MakeTempFile("hello world");

        const std::unique_ptr<System::IO::Stream> stream = OpenFile(path.string());
        ASSERT_NE(stream, nullptr);

        std::vector<SharpRuntime::bytecs> buffer(11);
        stream->Read(buffer.data(), 0, static_cast<int>(buffer.size()));
        const std::string readBack(buffer.begin(), buffer.end());

        EXPECT_EQ(readBack, "hello world");

        std::filesystem::remove(path);
    }

    TEST(ExternalResourceResolversTests, OpenFileThrowsOnEmptyPath)
    {
        EXPECT_THROW((void)OpenFile(""), std::invalid_argument);
    }

    TEST(ExternalResourceResolversTests, OpenTitleContainerStreamThrowsOnEmptyPath)
    {
        EXPECT_THROW((void)OpenTitleContainerStream(""), std::invalid_argument);
    }
}
