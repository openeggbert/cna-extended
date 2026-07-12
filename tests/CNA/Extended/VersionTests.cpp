// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/Version.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace CNA::Extended
{
    TEST(Version, VersionNumberMatchesComponents)
    {
        EXPECT_EQ(VersionNumber(), VersionMajor * 10000 + VersionMinor * 100 + VersionPatch);
    }

    TEST(Version, VersionStringIsNotEmpty)
    {
        EXPECT_FALSE(std::string_view(VersionString()).empty());
    }
}
