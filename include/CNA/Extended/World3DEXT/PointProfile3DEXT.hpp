// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::PointProfile3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Profiles::PointProfile: emits all particles
// from a single point (zero offset) with a uniformly random heading -- extended from a
// random point on the unit *circle* (2D) to a random point on the unit *sphere* (3D), via
// the standard z-uniform-in-[-1,1]/phi-uniform-in-[0,2*pi) construction (equivalent to
// ConeProfile3DEXT's own sampling with HalfAngleEXT = pi, i.e. no directional restriction at
// all -- see ConeProfile3DEXT.hpp).
#pragma once

#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Emits all particles from a single point (no offset) with a uniformly random heading over the full sphere. */
    class PointProfile3DEXT final : public Profile3DEXT
    {
    public:
        void GetOffsetAndHeading(Microsoft::Xna::Framework::Vector3* offset, Microsoft::Xna::Framework::Vector3* heading,
                                  System::Random& random) override;
    };
}
