// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/QuadTree/QuadTreeData.cs (upstream file name has a
// capital "T" in "QuadTreeData.cs"; the C# *class* itself is `QuadtreeData` with a lowercase "t" --
// preserved exactly as-is, not "fixed" to match the file name).
//
// Ownership model (a real design decision upstream's GC made invisible, decided explicitly here):
// a QuadtreeData is created and owned by QuadTreeSpace (see QuadTreeSpace.hpp) for the lifetime of
// an inserted actor. QuadTree nodes that store a QuadtreeData in their Contents set, and the
// QuadtreeData's own record of which QuadTree nodes currently reference it (`parents_`), both hold
// only non-owning raw pointers into that externally-owned instance -- mirroring upstream, where a
// QuadtreeData is a single GC-tracked object referenced from multiple places, never copied.
// HashSet<QuadTree> `_parents` -> sharp-runtime's System::Collections::Generic::HashSet<QuadTree*>:
// upstream never overrides Equals/GetHashCode on QuadTree, so C#'s default reference-equality
// HashSet<T> semantics are exactly what a HashSet of raw pointers gives for free in C++ (no custom
// std::hash specialization needed, unlike ActorPairKey in an earlier task, which genuinely needed
// value equality).
#pragma once

#include "CNA/Extended/BoundingBox2D.hpp"
#include "System/Collections/Generic/HashSet.hpp"

namespace CNA::Extended::Collisions
{
    class ICollisionActor;
    class QuadTree;

    /** @brief Stores an actor and the quadtree nodes that currently reference it. */
    class QuadtreeData
    {
    public:
        /** @brief Initializes a new entry wrapping the specified actor, capturing its current broadphase bounds. */
        explicit QuadtreeData(ICollisionActor& target);

        /** @brief Gets the bounding box used to place this actor in the quadtree. */
        [[nodiscard]] BoundingBox2D getBoundsProperty() const { return bounds_; }

        void setBoundsProperty(const BoundingBox2D& value) { bounds_ = value; }

        /** @brief Gets a value indicating whether this entry has already been visited during the current traversal. */
        [[nodiscard]] bool getDirtyProperty() const { return dirty_; }

        /** @brief Gets the collision actor stored in this entry. */
        [[nodiscard]] ICollisionActor& getTargetProperty() const { return *target_; }

        /** @brief Removes a parent node reference from this entry. */
        void RemoveParent(QuadTree& parent);

        /** @brief Adds a parent node reference to this entry and refreshes its cached bounds. */
        void AddParent(QuadTree& parent);

        /** @brief Removes this entry from every parent node that currently contains it. */
        void RemoveFromAllParents();

        /** @brief Marks this entry as visited for the current traversal. */
        void MarkDirty() { dirty_ = true; }

        /** @brief Marks this entry as unvisited for the next traversal. */
        void MarkClean() { dirty_ = false; }

    private:
        System::Collections::Generic::HashSet<QuadTree*> parents_;
        BoundingBox2D bounds_;
        bool dirty_ = false;
        ICollisionActor* target_;
    };
}
