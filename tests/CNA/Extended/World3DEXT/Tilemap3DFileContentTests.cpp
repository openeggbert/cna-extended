// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for Tilemap3DFileContent (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/Tilemap3DFileContent.hpp"

#include "System/Text/Json/JsonSerializer.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    TEST(Tilemap3DFileContentTests, ParsesAllFields)
    {
        const char* const document = R"({
            "width": 4, "height": 2, "depth": 3,
            "tileSize": {"x": 1.5, "y": 2.5, "z": 3.5},
            "tileIds": [1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0]
        })";

        const Tilemap3DFileContent content = System::Text::Json::JsonSerializer::Deserialize<Tilemap3DFileContent>(document);

        EXPECT_EQ(content.getWidthProperty(), 4);
        EXPECT_EQ(content.getHeightProperty(), 2);
        EXPECT_EQ(content.getDepthProperty(), 3);
        EXPECT_FLOAT_EQ(content.getTileSizeProperty().getXProperty(), 1.5f);
        EXPECT_FLOAT_EQ(content.getTileSizeProperty().getYProperty(), 2.5f);
        EXPECT_FLOAT_EQ(content.getTileSizeProperty().getZProperty(), 3.5f);
        EXPECT_EQ(content.getTileIdsProperty().size(), 24u);
        EXPECT_EQ(content.getTileIdsProperty()[0], 1);
    }
}
