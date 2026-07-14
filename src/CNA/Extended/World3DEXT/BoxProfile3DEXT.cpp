// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/BoxProfile3DEXT.hpp"

#include "CNA/Extended/World3DEXT/RandomVector3EXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void BoxProfile3DEXT::GetOffsetAndHeading(Vector3* offset, Vector3* heading, System::Random& random)
    {
        switch (random.Next(6))
        {
            case 0: // -X face
                offset->X = WidthEXT * -0.5f;
                offset->Y = HeightEXT * -0.5f + random.NextSingle() * HeightEXT;
                offset->Z = DepthEXT * -0.5f + random.NextSingle() * DepthEXT;
                break;

            case 1: // +X face
                offset->X = WidthEXT * 0.5f;
                offset->Y = HeightEXT * -0.5f + random.NextSingle() * HeightEXT;
                offset->Z = DepthEXT * -0.5f + random.NextSingle() * DepthEXT;
                break;

            case 2: // -Y face
                offset->X = WidthEXT * -0.5f + random.NextSingle() * WidthEXT;
                offset->Y = HeightEXT * -0.5f;
                offset->Z = DepthEXT * -0.5f + random.NextSingle() * DepthEXT;
                break;

            case 3: // +Y face
                offset->X = WidthEXT * -0.5f + random.NextSingle() * WidthEXT;
                offset->Y = HeightEXT * 0.5f;
                offset->Z = DepthEXT * -0.5f + random.NextSingle() * DepthEXT;
                break;

            case 4: // -Z face
                offset->X = WidthEXT * -0.5f + random.NextSingle() * WidthEXT;
                offset->Y = HeightEXT * -0.5f + random.NextSingle() * HeightEXT;
                offset->Z = DepthEXT * -0.5f;
                break;

            default: // +Z face (case 5)
                offset->X = WidthEXT * -0.5f + random.NextSingle() * WidthEXT;
                offset->Y = HeightEXT * -0.5f + random.NextSingle() * HeightEXT;
                offset->Z = DepthEXT * 0.5f;
                break;
        }

        NextUnitVector3EXT(random, *heading);
    }
}
