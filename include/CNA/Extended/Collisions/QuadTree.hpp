// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/QuadTree/QuadTree.cs: a hierarchical quadtree for
// broadphase overlap queries.
//
// Ownership model: upstream's `List<QuadTree> Children` are GC-owned by the parent node; ported as
// `std::vector<std::unique_ptr<QuadTree>>` (Children genuinely owns its child nodes -- nothing else
// references them). `HashSet<QuadtreeData> Contents` is ported as
// `System::Collections::Generic::HashSet<QuadtreeData*>` -- non-owning, since a QuadtreeData is
// owned by the QuadTreeSpace that created it (see QuadtreeData.hpp/QuadTreeSpace.hpp), not by the
// tree nodes that merely reference it. As with QuadtreeData's own `_parents` set, upstream never
// overrides Equals/GetHashCode on QuadtreeData, so C#'s default reference-equality HashSet<T>
// semantics are exactly what a HashSet of raw pointers gives for free in C++.
//
// `protected` visibility on Children/Contents/CurrentDepth/MaxDepth/MaxObjectsPerNode is preserved
// exactly, even though no subclass exists in this codebase yet, matching this project's "port 1:1"
// mandate.
//
// `Queue<QuadTree>` (BFS traversal in NumTargets/Shake) -> sharp-runtime's
// System::Collections::Generic::Queue<T>, matching this project's "reuse sharp-runtime types" rule.
//
// `ArgumentNullException.ThrowIfNull` on Insert(QuadtreeData)/Remove(QuadtreeData) has no C++
// equivalent to preserve -- both take QuadtreeData& (a reference, not representable as null),
// matching this project's established convention (see ActorPairKey.hpp's header comment) of
// dropping such checks where the parameter type itself rules out null.
//
// Remove(QuadtreeData)'s `throw new InvalidOperationException(...)` on a non-leaf node ->
// std::logic_error, matching ObjectPool<T>::New's precedent for InvalidOperationException-style
// C# exceptions with no closer std:: equivalent.
#pragma once

#include "CNA/Extended/BoundingBox2D.hpp"
#include "System/Collections/Generic/HashSet.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::Collisions
{
    class QuadtreeData;

    /** @brief Stores collision actors in a hierarchical quadtree for broadphase overlap queries. */
    class QuadTree
    {
    public:
        /** @brief The default maximum depth. */
        static constexpr int DefaultMaxDepth = 7;

        /** @brief The default maximum objects per node. */
        static constexpr int DefaultMaxObjectsPerNode = 25;

        /** @brief Initializes a new instance covering the specified bounds. */
        explicit QuadTree(const BoundingBox2D& bounds);

        /** @brief Gets the bounds of the area covered by this quadtree node. */
        [[nodiscard]] BoundingBox2D getNodeBoundsProperty() const { return NodeBounds; }

        /** @brief Gets a value indicating whether the current node has no child nodes. */
        [[nodiscard]] bool getIsLeafProperty() const { return Children.empty(); }

        /** @brief Counts the number of distinct actors stored beneath this node. */
        [[nodiscard]] int NumTargets();

        /** @brief Inserts the specified quadtree data into this node or one of its descendants. */
        void Insert(QuadtreeData& data);

        /** @brief Removes the specified quadtree data from this (leaf) node. */
        void Remove(QuadtreeData& data);

        /** @brief Removes unnecessary leaf nodes and simplifies the quadtree. */
        void Shake();

        /** @brief Splits the current leaf node into four child quadrants when additional depth is available. */
        void Split();

        /** @brief Removes all contents from this node and every descendant node. */
        void ClearAll();

        /**
         * @brief Queries the quadtree for targets that intersect with the given area.
         * @remark const: does not mutate this node's own Children/Contents structure, only calls
         * MarkDirty()/MarkClean() on the externally-owned QuadtreeData instances it references
         * (transient traversal bookkeeping on other objects, not part of this node's own state).
         */
        [[nodiscard]] std::vector<QuadtreeData*> Query(const BoundingBox2D& area) const;

    protected:
        /** @brief Contains the child nodes of this node. */
        std::vector<std::unique_ptr<QuadTree>> Children;

        /** @brief Contains the quadtree data stored directly in this node. */
        System::Collections::Generic::HashSet<QuadtreeData*> Contents;

        /** @brief Gets or sets the current depth for this node in the quadtree. */
        int CurrentDepth = 0;

        /** @brief Gets or sets the maximum depth of the quadtree. */
        int MaxDepth = DefaultMaxDepth;

        /** @brief Gets or sets the maximum objects per node in this quadtree. */
        int MaxObjectsPerNode = DefaultMaxObjectsPerNode;

        /** @brief Gets the bounds of the area covered by this quadtree node. */
        BoundingBox2D NodeBounds;

    private:
        void AddToLeaf(QuadtreeData& data);
        void Clear();
        void QueryWithoutReset(const BoundingBox2D& area, std::vector<QuadtreeData*>& recursiveResult) const;
    };
}
