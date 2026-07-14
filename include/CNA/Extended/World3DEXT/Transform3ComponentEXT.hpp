// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Transform3ComponentEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Bridges this project's existing CNA::Extended::Transform3 (already ported from
// MonoGame.Extended's Transform.cs -- a dirty-flag-optimized parent/child transform
// hierarchy, see Transform.hpp) into the ECS: Transform3 itself only knows about a parent
// *pointer* (another Transform3*), not a parent *entity*, so it has no way on its own to
// express "my parent is entity 7" the way an ECS scene graph needs. This component adds
// exactly that -- a ParentEntityIdEXT the TransformHierarchySystemEXT (see that file)
// resolves into a real Transform3::setParentProperty() call each frame. Transform3's own
// dirty-flag/world-matrix-recompute machinery is reused entirely unchanged; this component
// adds no new hierarchy logic of its own.
#pragma once

#include "CNA/Extended/Transform.hpp"

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief ECS component pairing a Transform3 with a parent *entity* ID, so
     * TransformHierarchySystemEXT can wire real Transform3 parent/child relationships from
     * ECS entity relationships.
     * @see TransformHierarchySystemEXT, which resolves ParentEntityIdEXT into a real
     * Transform3::setParentProperty() call.
     */
    struct Transform3ComponentEXT
    {
        /** @brief The wrapped transform. Reuses Transform3's existing dirty-flag-optimized world-matrix recompute as-is. */
        Transform3 TransformEXT;

        /** @brief The parent entity's ID, or -1 if this entity has no parent. */
        int ParentEntityIdEXT = -1;
    };
}
