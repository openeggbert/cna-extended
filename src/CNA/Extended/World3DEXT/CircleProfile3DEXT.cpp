// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/CircleProfile3DEXT.hpp"

#include "CNA/Extended/World3DEXT/RandomVector3EXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Random.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void CircleProfile3DEXT::GetOffsetAndHeading(Vector3* offset, Vector3* heading, System::Random& random)
    {
        const float distance = random.NextSingle() * RadiusEXT;

        NextUnitVector3EXT(random, *heading);

        switch (RadiateEXT)
        {
            case CircleRadiation3DEXT::In:
                *offset = *heading * -distance;
                break;

            case CircleRadiation3DEXT::Out:
                *offset = *heading * distance;
                break;

            case CircleRadiation3DEXT::None:
                *offset = *heading * distance;
                NextUnitVector3EXT(random, *heading);
                break;

            default:
                throw System::ArgumentOutOfRangeException("RadiateEXT", "Unsupported radiation mode");
        }
    }
}
