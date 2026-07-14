// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ParticleUpdateSystem3DEXT -- new, non-upstream addition.
// See 3d.md/plan3d.md at the repository root for the design.
//
// Each frame, calls ParticleEffectComponentEXT::EffectEXT->UpdateEXT(deltaSeconds, origin)
// for every active entity, where origin is the entity's Transform3ComponentEXT world
// position if it has one (Vector3::Zero otherwise) -- matching every other *3DEXT update
// system's identical Transform3ComponentEXT-is-optional convention (see
// TransformHierarchySystemEXT.hpp/RenderSystem3DEXT.hpp).
#pragma once

#include "CNA/Extended/ECS/Systems/EntityUpdateSystem.hpp"

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Advances every entity's ParticleEffectComponentEXT each frame.
     * @see ParticleEffectComponentEXT, the component this system advances.
     */
    class ParticleUpdateSystem3DEXT final : public ECS::Systems::EntityUpdateSystem
    {
    public:
        ParticleUpdateSystem3DEXT();

        using ECS::Systems::EntityUpdateSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Update(const Microsoft::Xna::Framework::GameTime& gameTime) override;
    };
}
