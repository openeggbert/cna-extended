// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/SpatialHash.cs: a fixed-cell-size spatial hash
// implementing ICollisionBroadphase2D. Actors are inserted into every integer cell their
// BoundingBox2D overlaps; queries collect the union of actors from all cells overlapping the
// query bounds (de-duplicated, since an actor spanning multiple cells must only be reported once),
// then re-checked against the exact query bounds since cell membership is an over-approximation.
//
// `Dictionary<CellKey, List<ICollisionActor>>` -> `std::unordered_map<SpatialHashCellKey,
// std::vector<ICollisionActor*>>`. `Dictionary<ICollisionActor, List<CellKey>>` ->
// `std::unordered_map<ICollisionActor*, std::vector<SpatialHashCellKey>>` (raw-pointer key:
// std::unordered_map<T*, ...> hashes by address out of the box, no custom specialization needed,
// unlike SpatialHashCellKey below). `List<ICollisionActor> _actors` ->
// `std::vector<ICollisionActor*>`, also what GetActors() returns a reference to directly. None of
// these maps/vectors own the actors (non-owning ICollisionActor* throughout), matching
// ICollisionBroadphase2D.hpp's established convention.
//
// Upstream's `private readonly struct CellKey : IEquatable<CellKey>` is nested inside SpatialHash.
// Here it is hoisted to a free-standing `SpatialHashCellKey` type at namespace scope instead of
// staying nested, for a structural reason, not a stylistic one: it is used as an
// `unordered_map` key INSIDE SpatialHash's own member declarations, so its `std::hash`
// specialization must be fully visible before those members are declared -- impossible for a type
// nested in the very class whose body needs that specialization already complete (C++ class bodies
// cannot be split by an intervening `namespace std { ... }` block and then resumed). Kept public at
// namespace scope but documented as an implementation detail, matching this project's established
// `internal`-has-no-C++-equivalent precedent (see CollisionShapeKind2D.hpp/ActorPairKey.hpp) --
// upstream code outside SpatialHash never references CellKey either.
//
// Insert/Remove take ICollisionActor* per the ICollisionBroadphase2D interface signature (not
// changed here); no null guard is added, matching ICollisionBroadphase2D.hpp's own documented
// precedent that implementers trust the caller (CollisionWorld2D always passes an already
// null-checked actor) rather than re-deriving upstream's ArgumentNullException.ThrowIfNull calls.
// The constructor's ArgumentOutOfRangeException.ThrowIfNegativeOrZero(size.Width)/(size.Height) ->
// sharp-runtime's System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(value, paramName),
// the same static guard-helper API, not re-implemented from scratch.
#pragma once

#include "CNA/Extended/Collisions/ICollisionBroadphase2D.hpp"
#include "CNA/Extended/SizeF.hpp"

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

namespace CNA::Extended::Collisions
{
    /** @brief Implementation detail of SpatialHash: an integer cell coordinate used as a hash-map key. Not part of the public API. */
    struct SpatialHashCellKey
    {
        int X = 0;
        int Y = 0;

        SpatialHashCellKey() = default;
        SpatialHashCellKey(int x, int y) : X(x), Y(y) {}

        [[nodiscard]] bool Equals(const SpatialHashCellKey& other) const { return X == other.X && Y == other.Y; }
        [[nodiscard]] int GetHashCode() const;

        friend bool operator==(const SpatialHashCellKey& left, const SpatialHashCellKey& right) { return left.Equals(right); }
        friend bool operator!=(const SpatialHashCellKey& left, const SpatialHashCellKey& right) { return !left.Equals(right); }
    };
}

namespace std
{
    template <>
    struct hash<CNA::Extended::Collisions::SpatialHashCellKey>
    {
        std::size_t operator()(const CNA::Extended::Collisions::SpatialHashCellKey& value) const noexcept
        {
            return static_cast<std::size_t>(value.GetHashCode());
        }
    };
}

namespace CNA::Extended::Collisions
{
    /** @brief Stores collision actors in a fixed-size spatial hash for broadphase overlap queries. */
    class SpatialHash : public ICollisionBroadphase2D
    {
    public:
        /**
         * @brief Initializes a new spatial hash with the specified cell size.
         * @param size The width and height of each hash cell in world units.
         * @throws System::ArgumentOutOfRangeException size has a width or height <= 0.
         */
        explicit SpatialHash(const SizeF& size);

        void Insert(ICollisionActor* actor) override;
        bool Remove(ICollisionActor* actor) override;
        [[nodiscard]] std::vector<ICollisionActor*> Query(const BoundingBox2D& bounds) const override;
        [[nodiscard]] const std::vector<ICollisionActor*>& GetActors() const override { return actors_; }
        void Reset() override;

    private:
        void InsertIntoCells(ICollisionActor* actor);
        void AddToCell(int x, int y, ICollisionActor* actor);
        void GetCellRange(const BoundingBox2D& bounds, int& minX, int& minY, int& maxX, int& maxY) const;
        [[nodiscard]] static int GetCellIndex(float value, float cellSize);

        std::unordered_map<SpatialHashCellKey, std::vector<ICollisionActor*>> cells_;
        std::unordered_map<ICollisionActor*, std::vector<SpatialHashCellKey>> actorCells_;
        std::vector<ICollisionActor*> actors_;
        SizeF cellSize_;
    };
}
