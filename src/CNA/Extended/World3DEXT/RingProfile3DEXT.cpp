// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/RingProfile3DEXT.hpp"

#include "CNA/Extended/World3DEXT/RandomVector3EXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Random.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void RingProfile3DEXT::GetOffsetAndHeading(Vector3* offset, Vector3* heading, System::Random& random)
    {
        NextUnitVector3EXT(random, *heading);

        switch (RadiateEXT)
        {
            case CircleRadiation3DEXT::In:
                *offset = *heading * -RadiusEXT;
                break;

            case CircleRadiation3DEXT::Out:
                *offset = *heading * RadiusEXT;
                break;

            case CircleRadiation3DEXT::None:
                *offset = *heading * RadiusEXT;
                NextUnitVector3EXT(random, *heading);
                break;

            default:
                throw System::ArgumentOutOfRangeException("RadiateEXT", "Unsupported radiation mode");
        }
    }
}
