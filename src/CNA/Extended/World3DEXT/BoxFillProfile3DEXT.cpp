// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/BoxFillProfile3DEXT.hpp"

#include "CNA/Extended/World3DEXT/RandomVector3EXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void BoxFillProfile3DEXT::GetOffsetAndHeading(Vector3* offset, Vector3* heading, System::Random& random)
    {
        offset->X = WidthEXT * -0.5f + random.NextSingle() * WidthEXT;
        offset->Y = HeightEXT * -0.5f + random.NextSingle() * HeightEXT;
        offset->Z = DepthEXT * -0.5f + random.NextSingle() * DepthEXT;
        NextUnitVector3EXT(random, *heading);
    }
}
