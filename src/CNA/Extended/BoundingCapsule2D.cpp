// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/BoundingCapsule2D.hpp"

#include <cmath>
#include <functional>
#include <numbers>

namespace CNA::Extended
{
    namespace
    {
        // Duplicates MonoGame.Extended's Collision2D.Epsilon (Collision2D.cs:44), which is not yet
        // ported (scheduled for Phase 2). Point this at Collision2D::Epsilon once that lands.
        constexpr float kCollision2DEpsilonPending = 1e-6f;
    }

    BoundingCapsule2D::BoundingCapsule2D(const Vector2& pointA, const Vector2& pointB, const float radius)
        : PointA(pointA), PointB(pointB), Radius(radius)
    {
    }

    float BoundingCapsule2D::getLengthProperty() const
    {
        return std::sqrt(getLengthSquaredProperty());
    }

    float BoundingCapsule2D::getLengthSquaredProperty() const
    {
        const Vector2 diff = PointB - PointA;
        return diff.X * diff.X + diff.Y * diff.Y;
    }

    Vector2 BoundingCapsule2D::getDirectionProperty() const
    {
        const Vector2 dir = PointB - PointA;
        const float lengthSquared = dir.X * dir.X + dir.Y * dir.Y;

        if (lengthSquared < kCollision2DEpsilonPending * kCollision2DEpsilonPending)
        {
            return Vector2::Zero;
        }

        return dir / std::sqrt(lengthSquared);
    }

    float BoundingCapsule2D::getAreaProperty() const
    {
        const float length = getLengthProperty();
        return length * (2.0f * Radius) + std::numbers::pi_v<float> * Radius * Radius;
    }

    BoundingCapsule2D BoundingCapsule2D::CreateFromCenterAndDirection(
        const Vector2& center, const Vector2& direction, const float length, const float radius)
    {
        // Check if the direction needs to be normalized and normalize it.
        const float lengthSq = direction.LengthSquared();
        Vector2 normalizedDir = Vector2::Zero;
        if (lengthSq >= kCollision2DEpsilonPending * kCollision2DEpsilonPending)
        {
            normalizedDir = direction / std::sqrt(lengthSq);
        }

        const Vector2 halfExtent = normalizedDir * (length * 0.5f);

        return BoundingCapsule2D(center - halfExtent, center + halfExtent, radius);
    }

    BoundingCapsule2D BoundingCapsule2D::Transform(const Matrix& matrix) const
    {
        const Vector2 transformedA = Vector2::Transform(PointA, matrix);
        const Vector2 transformedB = Vector2::Transform(PointB, matrix);

        // Scale radius by maximum scale component
        const float scaleX = std::sqrt(matrix.M11 * matrix.M11 + matrix.M12 * matrix.M12);
        const float scaleY = std::sqrt(matrix.M21 * matrix.M21 + matrix.M22 * matrix.M22);
        const float transformedRadius = Radius * std::max(scaleX, scaleY);

        return BoundingCapsule2D(transformedA, transformedB, transformedRadius);
    }

    BoundingCapsule2D BoundingCapsule2D::Translate(const Vector2& translation) const
    {
        return BoundingCapsule2D(PointA + translation, PointB + translation, Radius);
    }

    void BoundingCapsule2D::Deconstruct(Vector2& pointA, Vector2& pointB, float& radius) const
    {
        pointA = PointA;
        pointB = PointB;
        radius = Radius;
    }

    bool BoundingCapsule2D::Equals(const BoundingCapsule2D& other) const
    {
        return PointA == other.PointA && PointB == other.PointB && Radius == other.Radius;
    }

    int BoundingCapsule2D::GetHashCode() const
    {
        return PointA.GetHashCode() ^ PointB.GetHashCode() ^ static_cast<int>(std::hash<float>{}(Radius));
    }

    std::string BoundingCapsule2D::ToString() const
    {
        return "{PointA:" + PointA.ToString() + " PointB:" + PointB.ToString() + " Radius:" + std::to_string(Radius) + "}";
    }
}
