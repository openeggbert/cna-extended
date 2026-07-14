// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/PointProfile3DEXT.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

#include <cmath>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void PointProfile3DEXT::GetOffsetAndHeading(Vector3* offset, Vector3* heading, System::Random& random)
    {
        offset->X = offset->Y = offset->Z = 0.0f;

        const float z = -1.0f + random.NextSingle() * 2.0f; // uniform in [-1, 1]
        const float phi = random.NextSingle() * 6.28318530717958647692f; // 2*pi
        const float sinTheta = std::sqrt(std::max(0.0f, 1.0f - z * z));

        heading->X = sinTheta * std::cos(phi);
        heading->Y = sinTheta * std::sin(phi);
        heading->Z = z;
    }
}
