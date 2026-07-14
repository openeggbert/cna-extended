// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::AnimatedBillboardSystemEXT -- new, non-upstream addition.
// See 3d.md/plan3d.md and AnimatedBillboardComponentEXT.hpp's own header comment for the
// design.
//
// Each frame, advances every entity's AnimatedBillboardComponentEXT::ControllerEXT
// (Animations::AnimationController::Update, exactly as AnimatedSprite::Update() does --
// see AnimatedSprite.cpp) and, if the current frame index changed, re-derives the sibling
// BillboardComponentEXT's UV rect from the new frame's Texture2DRegion and re-uploads its
// VertexBufferEXT via SetData -- no new frame-timing engine, this system only keeps a
// BillboardComponentEXT's texture/UV in sync with an already-advancing AnimationController.
//
// Also syncs immediately in OnEntityAdded, not just on a later frame change: without this,
// a newly-attached entity would show whatever (or no) texture/UV the caller happened to
// leave on its BillboardComponentEXT until the animation's first frame boundary elapsed --
// AnimatedSprite::SetAnimation has the same requirement and solves it the same way
// (assigning the initial TextureRegion immediately, not waiting for the first Update()).
#pragma once

#include "CNA/Extended/ECS/Systems/EntityUpdateSystem.hpp"

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Advances every entity's AnimatedBillboardComponentEXT and updates its sibling
     * BillboardComponentEXT's texture/UV whenever the current animation frame changes (or
     * immediately, when the entity is first added).
     * @see AnimatedBillboardComponentEXT, the component this system animates.
     * @see BillboardComponentEXT, the sibling component this system updates in place.
     */
    class AnimatedBillboardSystemEXT final : public ECS::Systems::EntityUpdateSystem
    {
    public:
        AnimatedBillboardSystemEXT();

        using ECS::Systems::EntityUpdateSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Update(const Microsoft::Xna::Framework::GameTime& gameTime) override;

    protected:
        void OnEntityAdded(int entityId) override;

    private:
        void SyncCurrentFrameEXT(int entityId);
    };
}
