// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::AnimationSystem3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Builds on: CNA::Extended::ECS::Systems::EntityUpdateSystem (this project's already-ported
// ECS update-system base) and SkinnedModelComponentEXT/SkinnedModelEXT::ComputeBoneTransformsEXT
// (real bone-hierarchy + keyframe-interpolation sampling, already implemented in cna -- see
// SkinnedModelComponentEXT.hpp's header comment for why this is reused rather than
// re-implemented). Each frame, advances every animated entity's PositionEXT by the frame's
// elapsed time and recomputes BoneTransformsEXT; does not touch any Effect/GPU state itself
// -- that happens in RenderSystem3DEXT.Draw(), see that header's own comment for why.
//
// A dedicated system rather than reusing Animations::AnimationController (the existing 2D
// sprite-sheet frame-timing system, see 3d.md section 9.3): AnimationController's timing
// model advances a discrete frame index through a fixed-duration-per-frame sprite sheet,
// which doesn't fit continuous keyframe interpolation across arbitrarily-spaced
// TimeSpan-keyed keyframes -- SkinnedModelEXT::ComputeBoneTransformsEXT already implements
// the correct sampling for that, so this system is a thin per-entity driver over it, not a
// new timing engine.
#pragma once

#include "CNA/Extended/ECS/Systems/EntityUpdateSystem.hpp"

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Advances every entity's SkinnedModelComponentEXT playback position and
     * recomputes its BoneTransformsEXT each Update().
     * @see SkinnedModelComponentEXT, the component this system animates.
     * @see RenderSystem3DEXT, which draws using the BoneTransformsEXT this system computes.
     */
    class AnimationSystem3DEXT final : public ECS::Systems::EntityUpdateSystem
    {
    public:
        AnimationSystem3DEXT();

        using ECS::Systems::EntityUpdateSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Update(const Microsoft::Xna::Framework::GameTime& gameTime) override;
    };
}
