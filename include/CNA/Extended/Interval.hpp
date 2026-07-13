// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Interval.cs. Upstream constrains T via `where T :
// IComparable<T>` and compares with `T.CompareTo(T)`; C++ has no equivalent interface constraint
// for primitive types (int/float have no .CompareTo() method), so this uses ordinary
// comparison operators (<, ==) instead -- the natural C++ translation of "T is orderable",
// preserving the exact same ordering semantics. Header-only: a template, like sharp-runtime's
// IEquatable<T>/IEquatableByRef<T>.
#pragma once

#include "System/HashCode.hpp"

#include <stdexcept>
#include <string>

namespace CNA::Extended
{
    /**
     * @brief Represents a closed mathematical interval [Min, Max] defined by comparable bounds.
     * @tparam T The type of values contained in the interval. Must support operator< and
     * operator==.
     */
    template <typename T>
    class Interval
    {
    public:
        /**
         * @brief Initializes an interval with the specified bounds.
         * @param min The minimum bound of the interval.
         * @param max The maximum bound of the interval.
         * @throws std::invalid_argument if @p min is greater than @p max.
         */
        Interval(T min, T max) : min_(min), max_(max), isEmpty_(false)
        {
            if (max < min)
            {
                throw std::invalid_argument("Minimum bounds cannot be greater than maximum bounds");
            }
        }

        /**
         * @brief Initializes a degenerate interval containing only the specified value. Not
         * marked explicit: upstream has both a `Interval(T value)` constructor and a separate
         * `implicit operator Interval<T>(T value)` conversion; a single non-explicit C++
         * constructor here plays both roles at once.
         * @param value The single value to be contained in the interval.
         */
        Interval(T value) : Interval(value, value) // NOLINT(*-explicit-constructor)
        {
        }

        /** @brief Gets an empty interval. */
        [[nodiscard]] static Interval Empty() { return Interval(true); }

        /**
         * @brief Gets the minimum bound of the interval.
         * @throws std::logic_error if the interval is empty.
         */
        [[nodiscard]] T getMinProperty() const
        {
            if (isEmpty_)
            {
                throw std::logic_error("Cannot access bounds of an empty interval");
            }
            return min_;
        }

        /**
         * @brief Gets the maximum bound of the interval.
         * @throws std::logic_error if the interval is empty.
         */
        [[nodiscard]] T getMaxProperty() const
        {
            if (isEmpty_)
            {
                throw std::logic_error("Cannot access bounds of an empty interval");
            }
            return max_;
        }

        /** @brief Gets a value indicating whether the interval contains no values. */
        [[nodiscard]] bool getIsEmptyProperty() const { return isEmpty_; }

        /** @brief Gets a value indicating whether the interval contains exactly one value. */
        [[nodiscard]] bool getIsDegenerateProperty() const { return !isEmpty_ && min_ == max_; }

        /** @brief Gets a value indicating whether the interval contains more than one value. */
        [[nodiscard]] bool getIsProperProperty() const { return !isEmpty_ && !(min_ == max_); }

        /**
         * @brief Determines whether the interval contains the specified value.
         * @param value The value to test for containment.
         */
        [[nodiscard]] bool Contains(const T& value) const
        {
            if (isEmpty_)
            {
                return false;
            }
            return !(value < min_) && !(max_ < value);
        }

        /**
         * @brief Determines whether this interval completely contains another interval.
         * @param other The interval to test for containment.
         */
        [[nodiscard]] bool Contains(const Interval& other) const
        {
            if (other.isEmpty_)
            {
                return true;
            }
            if (isEmpty_)
            {
                return false;
            }
            return !(other.min_ < min_) && !(max_ < other.max_);
        }

        /**
         * @brief Determines whether this interval shares any values with another interval.
         * @param other The interval to test for overlap.
         */
        [[nodiscard]] bool Overlap(const Interval& other) const
        {
            if (isEmpty_ || other.isEmpty_)
            {
                return false;
            }
            return !(other.max_ < min_) && !(max_ < other.min_);
        }

        /**
         * @brief Computes the intersection of this interval with another interval.
         * @param other The interval to intersect with.
         * @return An interval containing only values present in both intervals, or Empty() if
         * no intersection exists.
         */
        [[nodiscard]] Interval Intersect(const Interval& other) const
        {
            if (isEmpty_ || other.isEmpty_ || !Overlap(other))
            {
                return Empty();
            }

            const T minBound = !(min_ < other.min_) ? min_ : other.min_;
            const T maxBound = !(other.max_ < max_) ? max_ : other.max_;
            return Interval(minBound, maxBound);
        }

        /**
         * @brief Computes the smallest interval containing both this interval and another interval.
         * @param other The interval to compute the hull with.
         */
        [[nodiscard]] Interval Hull(const Interval& other) const
        {
            if (isEmpty_)
            {
                return other;
            }
            if (other.isEmpty_)
            {
                return *this;
            }

            const T minBound = !(other.min_ < min_) ? min_ : other.min_;
            const T maxBound = !(max_ < other.max_) ? max_ : other.max_;
            return Interval(minBound, maxBound);
        }

        /**
         * @brief Creates an interval that spans both specified values.
         * @param a The first value.
         * @param b The second value.
         */
        [[nodiscard]] static Interval Hull(const T& a, const T& b)
        {
            if (!(b < a))
            {
                return Interval(a, b);
            }
            return Interval(b, a);
        }

        [[nodiscard]] bool Equals(const Interval& other) const
        {
            if (isEmpty_ && other.isEmpty_)
            {
                return true;
            }
            if (isEmpty_ != other.isEmpty_)
            {
                return false;
            }
            return min_ == other.min_ && max_ == other.max_;
        }

        [[nodiscard]] int GetHashCode() const
        {
            if (isEmpty_)
            {
                return 0;
            }
            return System::HashCode::Combine(min_, max_);
        }

        friend bool operator==(const Interval& left, const Interval& right) { return left.Equals(right); }
        friend bool operator!=(const Interval& left, const Interval& right) { return !left.Equals(right); }

        [[nodiscard]] std::string ToString() const
        {
            if (isEmpty_)
            {
                return "\xE2\x88\x85"; // U+2205 EMPTY SET
            }
            if (getIsDegenerateProperty())
            {
                return "[" + std::to_string(min_) + "]";
            }
            return "[" + std::to_string(min_) + ", " + std::to_string(max_) + "]";
        }

    private:
        explicit Interval(bool isEmpty) : min_(T{}), max_(T{}), isEmpty_(isEmpty)
        {
        }

        T min_;
        T max_;
        bool isEmpty_;
    };
}
