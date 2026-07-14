// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Matrix3x2.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/HashCode.hpp"

#include <cmath>
#include <limits>
#include <sstream>

namespace CNA::Extended
{
    // Deliberately NOT Matrix3x2(Vector2::UnitX, Vector2::UnitY, Vector2::Zero): those are
    // themselves static objects in a different translation unit (CNA's Vector2.cpp), and C++
    // gives no guarantee about cross-TU static initialization order ("static initialization
    // order fiasco") -- if this ran first, Identity would silently become all-zero. Literal
    // values have no such dependency.
    const Matrix3x2 Matrix3x2::Identity = Matrix3x2(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);

    Matrix3x2::Matrix3x2(const float m11, const float m12, const float m21, const float m22, const float m31, const float m32)
        : M11(m11), M12(m12), M21(m21), M22(m22), M31(m31), M32(m32)
    {
    }

    Matrix3x2::Matrix3x2(const Vector2& x, const Vector2& y, const Vector2& z)
        : Matrix3x2(x.X, x.Y, y.X, y.Y, z.X, z.Y)
    {
    }

    Vector2 Matrix3x2::Transform(const Vector2& vector) const
    {
        return Transform(vector.X, vector.Y);
    }

    void Matrix3x2::Transform(const Vector2& vector, Vector2& result) const
    {
        Transform(vector.X, vector.Y, result);
    }

    Vector2 Matrix3x2::Transform(const float x, const float y) const
    {
        Vector2 result;
        Transform(x, y, result);
        return result;
    }

    void Matrix3x2::Transform(const float x, const float y, Vector2& result) const
    {
        result.X = (x * M11) + (y * M21) + M31;
        result.Y = (x * M12) + (y * M22) + M32;
    }

    void Matrix3x2::Transform(const float x, const float y, Microsoft::Xna::Framework::Vector3& result) const
    {
        result.X = (x * M11) + (y * M21) + M31;
        result.Y = (x * M12) + (y * M22) + M32;
    }

    float Matrix3x2::Determinant() const
    {
        return (M11 * M22) - (M12 * M21);
    }

    void Matrix3x2::Decompose(Vector2& translation, float& rotation, Vector2& scale) const
    {
        translation.X = M31;
        translation.Y = M32;

        rotation = std::atan2(M21, M11);

        const float x = M11 * M11 + M21 * M21;
        const float y = M12 * M12 + M22 * M22;

        const float xSign = x < 0.0f ? -1.0f : 1.0f;
        const float ySign = y < 0.0f ? -1.0f : 1.0f;

        scale.X = xSign * std::sqrt(x);
        scale.Y = ySign * std::sqrt(y);
    }

    Matrix3x2 Matrix3x2::CreateFrom(
        const Vector2& position, const float rotation, const std::optional<Vector2>& scale, const std::optional<Vector2>& origin)
    {
        Matrix3x2 result;
        CreateFrom(position, rotation, scale, origin, result);
        return result;
    }

    void Matrix3x2::CreateFrom(const Vector2& position, const float rotation, const std::optional<Vector2>& scale,
        const std::optional<Vector2>& origin, Matrix3x2& result)
    {
        result = Identity;

        if (origin.has_value())
        {
            result.setZProperty(-origin.value());
        }

        if (scale.has_value())
        {
            Matrix3x2 scaleMatrix;
            CreateScale(scale.value(), scaleMatrix);
            Multiply(result, scaleMatrix, result);
        }

        if (rotation != 0.0f)
        {
            Matrix3x2 rotationMatrix;
            CreateRotationZ(-rotation, rotationMatrix);
            Multiply(result, rotationMatrix, result);
        }

        Matrix3x2 translationMatrix;
        CreateTranslation(position, translationMatrix);
        Multiply(result, translationMatrix, result);
    }

    Matrix3x2 Matrix3x2::CreateRotationZ(const float radians)
    {
        Matrix3x2 result;
        CreateRotationZ(radians, result);
        return result;
    }

    void Matrix3x2::CreateRotationZ(const float radians, Matrix3x2& result)
    {
        const float cos = std::cos(radians);
        const float sin = std::sin(radians);

        result.M11 = cos;
        result.M12 = sin;

        result.M21 = -sin;
        result.M22 = cos;

        result.M31 = 0;
        result.M32 = 0;
    }

    Matrix3x2 Matrix3x2::CreateScale(const float scale)
    {
        Matrix3x2 result;
        CreateScale(scale, scale, result);
        return result;
    }

    void Matrix3x2::CreateScale(const float scale, Matrix3x2& result)
    {
        CreateScale(scale, scale, result);
    }

    Matrix3x2 Matrix3x2::CreateScale(const Vector2& scale)
    {
        Matrix3x2 result;
        CreateScale(scale.X, scale.Y, result);
        return result;
    }

    void Matrix3x2::CreateScale(const Vector2& scale, Matrix3x2& result)
    {
        CreateScale(scale.X, scale.Y, result);
    }

