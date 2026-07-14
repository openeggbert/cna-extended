// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/PointProfile3DEXT.hpp"

#include "CNA/Extended/World3DEXT/RandomVector3EXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void PointProfile3DEXT::GetOffsetAndHeading(Vector3* offset, Vector3* heading, System::Random& random)
    {
        offset->X = offset->Y = offset->Z = 0.0f;
        NextUnitVector3EXT(random, *heading);
    }
}
