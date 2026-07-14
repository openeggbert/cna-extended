// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::AnimatedBillboardComponentEXT -- new, non-upstream addition.
// See 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Graphics::AnimatedSprite. Builds on this project's
// already-ported Graphics::SpriteSheet/Texture2DAtlas (named animation frames as atlas
// regions) and Animations::AnimationController (frame-timing/looping) exactly as
// AnimatedSprite itself does (see AnimatedSprite.cpp) -- reused, not reinvented: this
// component only pairs an AnimationController with the sibling BillboardComponentEXT
// AnimatedBillboardSystemEXT keeps in sync (see that system's header comment).
//
// Must be attached alongside a BillboardComponentEXT on the same entity:
// AnimatedBillboardSystemEXT reads/writes that sibling component's TextureEXT/
// VertexBufferEXT each time the current frame changes; BillboardRenderSystemEXT does the
// actual drawing, unaware this component even exists.
#pragma once

namespace CNA::Extended::Graphics
{
    class SpriteSheet;
}

namespace CNA::Extended::Animations
{
    class AnimationController;
}

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief ECS component pairing a SpriteSheet (named animations as texture atlas
     * regions) with a per-entity AnimationController, advanced by AnimatedBillboardSystemEXT.
     * @see AnimatedBillboardSystemEXT, which advances ControllerEXT and updates the sibling
     * BillboardComponentEXT's texture/UV on each frame change.
     */
    struct AnimatedBillboardComponentEXT
    {
        /** @brief The sprite sheet supplying animation frames as texture atlas regions. Not owned by this component. */
        Graphics::SpriteSheet* SpriteSheetEXT = nullptr;

        /** @brief Frame-timing/looping state for the currently playing animation. Not owned by this component; typically one instance per entity. */
        Animations::AnimationController* ControllerEXT = nullptr;
    };
}
