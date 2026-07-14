// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::OctreeNodeData3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::QuadtreeData, same ownership model: owned by
// Octree3DEXT (see Octree3DEXT.hpp) for the lifetime of an inserted actor; OctreeNode3DEXT
// nodes that store this in their Contents set, and this entry's own record of which
// OctreeNode3DEXT nodes currently reference it (parents_), both hold only non-owning raw
// pointers into the externally-owned instance -- mirroring QuadtreeData.hpp's own rationale.
#pragma once

#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "System/Collections/Generic/HashSet.hpp"

namespace CNA::Extended::World3DEXT
{
    class ICollisionActor3DEXT;
    class OctreeNode3DEXT;

    /** @brief Stores an actor and the octree nodes that currently reference it. */
    class OctreeNodeData3DEXT
    {
    public:
        /** @brief Initializes a new entry wrapping the specified actor, capturing its current broadphase bounds. */
        explicit OctreeNodeData3DEXT(ICollisionActor3DEXT& target);

        /** @brief Gets the bounding box used to place this actor in the octree. */
        [[nodiscard]] Microsoft::Xna::Framework::BoundingBox getBoundsProperty() const { return bounds_; }

        void setBoundsProperty(const Microsoft::Xna::Framework::BoundingBox& value) { bounds_ = value; }

        /** @brief Gets a value indicating whether this entry has already been visited during the current traversal. */
        [[nodiscard]] bool getDirtyProperty() const { return dirty_; }

        /** @brief Gets the collision actor stored in this entry. */
        [[nodiscard]] ICollisionActor3DEXT& getTargetProperty() const { return *target_; }

        /** @brief Removes a parent node reference from this entry. */
        void RemoveParent(OctreeNode3DEXT& parent);

        /** @brief Adds a parent node reference to this entry and refreshes its cached bounds. */
        void AddParent(OctreeNode3DEXT& parent);

        /** @brief Removes this entry from every parent node that currently contains it. */
        void RemoveFromAllParents();

        /** @brief Marks this entry as visited for the current traversal. */
        void MarkDirty() { dirty_ = true; }

        /** @brief Marks this entry as unvisited for the next traversal. */
        void MarkClean() { dirty_ = false; }

    private:
        System::Collections::Generic::HashSet<OctreeNode3DEXT*> parents_;
        Microsoft::Xna::Framework::BoundingBox bounds_;
        bool dirty_ = false;
        ICollisionActor3DEXT* target_;
    };
}
