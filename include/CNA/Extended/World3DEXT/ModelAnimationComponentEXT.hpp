// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ModelAnimationComponentEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// SkinnedModelComponentEXT (see that file) wraps cna's Avatar-specific SkinnedModelEXT data
// model. This component wraps the OTHER, separate skinned-playback path cna provides: a real
// Microsoft::Xna::Framework::Graphics::Model whose Tag holds a SkinningData (mirroring the
// well-known XNA "Skinned Model Sample"), animated with AnimationPlayer -- see
// AnimationPlayer.hpp's own header comment. This is the path cna's glTF/CNJ import tools
// (cna_tool_gltf_to_cnj, GltfModelTypeReader) actually populate for a skinned asset, so any
// game whose skinned-character content comes through that pipeline (as opposed to the
// bespoke .skinnedmodel.json format the Avatar system uses) needs this component instead of
// SkinnedModelComponentEXT -- there was previously no ECS component/system pair for it at all.
//
// Ownership: ModelEXT is a non-owning Model*, matching ModelComponentEXT's convention. PlayerEXT
// is owned by value -- unlike SkinnedModelComponentEXT's BoneTransformsEXT (a bare vector the
// system fills in), AnimationPlayer already IS the per-entity playback state (current clip,
// position, computed transforms), so wrapping it here instead of re-deriving its fields avoids
// duplicating logic AnimationPlayer already provides. AnimationPlayer has no default
// constructor (it must be bound to a SkinningData reference at construction, see
// AnimationPlayer.hpp), so this component has no default constructor either -- construct it
// with the Model's own SkinningData, retrieved via `static_cast<SkinningData*>(model.
// getTagProperty())` (see ModelAnimationSystem3DEXT's own usage for the exact pattern).
//
// No separate EffectEXT field is needed (unlike SkinnedModelComponentEXT): cna's glTF/CNJ
// importer already assigns a real SkinnedEffect (or SkinnedPbrEffect) to every skinned
// ModelMeshPart at load time, the same way ModelComponentEXT's unskinned path reuses each
// part's own Effect* instead of a separate field -- RenderSystem3DEXT only needs to push this
// component's computed bone transforms onto each of ModelEXT's existing per-mesh Effects
// before calling Model::Draw(), not construct or own a new Effect of its own.
#pragma once

#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"

#include <string>

namespace Microsoft::Xna::Framework::Graphics
{
    class Model;
}

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief ECS component pairing a non-owning Model* (whose Tag holds a SkinningData) with
     * an owned AnimationPlayer, so ModelAnimationSystem3DEXT can advance playback and
     * RenderSystem3DEXT can draw it each frame.
     * @see ModelAnimationSystem3DEXT, which advances PlayerEXT by switching/ticking ClipNameEXT.
     * @see RenderSystem3DEXT, which draws this component using PlayerEXT's computed skin transforms.
     */
    struct ModelAnimationComponentEXT
    {
        /**
         * @param skinningData The model's skeleton/clip data (from `model.getTagProperty()`).
         * Must outlive this component, matching AnimationPlayer's own lifetime requirement.
         */
        explicit ModelAnimationComponentEXT(const Microsoft::Xna::Framework::Graphics::SkinningData& skinningData)
            : SkinningDataEXT(&skinningData), PlayerEXT(skinningData)
        {
        }

        /** @brief The model to animate/draw. Not owned by this component -- caller/asset system manages lifetime. */
        Microsoft::Xna::Framework::Graphics::Model* ModelEXT = nullptr;

        /** @brief The skeleton/clip data PlayerEXT was constructed from; used to resolve ClipNameEXT by name. */
        const Microsoft::Xna::Framework::Graphics::SkinningData* SkinningDataEXT;

        /** @brief Advances/holds the current bone pose. See AnimationPlayer.hpp. */
        Microsoft::Xna::Framework::Graphics::AnimationPlayer PlayerEXT;

        /**
         * @brief Name of the clip in SkinningDataEXT->AnimationClips that should be playing.
         * Empty means "hold bind pose" (StartClip is never called). Changing this to a
         * different known clip name makes ModelAnimationSystem3DEXT call PlayerEXT.StartClip()
         * next Update() -- a hard cut, no blending between the old and new clip (see
         * ModelAnimationSystem3DEXT's own header comment for why blending is out of scope here).
         */
        std::string ClipNameEXT;

        /** @brief Whether playback wraps around the clip's Duration instead of clamping to it. */
        bool LoopEXT = true;

        /**
         * @brief The model's bounds in its own local (untransformed, bind-pose) space, used for
         * frustum culling. Defaults to a zero-radius sphere at the origin, which the caller MUST
         * replace with real bounds -- see ModelComponentEXT::BoundsEXT's identical warning, and
         * ComputeModelBoundsEXT (ModelBoundsEXT.hpp) for a ready-made helper.
         */
        Microsoft::Xna::Framework::BoundingSphere BoundsEXT;
    };
}