    Matrix3x2 Matrix3x2::CreateScale(const float xScale, const float yScale)
    {
        Matrix3x2 result;
        CreateScale(xScale, yScale, result);
        return result;
    }

    void Matrix3x2::CreateScale(const float xScale, const float yScale, Matrix3x2& result)
    {
        result.M11 = xScale;
        result.M12 = 0;

        result.M21 = 0;
        result.M22 = yScale;

        result.M31 = 0;
        result.M32 = 0;
    }

    Matrix3x2 Matrix3x2::CreateTranslation(const Vector2& vector)
    {
        return CreateTranslation(vector.X, vector.Y);
    }

    void Matrix3x2::CreateTranslation(const Vector2& vector, Matrix3x2& result)
    {
        CreateTranslation(vector.X, vector.Y, result);
    }

    Matrix3x2 Matrix3x2::CreateTranslation(const float x, const float y)
    {
        Matrix3x2 result;
        CreateTranslation(x, y, result);
        return result;
    }

    void Matrix3x2::CreateTranslation(const float x, const float y, Matrix3x2& result)
    {
        result.M11 = 1;
        result.M12 = 0;

        result.M21 = 0;
        result.M22 = 1;

        result.M31 = x;
        result.M32 = y;
    }

    Matrix3x2 Matrix3x2::Invert(Matrix3x2 matrix)
    {
        InvertImpl(matrix);
        return matrix;
    }

    void Matrix3x2::Invert(Matrix3x2& matrix)
    {
        InvertImpl(matrix);
    }

    void Matrix3x2::InvertImpl(Matrix3x2& matrix)
    {
        const float det = 1.0f / matrix.Determinant();

        if (std::isinf(det)) // Det(M) = 0
        {
            matrix = Identity;
            return;
        }

        // The new 3x2 matrix is the first and second column of the inverse of the 3x3 matrix
        // given by adding the third column (0, 0, 1) to the input matrix.
        matrix = Matrix3x2(matrix.M22 * det, -matrix.M12 * det, -matrix.M21 * det, matrix.M11 * det,
            (matrix.M32 * matrix.M21 - matrix.M31 * matrix.M22) * det, (matrix.M31 * matrix.M12 - matrix.M32 * matrix.M11) * det);
    }

    Matrix3x2 Matrix3x2::Add(const Matrix3x2& left, const Matrix3x2& right)
    {
        return left + right;
    }

    void Matrix3x2::Add(const Matrix3x2& left, const Matrix3x2& right, Matrix3x2& result)
    {
        result = Add(left, right);
    }

    Matrix3x2 Matrix3x2::Subtract(const Matrix3x2& left, const Matrix3x2& right)
    {
        return left - right;
    }

    void Matrix3x2::Subtract(const Matrix3x2& left, const Matrix3x2& right, Matrix3x2& result)
    {
        result = Subtract(left, right);
    }

    Matrix3x2 Matrix3x2::Multiply(const Matrix3x2& left, const Matrix3x2& right)
    {
        return left * right;
    }

    void Matrix3x2::Multiply(const Matrix3x2& left, const Matrix3x2& right, Matrix3x2& result)
    {
        result = left * right;
    }

    Matrix3x2 Matrix3x2::Multiply(const Matrix3x2& matrix, const float scalar)
    {
        return matrix * scalar;
    }

    void Matrix3x2::Multiply(const Matrix3x2& matrix, const float scalar, Matrix3x2& result)
    {
        result = Multiply(matrix, scalar);
    }

    Matrix3x2 Matrix3x2::Divide(const Matrix3x2& left, const Matrix3x2& right)
    {
        return left / right;
    }

    void Matrix3x2::Divide(const Matrix3x2& left, const Matrix3x2& right, Matrix3x2& result)
    {
        result = Divide(left, right);
    }

    Matrix3x2 Matrix3x2::Divide(const Matrix3x2& matrix, const float scalar)
    {
        return matrix / scalar;
    }

    void Matrix3x2::Divide(const Matrix3x2& matrix, const float scalar, Matrix3x2& result)
    {
        result = Divide(matrix, scalar);
    }

    bool Matrix3x2::Equals(const Matrix3x2& other) const
    {
        return getXProperty() == other.getXProperty() && getYProperty() == other.getYProperty() &&
               getZProperty() == other.getZProperty();
    }

    int Matrix3x2::GetHashCode() const
    {
        // Matches upstream's HashCode.Combine(X, Y, Z); System::HashCode::Add<T> needs
        // std::hash<T>, which doesn't exist for Vector2, so it's fed each row's GetHashCode()
        // int result directly (same pattern used for BoundingPolygon2D::GetHashCode).
        return System::HashCode::Combine(getXProperty().GetHashCode(), getYProperty().GetHashCode(), getZProperty().GetHashCode());
    }

