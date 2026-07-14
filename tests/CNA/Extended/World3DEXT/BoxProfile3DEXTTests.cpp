// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for BoxProfile3DEXT (see 3d.md/plan3d.md) -- there is no upstream
// MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/BoxProfile3DEXT.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(BoxProfile3DEXTTests, OffsetLiesOnOneFace)
    {
        BoxProfile3DEXT subject;
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
            EXPECT_LE(std::abs(offset.X), 10.0f + 1e-4f);
            EXPECT_LE(std::abs(offset.Y), 5.0f + 1e-4f);
            EXPECT_LE(std::abs(offset.Z), 3.0f + 1e-4f);
        }
    }

    TEST(BoxProfile3DEXTTests, HeadingIsUnitVector)
    {
        BoxProfile3DEXT subject;
        subject.WidthEXT = 20.0f;
        subject.HeightEXT = 10.0f;
        subject.DepthEXT = 6.0f;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_NEAR(heading.Length(), 1.0f, 1e-5f);
    }

    TEST(BoxProfile3DEXTTests, VisitsAllSixFacesOverManySamples)
    {
        BoxProfile3DEXT subject;
        subject.WidthEXT = 20.0f;
        subject.HeightEXT = 10.0f;
        subject.DepthEXT = 6.0f;
        System::Random random;

        bool sawNegX = false, sawPosX = false, sawNegY = false, sawPosY = false, sawNegZ = false, sawPosZ = false;
        for (int i = 0; i < 500; ++i)
        {
            Vector3 offset;
            Vector3 heading;
            subject.GetOffsetAndHeading(&offset, &heading, random);

            if (std::abs(offset.X - (-10.0f)) < 1e-4f) sawNegX = true;
            if (std::abs(offset.X - 10.0f) < 1e-4f) sawPosX = true;
            if (std::abs(offset.Y - (-5.0f)) < 1e-4f) sawNegY = true;
            if (std::abs(offset.Y - 5.0f) < 1e-4f) sawPosY = true;
            if (std::abs(offset.Z - (-3.0f)) < 1e-4f) sawNegZ = true;
            if (std::abs(offset.Z - 3.0f) < 1e-4f) sawPosZ = true;
        }

        EXPECT_TRUE(sawNegX && sawPosX && sawNegY && sawPosY && sawNegZ && sawPosZ);
    }
}
