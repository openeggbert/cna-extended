// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/RandomVector3EXT.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended::World3DEXT
{
    void NextUnitVector3EXT(System::Random& random, Microsoft::Xna::Framework::Vector3& vector)
    {
        const float z = -1.0f + random.NextSingle() * 2.0f; // uniform in [-1, 1]
        const float phi = random.NextSingle() * 6.28318530717958647692f; // 2*pi
        const float sinTheta = std::sqrt(std::max(0.0f, 1.0f - z * z));

        vector.X = sinTheta * std::cos(phi);
        vector.Y = sinTheta * std::sin(phi);
        vector.Z = z;
    }
}
