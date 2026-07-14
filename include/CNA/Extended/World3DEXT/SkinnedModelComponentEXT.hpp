// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::SkinnedModelComponentEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Builds on Microsoft::Xna::Framework::Graphics::SkinnedModelEXT -- a real, already-
// implemented CNA type (GPU-skinnable mesh + skeleton + animation clips, with a working
// ComputeBoneTransformsEXT bone sampler), not built here from scratch. SkinnedModelEXT was
// originally added for cna's Avatar system (AvatarRenderer::EnableRealRenderingEXT), but
// its bone-hierarchy/keyframe-sampling logic and GPU-backed Parts are entirely avatar-
// agnostic -- see SkinnedModelEXT.hpp's own header comment ("no attempt is made to align
// indices, counts, or semantics" with the Xbox Avatar's 71-bone arrays). Reused directly
// here rather than re-implemented, per plan3d.md's "reuse sharp-runtime/cna types, don't
// re-roll them" convention (root CLAUDE.md's working rules).
//
// Ownership (resolves the question 3d.md section 9.1 deferred to this phase): EffectEXT is
// a non-owning SkinnedEffect* -- the caller constructs and owns one SkinnedEffect per
// animated entity (bone transforms differ per entity, so the effect cannot be shared
// across entities the way a stateless BasicEffect sometimes could be), matching every
// other component in this ECS being purely non-owning (see ModelComponentEXT.hpp).
// Modeled directly on AvatarRenderer::DrawRealEXT's real, already-working usage
// (AvatarRenderer.cpp): a *single* SkinnedEffect is shared across every Part of one
// SkinnedModelEXT (set once per draw: World/View/Projection/BoneTransforms, then per part:
// Texture + Apply() + draw) -- unlike ModelComponentEXT, individual ModelMeshPart::Effect*
// pointers on SkinnedModelEXT::Parts are not used for real rendering at all.
//
// BoneTransformsEXT is populated by AnimationSystem3DEXT.Update() each frame (an
// EntityUpdateSystem) and consumed by RenderSystem3DEXT.Draw() (an EntityDrawSystem,
// running afterward in the same frame): the transforms are pushed onto EffectEXT
// immediately before that entity's own draw calls, not eagerly in AnimationSystem3DEXT --
// see RenderSystem3DEXT.cpp's header comment for why that ordering matters.
#pragma once

#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "System/TimeSpan.hpp"

#include <string>
#include <vector>

namespace Microsoft::Xna::Framework::Graphics
{
    class SkinnedModelEXT;
    class SkinnedEffect;
}

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief ECS component pairing a non-owning SkinnedModelEXT* and SkinnedEffect* with
     * the current animation clip/playback position, so AnimationSystem3DEXT can advance
     * its pose and RenderSystem3DEXT can draw it each frame.
     * @see AnimationSystem3DEXT, which advances PositionEXT and computes BoneTransformsEXT.
     * @see RenderSystem3DEXT, which draws this component using BoneTransformsEXT.
     */
    struct SkinnedModelComponentEXT
    {
        /** @brief The skinned model to animate/draw. Not owned by this component. */
        Microsoft::Xna::Framework::Graphics::SkinnedModelEXT* ModelEXT = nullptr;

        /** @brief The effect used to draw ModelEXT's parts. Not owned by this component; typically one instance per entity. */
        Microsoft::Xna::Framework::Graphics::SkinnedEffect* EffectEXT = nullptr;

        /** @brief Name of the clip in ModelEXT->Clips currently playing. Empty means "no animation". */
        std::string ClipNameEXT;

        /** @brief Current playback position within the clip. Advanced each frame by AnimationSystem3DEXT. */
        System::TimeSpan PositionEXT;

        /** @brief Whether playback wraps around the clip's Duration instead of clamping to it. */
        bool LoopEXT = true;

        /**
         * @brief The model's bounds in its own local (untransformed, bind-pose) space, used
         * for frustum culling. Defaults to a zero-radius sphere at the origin, which the
         * caller MUST replace with real bounds -- a zero-radius sphere almost never
         * intersects the view frustum once the entity has moved, silently culling a real,
         * visible model with no warning. Unlike ModelComponentEXT (see ComputeModelBoundsEXT,
         * ModelBoundsEXT.hpp), there is no equivalent helper here yet: SkinnedModelEXT's
         * PartEXT::Part is a plain ModelMeshPart with no per-part BoundingSphere of its own
         * (unlike ModelMesh), so deriving real bounds would mean reading back each part's
         * VertexBuffer and computing BoundingSphere::CreateFromPoints -- left for a future
         * session (audit.md finding A-08's documented fallback) rather than guessed at here.
         */
        Microsoft::Xna::Framework::BoundingSphere BoundsEXT;

        /** @brief Skinning-ready world bone matrices, recomputed each frame by AnimationSystem3DEXT. */
        std::vector<Microsoft::Xna::Framework::Matrix> BoneTransformsEXT;
    };
}
