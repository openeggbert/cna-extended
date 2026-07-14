// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::VortexModifier3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Not a literal 1:1 port of CNA::Extended::Particles::Modifiers::VortexModifier: 2D's vortex
// rotates a 2D gravitational force around a single point (the whole 2D plane has one implicit
// rotation axis, perpendicular to the screen). Per the approved plan, the 3D generalization
// rotates around an explicit AxisEXT line (through TriggeredPositionEXT + PositionEXT), not a
// single point -- particles are pulled toward/around that line, not a sphere center. The
// radial distance used for InnerRadiusEXT/OuterRadiusEXT and the 1/distance force falloff is
// measured perpendicular to AxisEXT (the displacement vector's component orthogonal to the
// axis), and "rotating the gravity vector by RotationAngleEXT" becomes a proper rotation
// around AxisEXT via Rodrigues' rotation formula restricted to a vector already perpendicular
// to the axis (v*cos(theta) + (axis x v)*sin(theta)), which is exactly the identity a 2D
// rotation matrix computes when AxisEXT is the implicit 2D screen-normal -- so this is a true
// generalization, not an unrelated new algorithm.
#pragma once

#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Applies a rotated gravitational force around an axis line, creating spiral/orbital/vortex particle motion in 3D. */
    class VortexModifier3DEXT final : public Modifier3DEXT
    {
    public:
        /** @brief Offset (from each particle's TriggeredPositionEXT) of a point the vortex axis line passes through. */
        Microsoft::Xna::Framework::Vector3 PositionEXT = Microsoft::Xna::Framework::Vector3::Zero;

        /** @brief Direction of the vortex's rotation axis, in world space. Normalized internally. */
        Microsoft::Xna::Framework::Vector3 AxisEXT = Microsoft::Xna::Framework::Vector3::Up;

        float StrengthEXT = 0.0f;
        float OuterRadiusEXT = 0.0f;
        float InnerRadiusEXT = 0.0f;
        float MaxVelocityEXT = 0.0f;

        VortexModifier3DEXT();

        /** @brief Gets the rotation (radians) applied to the gravitational force vector around AxisEXT: 0 is pure attraction, larger values spiral/orbit. */
        [[nodiscard]] float getRotationAngleEXTProperty() const { return rotationAngleEXT_; }
        /** @brief Sets RotationAngleEXT, recomputing the cached cos/sin used each Update() so it isn't recomputed per particle. */
        void setRotationAngleEXTProperty(float value);

    protected:
        void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override;

    private:
        float rotationAngleEXT_ = 0.0f;
        float cosAngleEXT_ = 1.0f;
        float sinAngleEXT_ = 0.0f;
    };
}
