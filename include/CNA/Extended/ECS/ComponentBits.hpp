// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/ComponentBits.cs: a fixed 256-bit field (4 segments of 64
// bits) for tracking which component types are present on an entity. Upstream implements its
// indexer via `Unsafe.Add(ref _bits0, segmentIndex)` -- raw pointer arithmetic treating four
// separate `ulong` fields as a contiguous 4-element array, done purely to avoid a real array's
// bounds-check overhead. Translated as a plain `std::array<std::uint64_t, 4>` indexed normally;
// this project has no equivalent need to dodge C#-specific struct-layout/field-address tricks,
// and a real array is both simpler and no less safe than the upstream reinterpretation.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace CNA::Extended::ECS
{
    /** @brief A fixed 256-bit field tracking which component types (by ID) are present on an entity. */
    class ComponentBits
    {
    public:
        static constexpr int BitsPerSegment = 64;
        static constexpr int SegmentCount = 4;
        static constexpr int MaxBits = BitsPerSegment * SegmentCount;

        ComponentBits() = default;

        /** @brief A proxy returned by non-const operator[] so `bits[i] = value` works like C#'s indexed property setter. */
        class BitReference
        {
        public:
            BitReference(ComponentBits& owner, int bitIndex) : owner_(&owner), bitIndex_(bitIndex) {}

            BitReference& operator=(bool value)
            {
                owner_->Set(bitIndex_, value);
                return *this;
            }

            [[nodiscard]] operator bool() const { return owner_->Get(bitIndex_); } // NOLINT(*-explicit-constructor)

        private:
            ComponentBits* owner_;
            int bitIndex_;
        };

        /** @brief Gets whether the bit at @p bitIndex is set. @throws std::out_of_range bitIndex is outside [0, 256). */
        [[nodiscard]] bool operator[](int bitIndex) const { return Get(bitIndex); }
        /** @brief Returns a proxy so `bits[bitIndex] = value` sets the bit at @p bitIndex. */
        BitReference operator[](int bitIndex) { return BitReference(*this, bitIndex); }

        /** @brief Gets whether no component types are present. */
        [[nodiscard]] bool getIsEmptyProperty() const
        {
            return bits_[0] == 0 && bits_[1] == 0 && bits_[2] == 0 && bits_[3] == 0;
        }

        /** @brief Determines if all component types in @p required are present in this set. */
        [[nodiscard]] bool HasAll(const ComponentBits& required) const
        {
            for (std::size_t i = 0; i < SegmentCount; ++i)
            {
                if ((bits_[i] & required.bits_[i]) != required.bits_[i])
                {
                    return false;
                }
            }
            return true;
        }

        /** @brief Determines if this set contains at least one of the component types in @p options. */
        [[nodiscard]] bool HasAny(const ComponentBits& options) const
        {
            for (std::size_t i = 0; i < SegmentCount; ++i)
            {
                if ((bits_[i] & options.bits_[i]) != 0)
                {
                    return true;
                }
            }
            return false;
        }

        /** @brief Determines if this set contains none of the component types in @p excluded. */
        [[nodiscard]] bool HasNone(const ComponentBits& excluded) const { return !HasAny(excluded); }

        /** @brief Removes all component types from this set. */
        void Clear() { bits_.fill(0); }

        /** @brief Marks all possible component types as present in this set. */
        void SetAll() { bits_.fill(~static_cast<std::uint64_t>(0)); }

        [[nodiscard]] bool operator==(const ComponentBits& other) const { return bits_ == other.bits_; }
        [[nodiscard]] bool operator!=(const ComponentBits& other) const { return !(*this == other); }

        [[nodiscard]] friend ComponentBits operator|(const ComponentBits& left, const ComponentBits& right)
        {
            ComponentBits result;
            for (std::size_t i = 0; i < SegmentCount; ++i)
            {
                result.bits_[i] = left.bits_[i] | right.bits_[i];
            }
            return result;
        }

        [[nodiscard]] friend ComponentBits operator&(const ComponentBits& left, const ComponentBits& right)
        {
            ComponentBits result;
            for (std::size_t i = 0; i < SegmentCount; ++i)
            {
                result.bits_[i] = left.bits_[i] & right.bits_[i];
            }
            return result;
        }

        [[nodiscard]] friend ComponentBits operator~(const ComponentBits& value)
        {
            ComponentBits result;
            for (std::size_t i = 0; i < SegmentCount; ++i)
            {
                result.bits_[i] = ~value.bits_[i];
            }
            return result;
        }

    private:
        [[nodiscard]] bool Get(int bitIndex) const
        {
            CheckIndex(bitIndex);
            const auto segmentIndex = static_cast<std::size_t>(bitIndex >> 6);
            const int bitPosition = bitIndex & 63;
            const std::uint64_t mask = std::uint64_t{1} << bitPosition;
            return (bits_[segmentIndex] & mask) != 0;
        }

        void Set(int bitIndex, bool value)
        {
            CheckIndex(bitIndex);
            const auto segmentIndex = static_cast<std::size_t>(bitIndex >> 6);
            const int bitPosition = bitIndex & 63;
            const std::uint64_t mask = std::uint64_t{1} << bitPosition;
            if (value)
            {
                bits_[segmentIndex] |= mask;
            }
            else
            {
                bits_[segmentIndex] &= ~mask;
            }
        }

        static void CheckIndex(int bitIndex)
        {
            if (bitIndex < 0 || bitIndex >= MaxBits)
            {
                throw std::out_of_range("bitIndex must be between 0 and 255.");
            }
        }

        std::array<std::uint64_t, SegmentCount> bits_{};
    };
}
