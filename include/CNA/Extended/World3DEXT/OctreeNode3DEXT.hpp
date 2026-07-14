// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::OctreeNode3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::QuadTree, same recursive-subdivision
// algorithm extended from 4 quadrants to 8 octants (Split() divides NodeBounds around its
// volumetric center into 8 sub-boxes instead of 4 sub-rectangles; everything else --
// Insert's non-exclusive "insert into every overlapping child" recursion, Query's dirty-flag
// dedup, Shake's BFS merge-back-down, Remove's leaf-only-then-let-the-data's-own-parents-set-
// drive-real-removal pattern -- is identical in shape to QuadTree, just walking 8 children
// instead of 4). This is the piece that finishes bringing SpatialHash3DEXT's own header
// comment's "true recursive octree subdivision can be added later as a separate broadphase
// implementation" promise to reality (Octree3DEXT.hpp is the public
// ICollisionBroadphase3DEXT adapter built on top of this class, mirroring QuadTree/
// QuadTreeSpace's own split between "pure recursive tree" and "broadphase-adapter/ownership
// logic").
//
// Ownership model, mirroring QuadTree.hpp exactly: Children genuinely owns its child nodes
// (std::vector<std::unique_ptr<OctreeNode3DEXT>>, 0 or exactly 8, never partial). Contents is
// a non-owning System::Collections::Generic::HashSet<OctreeNodeData3DEXT*>: an
// OctreeNodeData3DEXT is owned by the Octree3DEXT that created it, not by the tree nodes
// that merely reference it, matching OctreeNodeData3DEXT.hpp's own rationale.
//
// protected visibility on Children/Contents/CurrentDepth/MaxDepth/MaxObjectsPerNode is kept
// (matching QuadTree.hpp's own precedent), even though no subclass exists in this codebase
// yet.
#pragma once

#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "System/Collections/Generic/HashSet.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    class OctreeNodeData3DEXT;

    /** @brief Stores collision actors in a hierarchical octree for broadphase overlap queries. */
    class OctreeNode3DEXT
    {
    public:
        /** @brief The default maximum depth. */
        static constexpr int DefaultMaxDepth = 7;

        /** @brief The default maximum objects per node. */
        static constexpr int DefaultMaxObjectsPerNode = 25;

        /** @brief Initializes a new instance covering the specified bounds. */
        explicit OctreeNode3DEXT(const Microsoft::Xna::Framework::BoundingBox& bounds);

        /** @brief Gets the bounds of the volume covered by this octree node. */
        [[nodiscard]] Microsoft::Xna::Framework::BoundingBox getNodeBoundsProperty() const { return NodeBounds; }

        /** @brief Gets a value indicating whether the current node has no child nodes. */
        [[nodiscard]] bool getIsLeafProperty() const { return Children.empty(); }

        /** @brief Counts the number of distinct actors stored beneath this node. */
        [[nodiscard]] int NumTargets();

        /** @brief Inserts the specified octree data into this node or one of its descendants. */
        void Insert(OctreeNodeData3DEXT& data);

        /** @brief Removes the specified octree data from this (leaf) node. */
        void Remove(OctreeNodeData3DEXT& data);

        /** @brief Removes unnecessary leaf nodes and simplifies the octree. */
        void Shake();

        /** @brief Splits the current leaf node into eight child octants when additional depth is available. */
        void Split();

        /** @brief Removes all contents from this node and every descendant node. */
        void ClearAll();

        /**
         * @brief Queries the octree for targets that intersect with the given volume.
         * @remark const: does not mutate this node's own Children/Contents structure, only calls
         * MarkDirty()/MarkClean() on the externally-owned OctreeNodeData3DEXT instances it
         * references (transient traversal bookkeeping on other objects, not part of this
         * node's own state).
         */
        [[nodiscard]] std::vector<OctreeNodeData3DEXT*> Query(const Microsoft::Xna::Framework::BoundingBox& area) const;

    protected:
        /** @brief Contains the child nodes of this node (0, or exactly 8). */
        std::vector<std::unique_ptr<OctreeNode3DEXT>> Children;

        /** @brief Contains the octree data stored directly in this node. */
        System::Collections::Generic::HashSet<OctreeNodeData3DEXT*> Contents;

        /** @brief Gets or sets the current depth for this node in the octree. */
        int CurrentDepth = 0;

        /** @brief Gets or sets the maximum depth of the octree. */
        int MaxDepth = DefaultMaxDepth;

        /** @brief Gets or sets the maximum objects per node in this octree. */
        int MaxObjectsPerNode = DefaultMaxObjectsPerNode;

        /** @brief Gets the bounds of the volume covered by this octree node. */
        Microsoft::Xna::Framework::BoundingBox NodeBounds;

    private:
        void AddToLeaf(OctreeNodeData3DEXT& data);
        void Clear();
        void QueryWithoutReset(const Microsoft::Xna::Framework::BoundingBox& area, std::vector<OctreeNodeData3DEXT*>& recursiveResult) const;
    };
}
