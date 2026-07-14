// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::OctreeEXT -- new, non-upstream addition. See 3d.md/plan3d.md
// at the repository root for the design.
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
#pragma once

#include "CNA/Extended/World3DEXT/ICollisionBroadphase3DEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    /** @brief Implementation detail of OctreeEXT: an integer cell coordinate used as a hash-map key. Not part of the public API. */
    struct OctreeCellKeyEXT
    {
        int X = 0;
        int Y = 0;
        int Z = 0;

        OctreeCellKeyEXT() = default;
        OctreeCellKeyEXT(int x, int y, int z) : X(x), Y(y), Z(z) {}

        [[nodiscard]] bool Equals(const OctreeCellKeyEXT& other) const { return X == other.X && Y == other.Y && Z == other.Z; }
        [[nodiscard]] int GetHashCode() const;

        friend bool operator==(const OctreeCellKeyEXT& left, const OctreeCellKeyEXT& right) { return left.Equals(right); }
        friend bool operator!=(const OctreeCellKeyEXT& left, const OctreeCellKeyEXT& right) { return !left.Equals(right); }
    };
}

namespace std
{
    template <>
    struct hash<CNA::Extended::World3DEXT::OctreeCellKeyEXT>
    {
        std::size_t operator()(const CNA::Extended::World3DEXT::OctreeCellKeyEXT& value) const noexcept
        {
            return static_cast<std::size_t>(value.GetHashCode());
        }
    };
}

namespace CNA::Extended::World3DEXT
{
    /** @brief Stores 3D collision actors in a fixed-cell-size spatial hash for broadphase overlap queries. */
    class OctreeEXT final : public ICollisionBroadphase3DEXT
    {
    public:
        /**
         * @brief Initializes a new spatial hash with the specified uniform cell size.
         * @param cellSize The width/height/depth of each cubic hash cell, in world units. Must be > 0.
         * @throws System::ArgumentOutOfRangeException cellSize <= 0.
         */
        explicit OctreeEXT(float cellSize);

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

        std::unordered_map<OctreeCellKeyEXT, std::vector<ICollisionActor3DEXT*>> cells_;
        std::unordered_map<ICollisionActor3DEXT*, std::vector<OctreeCellKeyEXT>> actorCells_;
        std::vector<ICollisionActor3DEXT*> actors_;
        float cellSize_;
    };
}
