// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::SpatialHash3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::QuadTree, per plan3d.md Phase 5's "start
// with the simplest correct version" scope decision. Design correction made during
// implementation, documented transparently (matching this project's established practice
// for such corrections -- see Camera3DEXT.hpp/ModelComponentEXT.hpp's own header
// comments): rather than a true recursive octree with node splitting/merging (mirroring
// QuadTree.cpp's own ~250-line recursive-subdivision implementation), this class is
// implemented as a fixed-cell-size 3D spatial hash -- CNA::Extended::Collisions::
// SpatialHash's own real, already-tested algorithm (a legitimate first-class 2D
// broadphase in its own right, not a fallback) extended with a third (Z) axis. A uniform
// grid hash generalizes far more directly from 2D to 3D than QuadTree's recursive
// splitting logic does, while still being a real spatially-accelerated broadphase (not a
// linear scan). True recursive octree subdivision can be added later as a separate
// broadphase implementation if profiling ever shows a real need -- nothing in
// ICollisionBroadphase3DEXT's interface or CollisionWorld3DEXT precludes swapping it in.
//
// Renamed from OctreeEXT (Phase 10, 2026-07-14, audit.md finding A-05): the class was
// always honestly documented above as a spatial hash, not a hierarchical octree, but the
// *name* itself still invited the opposite assumption for anyone who hadn't read this
// comment. Renaming to match its actual algorithm (and CNA::Extended::Collisions::
// SpatialHash's own naming) removes that ambiguity outright rather than relying on
// documentation alone. This was a pure rename -- no behavior change; see plan3d.md's
// Phase 10 entry for A-05's still-open performance concern (Query()'s O(K) std::find
// duplicate check), which this rename does not address.
#pragma once

#include "CNA/Extended/World3DEXT/ICollisionBroadphase3DEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    /** @brief Implementation detail of SpatialHash3DEXT: an integer cell coordinate used as a hash-map key. Not part of the public API. */
    struct SpatialHash3DCellKeyEXT
    {
        int X = 0;
        int Y = 0;
        int Z = 0;

        SpatialHash3DCellKeyEXT() = default;
        SpatialHash3DCellKeyEXT(int x, int y, int z) : X(x), Y(y), Z(z) {}

        [[nodiscard]] bool Equals(const SpatialHash3DCellKeyEXT& other) const { return X == other.X && Y == other.Y && Z == other.Z; }
        [[nodiscard]] int GetHashCode() const;

        friend bool operator==(const SpatialHash3DCellKeyEXT& left, const SpatialHash3DCellKeyEXT& right) { return left.Equals(right); }
        friend bool operator!=(const SpatialHash3DCellKeyEXT& left, const SpatialHash3DCellKeyEXT& right) { return !left.Equals(right); }
    };
}

namespace std
{
    template <>
    struct hash<CNA::Extended::World3DEXT::SpatialHash3DCellKeyEXT>
    {
        std::size_t operator()(const CNA::Extended::World3DEXT::SpatialHash3DCellKeyEXT& value) const noexcept
        {
            return static_cast<std::size_t>(value.GetHashCode());
        }
    };
}

namespace CNA::Extended::World3DEXT
{
    /** @brief Stores 3D collision actors in a fixed-cell-size spatial hash for broadphase overlap queries. */
    class SpatialHash3DEXT final : public ICollisionBroadphase3DEXT
    {
    public:
        /**
         * @brief Initializes a new spatial hash with the specified uniform cell size.
         * @param cellSize The width/height/depth of each cubic hash cell, in world units. Must be > 0.
         * @throws System::ArgumentOutOfRangeException cellSize <= 0.
         */
        explicit SpatialHash3DEXT(float cellSize);

        void Insert(ICollisionActor3DEXT* actor) override;
        bool Remove(ICollisionActor3DEXT* actor) override;
        [[nodiscard]] std::vector<ICollisionActor3DEXT*> Query(const Microsoft::Xna::Framework::BoundingBox& bounds) const override;
        [[nodiscard]] const std::vector<ICollisionActor3DEXT*>& GetActors() const override { return actors_; }
        void Reset() override;

    private:
        void InsertIntoCells(ICollisionActor3DEXT* actor);
        void AddToCell(int x, int y, int z, ICollisionActor3DEXT* actor);
        void GetCellRange(const Microsoft::Xna::Framework::BoundingBox& bounds, int& minX, int& minY, int& minZ, int& maxX, int& maxY, int& maxZ) const;
        [[nodiscard]] int GetCellIndex(float value) const;

        std::unordered_map<SpatialHash3DCellKeyEXT, std::vector<ICollisionActor3DEXT*>> cells_;
        std::unordered_map<ICollisionActor3DEXT*, std::vector<SpatialHash3DCellKeyEXT>> actorCells_;
        std::vector<ICollisionActor3DEXT*> actors_;
        float cellSize_;
    };
}
