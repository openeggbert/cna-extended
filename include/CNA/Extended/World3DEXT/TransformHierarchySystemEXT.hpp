// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::TransformHierarchySystemEXT -- new, non-upstream addition.
// See 3d.md/plan3d.md at the repository root for the design.
//
// A thin adapter, not a new hierarchy implementation: every frame, for each entity with a
// Transform3ComponentEXT, resolves ParentEntityIdEXT to the parent entity's own
// Transform3ComponentEXT (or nullptr if it has none/isn't found) and calls the real
// Transform3::setParentProperty() -- after that single call, Transform3's own existing
// dirty-flag machinery (see Transform.hpp) handles all local/world matrix propagation with
// no further logic needed here. setParentProperty() already no-ops when the parent hasn't
// actually changed, so calling it unconditionally every frame is cheap.
#pragma once

#include "CNA/Extended/ECS/Systems/EntityUpdateSystem.hpp"

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Resolves each Transform3ComponentEXT's ParentEntityIdEXT into a real
     * Transform3 parent/child relationship every Update().
     * @see Transform3ComponentEXT, the component this system operates on.
     */
    class TransformHierarchySystemEXT final : public ECS::Systems::EntityUpdateSystem
    {
    public:
        TransformHierarchySystemEXT();

        // Bring EntityUpdateSystem::Initialize(World&) back into scope: overriding just the
        // ComponentManager& overload below would otherwise hide it (-Werror=overloaded-virtual).
        using ECS::Systems::EntityUpdateSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Update(const Microsoft::Xna::Framework::GameTime& gameTime) override;
    };
}
