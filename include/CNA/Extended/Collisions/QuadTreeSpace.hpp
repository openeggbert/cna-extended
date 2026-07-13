// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/QuadTree/QuadTreeSpace.cs: a thin ICollisionBroadphase2D
// adapter wrapping a QuadTree root node. Owns every QuadtreeData it creates in Insert (see
// QuadtreeData.hpp's header comment for the full ownership rationale) via
// std::unordered_map<ICollisionActor*, std::unique_ptr<QuadtreeData>> -- the QuadTree/its nodes
// only ever hold non-owning QuadtreeData* references into instances owned here, mirroring
// upstream's `Dictionary<ICollisionActor, QuadtreeData> _actorData` (GC-owned there).
#pragma once

#include "CNA/Extended/Collisions/ICollisionBroadphase2D.hpp"
#include "CNA/Extended/Collisions/QuadTree.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace CNA::Extended::Collisions
{
    class QuadtreeData;

    class QuadTreeSpace : public ICollisionBroadphase2D
    {
    public:
        /** @brief Initializes a new quadtree-backed collision space for the specified axis-aligned boundary. */
        explicit QuadTreeSpace(const BoundingBox2D& boundary);

        // Declared (not defaulted inline) and defined in QuadTreeSpace.cpp, where QuadtreeData is a
        // complete type -- actorData_'s std::unique_ptr<QuadtreeData> destructor requires that at
        // the point it's instantiated, which would otherwise happen wherever QuadTreeSpace is first
        // destroyed (e.g. a test file that never includes QuadtreeData.hpp), failing to compile.
        ~QuadTreeSpace() override;
        QuadTreeSpace(QuadTreeSpace&&) noexcept;
        QuadTreeSpace& operator=(QuadTreeSpace&&) noexcept;
        QuadTreeSpace(const QuadTreeSpace&) = delete;
        QuadTreeSpace& operator=(const QuadTreeSpace&) = delete;

        void Insert(ICollisionActor* actor) override;
        bool Remove(ICollisionActor* actor) override;
        [[nodiscard]] std::vector<ICollisionActor*> Query(const BoundingBox2D& bounds) const override;
        [[nodiscard]] const std::vector<ICollisionActor*>& GetActors() const override { return actors_; }
        void Reset() override;

    private:
        QuadTree collisionTree_;
        std::vector<ICollisionActor*> actors_;
        std::unordered_map<ICollisionActor*, std::unique_ptr<QuadtreeData>> actorData_;
    };
}
