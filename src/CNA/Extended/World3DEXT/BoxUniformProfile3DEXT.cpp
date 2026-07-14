// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/BoxUniformProfile3DEXT.hpp"

#include "CNA/Extended/World3DEXT/RandomVector3EXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void BoxUniformProfile3DEXT::GetOffsetAndHeading(Vector3* offset, Vector3* heading, System::Random& random)
    {
        const float areaX = HeightEXT * DepthEXT; // area of each of the two X faces
        const float areaY = WidthEXT * DepthEXT; // area of each of the two Y faces
        const float areaZ = WidthEXT * HeightEXT; // area of each of the two Z faces
        const float totalArea = 2.0f * (areaX + areaY + areaZ);
        const float value = random.NextSingle() * totalArea;

        if (value < areaX) // -X face
        {
            offset->X = WidthEXT * -0.5f;
            offset->Y = HeightEXT * -0.5f + random.NextSingle() * HeightEXT;
            offset->Z = DepthEXT * -0.5f + random.NextSingle() * DepthEXT;
        }
        else if (value < 2.0f * areaX) // +X face
        {
            offset->X = WidthEXT * 0.5f;
            offset->Y = HeightEXT * -0.5f + random.NextSingle() * HeightEXT;
            offset->Z = DepthEXT * -0.5f + random.NextSingle() * DepthEXT;
        }
        else if (value < 2.0f * areaX + areaY) // -Y face
        {
            offset->X = WidthEXT * -0.5f + random.NextSingle() * WidthEXT;
            offset->Y = HeightEXT * -0.5f;
            offset->Z = DepthEXT * -0.5f + random.NextSingle() * DepthEXT;
        }
        else if (value < 2.0f * areaX + 2.0f * areaY) // +Y face
        {
            offset->X = WidthEXT * -0.5f + random.NextSingle() * WidthEXT;
            offset->Y = HeightEXT * 0.5f;
            offset->Z = DepthEXT * -0.5f + random.NextSingle() * DepthEXT;
        }
        else if (value < 2.0f * areaX + 2.0f * areaY + areaZ) // -Z face
        {
            offset->X = WidthEXT * -0.5f + random.NextSingle() * WidthEXT;
            offset->Y = HeightEXT * -0.5f + random.NextSingle() * HeightEXT;
            offset->Z = DepthEXT * -0.5f;
        }
        else // +Z face
        {
            offset->X = WidthEXT * -0.5f + random.NextSingle() * WidthEXT;
            offset->Y = HeightEXT * -0.5f + random.NextSingle() * HeightEXT;
            offset->Z = DepthEXT * 0.5f;
        }

        NextUnitVector3EXT(random, *heading);
    }
}
