// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Octree3DEXT -- new, non-upstream addition. See 3d.md/plan3d.md
// at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::QuadTreeSpace: a thin ICollisionBroadphase3DEXT
// adapter wrapping an OctreeNode3DEXT root node. Owns every OctreeNodeData3DEXT it creates
// in Insert (see OctreeNodeData3DEXT.hpp's header comment for the full ownership rationale)
// via std::unordered_map<ICollisionActor3DEXT*, std::unique_ptr<OctreeNodeData3DEXT>> -- the
// octree/its nodes only ever hold non-owning OctreeNodeData3DEXT* references into instances
// owned here, mirroring QuadTreeSpace.hpp's own rationale.
//
// This is a real recursive-subdivision octree, brought to parity with
// CNA::Extended::Collisions::QuadTree (2026-07-14, user-requested), unlike SpatialHash3DEXT
// (a uniform fixed-cell grid hash). It is NOT CollisionWorld3DEXT's default broadphase --
// SpatialHash3DEXT still is -- usable via the existing
// CollisionWorld3DEXT(std::unique_ptr<ICollisionBroadphase3DEXT>) constructor, exactly as
// SpatialHash3DEXT.hpp's own header comment already promised ("nothing... precludes swapping
// it in").
#pragma once

#include "CNA/Extended/World3DEXT/ICollisionBroadphase3DEXT.hpp"
#include "CNA/Extended/World3DEXT/OctreeNode3DEXT.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    class OctreeNodeData3DEXT;

    class Octree3DEXT final : public ICollisionBroadphase3DEXT
    {
    public:
        /** @brief Initializes a new octree-backed collision space for the specified axis-aligned boundary. */
        explicit Octree3DEXT(const Microsoft::Xna::Framework::BoundingBox& boundary);

        // Declared (not defaulted inline) and defined in Octree3DEXT.cpp, where
        // OctreeNodeData3DEXT is a complete type -- actorData_'s
        // std::unique_ptr<OctreeNodeData3DEXT> destructor requires that at the point it's
        // instantiated, which would otherwise happen wherever Octree3DEXT is first
        // destroyed (e.g. a test file that never includes OctreeNodeData3DEXT.hpp), failing
        // to compile. Matches QuadTreeSpace.hpp's own precedent exactly.
        ~Octree3DEXT() override;
        Octree3DEXT(Octree3DEXT&&) noexcept;
        Octree3DEXT& operator=(Octree3DEXT&&) noexcept;
        Octree3DEXT(const Octree3DEXT&) = delete;
        Octree3DEXT& operator=(const Octree3DEXT&) = delete;

        void Insert(ICollisionActor3DEXT* actor) override;
        bool Remove(ICollisionActor3DEXT* actor) override;
        [[nodiscard]] std::vector<ICollisionActor3DEXT*> Query(const Microsoft::Xna::Framework::BoundingBox& bounds) const override;
        [[nodiscard]] const std::vector<ICollisionActor3DEXT*>& GetActors() const override { return actors_; }
        void Reset() override;

    private:
        OctreeNode3DEXT collisionTree_;
        std::vector<ICollisionActor3DEXT*> actors_;
        std::unordered_map<ICollisionActor3DEXT*, std::unique_ptr<OctreeNodeData3DEXT>> actorData_;
    };
}
