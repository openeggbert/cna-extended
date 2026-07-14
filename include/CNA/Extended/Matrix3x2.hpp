// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Matrix3x2.cs. Upstream's X/Y/Z row properties use
// `Unsafe.As<float, Vector2>` to alias pairs of fields as a Vector2 without copying; C++ has no
// equally safe, portable equivalent (reinterpret_cast through unrelated types here would violate
// strict aliasing), so getXProperty()/getYProperty()/getZProperty() construct a Vector2 by value
// instead -- same observable values, different (safe) mechanism. `internal string
// DebugDisplayString()` is a debugger-only helper (matches the precedent of skipping such members
// elsewhere in this project) and is not ported. `object obj` Equals overload not ported (no C++
// equivalent), matching precedent.
#pragma once

#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <optional>
#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector2;

    /**
     * @brief Represents a 3x2 matrix using floating point values for each component that can
     * store two dimensional translation, scale, and rotation information for a right-handed
     * coordinate system.
     *
     * Matrices use a row vector layout: row vector matrices view vectors as a row from left to
     * right. The translation corresponds to the fields M31 and M32. The fields M13/M23 always
     * have a value of 0.0f, and M33 always has a value of 1.0f; all three are removed from this
     * type to reduce its memory footprint (matching upstream).
     */
    struct Matrix3x2
    {
        /** @brief Scaling factor on the x-axis or a combination of scaling and rotation. */
        float M11 = 0.0f;
        /** @brief Shearing factor on the y-axis or a combination of shearing and rotation. */
        float M12 = 0.0f;
        /** @brief Shearing factor on the x-axis or a combination of shear and rotation. */
        float M21 = 0.0f;
        /** @brief Scaling factor on the y-axis or a combination of scale and rotation. */
        float M22 = 0.0f;
        /** @brief The translation on the x-axis. */
        float M31 = 0.0f;
        /** @brief The translation on the y-axis. */
        float M32 = 0.0f;

        /** @brief Gets the vector formed by the first row of this Matrix3x2 (M11, M12). */
        [[nodiscard]] Vector2 getXProperty() const { return Vector2(M11, M12); }
        /** @brief Sets the vector formed by the first row of this Matrix3x2. */
        void setXProperty(const Vector2& value) { M11 = value.X; M12 = value.Y; }

        /** @brief Gets the vector formed by the second row of this Matrix3x2 (M21, M22). */
        [[nodiscard]] Vector2 getYProperty() const { return Vector2(M21, M22); }
        /** @brief Sets the vector formed by the second row of this Matrix3x2. */
        void setYProperty(const Vector2& value) { M21 = value.X; M22 = value.Y; }

        /** @brief Gets the vector formed by the third row of this Matrix3x2 (M31, M32). */
        [[nodiscard]] Vector2 getZProperty() const { return Vector2(M31, M32); }
        /** @brief Sets the vector formed by the third row of this Matrix3x2. */
        void setZProperty(const Vector2& value) { M31 = value.X; M32 = value.Y; }

        /** @brief The multiplicative identity matrix. */
        static const Matrix3x2 Identity;

        Matrix3x2() = default;

        /** @brief Creates a 3x2 matrix from the specified components. */
        Matrix3x2(float m11, float m12, float m21, float m22, float m31, float m32);

        /** @brief Creates a new 3x2 matrix from the specified row vectors. */
        Matrix3x2(const Vector2& x, const Vector2& y, const Vector2& z);

        /** @brief Transforms the given vector by this Matrix3x2. */
        [[nodiscard]] Vector2 Transform(const Vector2& vector) const;
        /** @brief Transforms the given vector by this Matrix3x2. */
        void Transform(const Vector2& vector, Vector2& result) const;
        /** @brief Transforms a vector with the specified x/y component values by this Matrix3x2. */
        [[nodiscard]] Vector2 Transform(float x, float y) const;
        /** @brief Transforms a vector with the specified x/y component values by this Matrix3x2. */
        void Transform(float x, float y, Vector2& result) const;
        /**
         * @brief Transforms a vector with the specified x/y component values by this Matrix3x2,
         * writing into the X/Y components of an existing Vector3 (its Z is left untouched).
         */
        void Transform(float x, float y, Microsoft::Xna::Framework::Vector3& result) const;

        /**
         * @brief Calculates the determinant of this Matrix3x2, computed by expanding this matrix
         * with a third column whose values are (0, 0, 1).
         */
        [[nodiscard]] float Determinant() const;

        /** @brief Deconstructs this Matrix3x2 into its translation, rotation, and scale components. */
        void Decompose(Vector2& translation, float& rotation, Vector2& scale) const;

        /**
         * @brief Creates a new Matrix3x2 value for 2D translation, rotation, and scale.
         * Rotation is performed along the z-axis.
         */
        [[nodiscard]] static Matrix3x2 CreateFrom(
            const Vector2& position, float rotation, const std::optional<Vector2>& scale, const std::optional<Vector2>& origin);
        /** @brief Creates a new Matrix3x2 value for 2D translation, rotation, and scale. */
        static void CreateFrom(const Vector2& position, float rotation, const std::optional<Vector2>& scale,
            const std::optional<Vector2>& origin, Matrix3x2& result);

        /** @brief Creates a Matrix3x2 value for 2D rotation along the z-axis. */
        [[nodiscard]] static Matrix3x2 CreateRotationZ(float radians);
        /** @brief Creates a Matrix3x2 value for 2D rotation along the z-axis. */
        static void CreateRotationZ(float radians, Matrix3x2& result);

        /** @brief Creates a Matrix3x2 value for uniform 2D scaling. */
        [[nodiscard]] static Matrix3x2 CreateScale(float scale);
        /** @brief Creates a Matrix3x2 value for uniform 2D scaling. */
        static void CreateScale(float scale, Matrix3x2& result);
        /** @brief Creates a Matrix3x2 value for 2D scaling with a per-axis scale vector. */
        [[nodiscard]] static Matrix3x2 CreateScale(const Vector2& scale);
        /** @brief Creates a Matrix3x2 value for 2D scaling with a per-axis scale vector. */
        static void CreateScale(const Vector2& scale, Matrix3x2& result);
        /** @brief Creates a Matrix3x2 value for 2D scaling with independent x/y scale factors. */
        [[nodiscard]] static Matrix3x2 CreateScale(float xScale, float yScale);
        /** @brief Creates a Matrix3x2 value for 2D scaling with independent x/y scale factors. */
        static void CreateScale(float xScale, float yScale, Matrix3x2& result);

        /** @brief Creates a Matrix3x2 value for 2D translation. */
        [[nodiscard]] static Matrix3x2 CreateTranslation(const Vector2& vector);
        /** @brief Creates a Matrix3x2 value for 2D translation. */
        static void CreateTranslation(const Vector2& vector, Matrix3x2& result);
        /** @brief Creates a Matrix3x2 value for 2D translation. */
        [[nodiscard]] static Matrix3x2 CreateTranslation(float x, float y);
        /** @brief Creates a Matrix3x2 value for 2D translation. */
        static void CreateTranslation(float x, float y, Matrix3x2& result);

        /** @brief Inverts the provided Matrix3x2. */
        [[nodiscard]] static Matrix3x2 Invert(Matrix3x2 matrix);
        /** @brief Inverts the provided Matrix3x2 in place. */
        static void Invert(Matrix3x2& matrix);

        /** @brief Adds the elements of two Matrix3x2 values component-wise. */
        [[nodiscard]] static Matrix3x2 Add(const Matrix3x2& left, const Matrix3x2& right);
        /** @brief Adds the elements of two Matrix3x2 values component-wise. */
        static void Add(const Matrix3x2& left, const Matrix3x2& right, Matrix3x2& result);
        /** @brief Subtracts the elements of a Matrix3x2 from another, component-wise. */
        [[nodiscard]] static Matrix3x2 Subtract(const Matrix3x2& left, const Matrix3x2& right);
        /** @brief Subtracts the elements of a Matrix3x2 from another, component-wise. */
        static void Subtract(const Matrix3x2& left, const Matrix3x2& right, Matrix3x2& result);
        /** @brief Multiplies two Matrix3x2 values (matrix multiplication). */
        [[nodiscard]] static Matrix3x2 Multiply(const Matrix3x2& left, const Matrix3x2& right);
        /** @brief Multiplies two Matrix3x2 values (matrix multiplication). */
        static void Multiply(const Matrix3x2& left, const Matrix3x2& right, Matrix3x2& result);
        /** @brief Multiplies the elements of a Matrix3x2 by a scalar value, component-wise. */
        [[nodiscard]] static Matrix3x2 Multiply(const Matrix3x2& matrix, float scalar);
        /** @brief Multiplies the elements of a Matrix3x2 by a scalar value, component-wise. */
        static void Multiply(const Matrix3x2& matrix, float scalar, Matrix3x2& result);
        /** @brief Divides the elements of a Matrix3x2 by another, component-wise. */
        [[nodiscard]] static Matrix3x2 Divide(const Matrix3x2& left, const Matrix3x2& right);
        /** @brief Divides the elements of a Matrix3x2 by another, component-wise. */
        static void Divide(const Matrix3x2& left, const Matrix3x2& right, Matrix3x2& result);
        /** @brief Divides the elements of a Matrix3x2 by a scalar value, component-wise. */
        [[nodiscard]] static Matrix3x2 Divide(const Matrix3x2& matrix, float scalar);
        /** @brief Divides the elements of a Matrix3x2 by a scalar value, component-wise. */
        static void Divide(const Matrix3x2& matrix, float scalar, Matrix3x2& result);

        [[nodiscard]] bool Equals(const Matrix3x2& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const Matrix3x2& left, const Matrix3x2& right) { return left.Equals(right); }
        friend bool operator!=(const Matrix3x2& left, const Matrix3x2& right) { return !left.Equals(right); }

        friend Matrix3x2 operator+(const Matrix3x2& left, const Matrix3x2& right);
        friend Matrix3x2 operator-(const Matrix3x2& left, const Matrix3x2& right);
        /** @brief Negates the elements of a Matrix3x2, component-wise. */
        [[nodiscard]] Matrix3x2 operator-() const;
        friend Matrix3x2 operator*(const Matrix3x2& left, const Matrix3x2& right);
        friend Matrix3x2 operator*(const Matrix3x2& matrix, float scalar);
        friend Matrix3x2 operator/(const Matrix3x2& left, const Matrix3x2& right);
        friend Matrix3x2 operator/(const Matrix3x2& matrix, float scalar);

        /**
         * @brief Converts this Matrix3x2 value into a Matrix value. The third row of the
         * resulting Matrix is set to (0, 0, 1, 0); the fourth row is set to (M31, M32, depth, 1).
         */
        [[nodiscard]] Matrix ToMatrix(float depth = 0.0f) const;
        /** @brief Converts this Matrix3x2 value into a Matrix value, with depth 0. */
        void ToMatrix(Matrix& result) const;
        /** @brief Converts this Matrix3x2 value into a Matrix value. */
        void ToMatrix(float depth, Matrix& result) const;
        /** @brief Converts a Matrix3x2 value into a Matrix value. */
        [[nodiscard]] static Matrix ToMatrix(const Matrix3x2& matrix, float depth = 0.0f);
        /** @brief Converts a Matrix3x2 value into a Matrix value, with depth 0. */
        static void ToMatrix(const Matrix3x2& matrix, Matrix& result);
        /** @brief Converts a Matrix3x2 value into a Matrix value. */
        static void ToMatrix(const Matrix3x2& matrix, float depth, Matrix& result);

        /** @brief Implicitly converts a Matrix3x2 value to a Matrix value (matches upstream's implicit operator). */
        operator Matrix() const; // NOLINT(*-explicit-constructor)

    private:
        // Upstream has both `Invert(Matrix3x2 matrix) -> Matrix3x2` (by value) and
        // `Invert(ref Matrix3x2 matrix)` (in place) with the same single-parameter shape; C#'s
        // `ref` keyword disambiguates these at the call site, but C++ overload resolution treats
        // f(T) and f(T&) as equally viable for a mutable lvalue argument, which is ambiguous.
        // Both public overloads delegate to this private helper instead of calling each other.
        static void InvertImpl(Matrix3x2& matrix);
    };
}
