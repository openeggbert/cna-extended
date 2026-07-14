// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for BoxUniformProfile3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/BoxUniformProfile3DEXT.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(BoxUniformProfile3DEXTTests, OffsetLiesOnOneFace)
    {
        BoxUniformProfile3DEXT subject;
        subject.WidthEXT = 20.0f;
        subject.HeightEXT = 10.0f;
        subject.DepthEXT = 6.0f;
        System::Random random;

        for (int i = 0; i < 100; ++i)
        {
            Vector3 offset;
            Vector3 heading;
            subject.GetOffsetAndHeading(&offset, &heading, random);

            const bool onXFace = std::abs(std::abs(offset.X) - 10.0f) < 1e-4f;
            const bool onYFace = std::abs(std::abs(offset.Y) - 5.0f) < 1e-4f;
            const bool onZFace = std::abs(std::abs(offset.Z) - 3.0f) < 1e-4f;
            EXPECT_TRUE(onXFace || onYFace || onZFace);
        }
    }

    // A larger face (bigger area) should be sampled proportionally more often than a smaller
    // one -- the whole point of area-weighting, distinguishing this from BoxProfile3DEXT's
    // equal-probability-per-face selection.
    TEST(BoxUniformProfile3DEXTTests, LargerFaceIsSampledMoreOftenThanSmallerFace)
    {
        // A long, thin, flat box: X faces (H*D = 1*1 = 1) are tiny; Z faces (W*H = 100*1 = 100)
        // are huge -- Z faces should dominate samples heavily.
        BoxUniformProfile3DEXT subject;
        subject.WidthEXT = 100.0f;
        subject.HeightEXT = 1.0f;
        subject.DepthEXT = 1.0f;
        System::Random random;

        int zFaceCount = 0;
        int xFaceCount = 0;
        constexpr int kSamples = 500;
        for (int i = 0; i < kSamples; ++i)
        {
            Vector3 offset;
            Vector3 heading;
            subject.GetOffsetAndHeading(&offset, &heading, random);

            if (std::abs(std::abs(offset.Z) - 0.5f) < 1e-4f) ++zFaceCount;
            if (std::abs(std::abs(offset.X) - 50.0f) < 1e-4f) ++xFaceCount;
        }

        EXPECT_GT(zFaceCount, xFaceCount);
    }

    TEST(BoxUniformProfile3DEXTTests, HeadingIsUnitVector)
    {
        BoxUniformProfile3DEXT subject;
        subject.WidthEXT = 20.0f;
        subject.HeightEXT = 10.0f;
        subject.DepthEXT = 6.0f;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_NEAR(heading.Length(), 1.0f, 1e-5f);
    }
}
