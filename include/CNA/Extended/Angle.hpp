// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Angle.cs. Upstream's own file header additionally credits
// the SlimMath project (http://code.google.com/p/slimmath/) as the origin of this code:
//   Copyright (c) 2007-2010 SlimDX Group, MIT License.
// Carried forward here for the same reason MonoGame.Extended carries it: this type traces back
// further than Craftwork Games. See NOTICE.md.
//
// Fidelity note: upstream's Equals(Angle)/CompareTo(Angle) are NOT `readonly` methods -- they
// call WrapPositive() on `this` (mutating it in place) and on a *by-value copy* of the `other`
// parameter. This is preserved exactly: Equals/CompareTo below are non-const and take `other` by
// value, so calling either one has the side effect of wrapping this Angle's Radians into
// (-pi, pi]. operator==/!= take both operands by value for the same reason (upstream's operator
// overloads take Angle by value, not by ref). GetHashCode() does NOT call WrapPositive() first
// (unlike Equals) -- an apparent inconsistency in upstream between its equality and hashing
// contracts, preserved as-is rather than "fixed", since this is a faithful port. Also: upstream's
// `WrapPositive()` ends with a literal `Radians = Radians;` self-assignment (a no-op with zero
// observable effect); omitted here as dead code, not as a behavioral simplification.
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <cstdint>
#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    enum class AngleType : std::uint8_t
    {
        Radian = 0,
        Degree,
        Revolution, // or Turn / cycle
        Gradian     // or Gon
    };

    /** @brief Represents an angle, convertible between radians, degrees, gradians, and revolutions. */
    class Angle
    {
    public:
        /**
         * @brief Initializes a new Angle from the specified value, interpreted according to
         * @p angleType. Marked explicit to match upstream's explicit-only float conversion
         * (there is no implicit float -> Angle conversion, only Angle -> float, see operator
         * float() below).
         * @param value The angle value, in the units specified by @p angleType.
         * @param angleType The unit that @p value is expressed in. Defaults to radians.
         */
        explicit Angle(float value, AngleType angleType = AngleType::Radian);

        /** @brief Gets the angle value in radians. */
        [[nodiscard]] float getRadiansProperty() const { return radians_; }
        /** @brief Sets the angle value in radians. */
        void setRadiansProperty(const float value) { radians_ = value; }

        /** @brief Gets the angle value in degrees. */
        [[nodiscard]] float getDegreesProperty() const;
        /** @brief Sets the angle value in degrees. */
        void setDegreesProperty(float value);

        /** @brief Gets the angle value in gradians. */
        [[nodiscard]] float getGradiansProperty() const;
        /** @brief Sets the angle value in gradians. */
        void setGradiansProperty(float value);

        /** @brief Gets the angle value in revolutions (turns/cycles). */
        [[nodiscard]] float getRevolutionsProperty() const;
        /** @brief Sets the angle value in revolutions (turns/cycles). */
        void setRevolutionsProperty(float value);

        /** @brief Gets the angle value in the units specified by @p angleType. */
        [[nodiscard]] float GetValue(AngleType angleType) const;

        /**
         * @brief Wraps this angle's value to the range (-pi, pi], preserving upstream's exact
         * (unusual, sequential-if rather than if/else) wrapping logic.
         */
        void Wrap();

        /** @brief Wraps this angle's value to the range [0, tau). */
        void WrapPositive();

        /**
         * @brief Creates an Angle from a direction vector, using the angle between the vector
         * and the positive X-axis (Y is negated to match screen-space Y-down conventions).
         */
        [[nodiscard]] static Angle FromVector(const Vector2& vector);

        /** @brief Converts this angle to a unit-length direction vector. */
        [[nodiscard]] Vector2 ToUnitVector() const;

        /** @brief Converts this angle to a direction vector of the specified length. */
        [[nodiscard]] Vector2 ToVector(float length) const;

        /**
         * @brief Determines whether @p value falls within the range [min, end], wrapping around
         * if @p end is less than @p min.
         */
        [[nodiscard]] static bool IsBetween(const Angle& value, const Angle& min, const Angle& end);

        /**
         * @brief Compares this angle to another. Mutates both this angle and (a local copy of)
         * @p other via WrapPositive() first -- see the file-level fidelity note.
         */
        [[nodiscard]] int CompareTo(Angle other);

        /**
         * @brief Determines equality with another angle. Mutates both this angle and (a local
         * copy of) @p other via WrapPositive() first -- see the file-level fidelity note.
         */
        [[nodiscard]] bool Equals(Angle other);

        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        /** @brief Implicitly converts this Angle to its value in radians (matches upstream's implicit operator). */
        [[nodiscard]] operator float() const { return radians_; } // NOLINT(*-explicit-constructor)

        [[nodiscard]] Angle operator-() const;

        friend bool operator==(Angle a, Angle b) { return a.Equals(b); }
        friend bool operator!=(Angle a, Angle b) { return !a.Equals(b); }

        friend Angle operator-(const Angle& left, const Angle& right);
        friend Angle operator*(const Angle& left, float right);
        friend Angle operator*(float left, const Angle& right);
        friend Angle operator+(const Angle& left, const Angle& right);

    private:
        static constexpr float kTau = 6.283185307179586f;       // 2*pi
        static constexpr float kTauInv = 0.15915494309189535f;  // 1/tau
        static constexpr float kDegreeRadian = 0.017453292519943295f; // pi/180
        static constexpr float kRadianDegree = 57.29577951308232f;    // 180/pi
        static constexpr float kGradianRadian = 0.015707963267948967f; // pi/200
        static constexpr float kRadianGradian = 63.66197723675813f;    // 200/pi

        float radians_ = 0.0f;
    };
}
