// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ModelAnimationSystem3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// The AnimationPlayer-driven counterpart to AnimationSystem3DEXT (which drives the separate,
// Avatar-specific SkinnedModelComponentEXT/SkinnedModelEXT path instead -- see that system's own
// header comment). Each frame: if ModelAnimationComponentEXT::ClipNameEXT names a clip other
// than the one PlayerEXT is currently playing, snapshots the outgoing pose and starts the new
// clip from the beginning (StartClip), then crossfades from that snapshot toward the new clip's
// live pose over ClipNameEXT's own ModelAnimationComponentEXT::BlendDurationEXT seconds (a
// per-bone Matrix::Lerp -- see ModelAnimationComponentEXT's own header comment for why that
// simplification is adequate here); otherwise just advances PlayerEXT and keeps the crossfade (if
// any) progressing. Writes the frame's actual output to
// ModelAnimationComponentEXT::BlendedSkinTransformsEXT -- callers (RenderSystem3DEXT) must draw
// from that, not PlayerEXT.GetSkinTransforms() directly. Does not touch any Effect/GPU state
// itself -- that happens in RenderSystem3DEXT.Draw(), matching AnimationSystem3DEXT's own
// division of responsibility.
#pragma once

#include "CNA/Extended/ECS/Systems/EntityUpdateSystem.hpp"

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Advances every entity's ModelAnimationComponentEXT playback each Update(),
     * crossfading (see this file's own header comment) when ClipNameEXT names a different clip
     * than the one currently playing.
     * @see ModelAnimationComponentEXT, the component this system animates.
     * @see RenderSystem3DEXT, which draws using BlendedSkinTransformsEXT, the bone transforms
     * this system computes.
     */
    class ModelAnimationSystem3DEXT final : public ECS::Systems::EntityUpdateSystem
    {
    public:
        ModelAnimationSystem3DEXT();

        using ECS::Systems::EntityUpdateSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Update(const Microsoft::Xna::Framework::GameTime& gameTime) override;
    };
}