    std::string Matrix3x2::ToString() const
    {
        std::ostringstream stream;
        stream << "{M11:" << M11 << " M12:" << M12 << "} {M21:" << M21 << " M22:" << M22 << "} {M31:" << M31 << " M32:" << M32
               << "}";
        return stream.str();
    }

    Matrix3x2 operator+(const Matrix3x2& left, const Matrix3x2& right)
    {
        Matrix3x2 result;

        result.M11 = left.M11 + right.M11;
        result.M12 = left.M12 + right.M12;

        result.M21 = left.M21 + right.M21;
        result.M22 = left.M22 + right.M22;

        result.M31 = left.M31 + right.M31;
        result.M32 = left.M32 + right.M32;

        return result;
    }

    Matrix3x2 operator-(const Matrix3x2& left, const Matrix3x2& right)
    {
        Matrix3x2 result;

        result.M11 = left.M11 - right.M11;
        result.M12 = left.M12 - right.M12;

        result.M21 = left.M21 - right.M21;
        result.M22 = left.M22 - right.M22;

        result.M31 = left.M31 - right.M31;
        result.M32 = left.M32 - right.M32;

        return result;
    }

    Matrix3x2 Matrix3x2::operator-() const
    {
        Matrix3x2 result;

        result.M11 = -M11;
        result.M12 = -M12;

        result.M21 = -M21;
        result.M22 = -M22;

        result.M31 = -M31;
        result.M32 = -M32;

        return result;
    }

    Matrix3x2 operator*(const Matrix3x2& left, const Matrix3x2& right)
    {
        Matrix3x2 result;

        result.M11 = left.M11 * right.M11 + left.M12 * right.M21;
        result.M12 = left.M11 * right.M12 + left.M12 * right.M22;

        result.M21 = left.M21 * right.M11 + left.M22 * right.M21;
        result.M22 = left.M21 * right.M12 + left.M22 * right.M22;

        result.M31 = left.M31 * right.M11 + left.M32 * right.M21 + right.M31;
        result.M32 = left.M31 * right.M12 + left.M32 * right.M22 + right.M32;

        return result;
    }

    Matrix3x2 operator*(const Matrix3x2& matrix, const float scalar)
    {
        Matrix3x2 result;

        result.M11 = matrix.M11 * scalar;
        result.M12 = matrix.M12 * scalar;

        result.M21 = matrix.M21 * scalar;
        result.M22 = matrix.M22 * scalar;

        result.M31 = matrix.M31 * scalar;
        result.M32 = matrix.M32 * scalar;

        return result;
    }

    Matrix3x2 operator/(const Matrix3x2& left, const Matrix3x2& right)
    {
        Matrix3x2 result;

        result.M11 = left.M11 / right.M11;
        result.M12 = left.M12 / right.M12;

        result.M21 = left.M21 / right.M21;
        result.M22 = left.M22 / right.M22;

        result.M31 = left.M31 / right.M31;
        result.M32 = left.M32 / right.M32;

        return result;
    }

    Matrix3x2 operator/(const Matrix3x2& matrix, const float scalar)
    {
        const float num = 1.0f / scalar;
        Matrix3x2 result;

        result.M11 = matrix.M11 * num;
        result.M12 = matrix.M12 * num;

        result.M21 = matrix.M21 * num;
        result.M22 = matrix.M22 * num;

        result.M31 = matrix.M31 * num;
        result.M32 = matrix.M32 * num;

        return result;
    }

    Matrix Matrix3x2::ToMatrix(const float depth) const
    {
        return ToMatrix(*this, depth);
    }

    void Matrix3x2::ToMatrix(Matrix& result) const
    {
        ToMatrix(0.0f, result);
    }

    void Matrix3x2::ToMatrix(const float depth, Matrix& result) const
    {
        ToMatrix(*this, depth, result);
    }

    Matrix Matrix3x2::ToMatrix(const Matrix3x2& matrix, const float depth)
    {
        Matrix result;
        ToMatrix(matrix, depth, result);
        return result;
    }

    void Matrix3x2::ToMatrix(const Matrix3x2& matrix, Matrix& result)
    {
        ToMatrix(matrix, 0.0f, result);
    }

    void Matrix3x2::ToMatrix(const Matrix3x2& matrix, const float depth, Matrix& result)
    {
        result.M11 = matrix.M11;
        result.M12 = matrix.M12;
        result.M13 = 0;
        result.M14 = 0;

        result.M21 = matrix.M21;
        result.M22 = matrix.M22;
        result.M23 = 0;
        result.M24 = 0;

        result.M31 = 0;
        result.M32 = 0;
        result.M33 = 1;
        result.M34 = 0;

        result.M41 = matrix.M31;
        result.M42 = matrix.M32;
        result.M43 = depth;
        result.M44 = 1;
    }

    Matrix3x2::operator Matrix() const
    {
        return ToMatrix(*this, 0.0f);
    }
}
