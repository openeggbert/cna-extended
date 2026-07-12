// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Ray2D.hpp"

#include <cmath>
#include <stdexcept>

namespace CNA::Extended
{
    namespace
    {
        // Duplicates MonoGame.Extended's Collision2D.Epsilon (Collision2D.cs:44), which is not yet
        // ported (scheduled for Phase 2). Point this at Collision2D::Epsilon once that lands.
        constexpr float kCollision2DEpsilonPending = 1e-6f;
    }

    Ray2D::Ray2D(const Vector2& origin, const Vector2& direction) : Direction(direction), Origin(origin)
    {
    }

    Ray2D Ray2D::CreateFromPoints(const Vector2& start, const Vector2& through)
    {
        const Vector2 direction = through - start;
        const float lengthSq = direction.LengthSquared();

        if (lengthSq < kCollision2DEpsilonPending * kCollision2DEpsilonPending)
        {
            throw std::invalid_argument("Points must be distinct to define a ray direction");
        }

        return Ray2D(start, Vector2::Normalize(direction));
    }

    Vector2 Ray2D::GetPoint(const float distanceAlongRay) const
    {
        return Origin + distanceAlongRay * Direction;
    }

    Vector2 Ray2D::ClosestPoint(const Vector2& point, float& distanceAlongRay) const
    {
        const Vector2 ab = Direction;

        const float denom = Vector2::Dot(ab, ab);
        if (denom <= kCollision2DEpsilonPending)
        {
            // Direction of ray is effectively zero, so it's just a point.
            distanceAlongRay = 0.0f;
            return Origin;
        }

        // Project point ab, but deferring divide by Dot(ab, ab)
        distanceAlongRay = Vector2::Dot(point - Origin, ab);
        if (distanceAlongRay <= 0.0f)
        {
            // point projects before the ray origin, clamp to origin
            distanceAlongRay = 0.0f;
            return Origin;
        }

        // Point projects after the ray origin, must do deferred divide
        distanceAlongRay /= denom;
        return Origin + distanceAlongRay * ab;
    }

    float Ray2D::DistanceSquaredToPoint(const Vector2& point) const
    {
        float distanceAlongRay = 0.0f;
        const Vector2 closestPoint = ClosestPoint(point, distanceAlongRay);
        return Vector2::DistanceSquared(point, closestPoint);
    }

    float Ray2D::DistanceToPoint(const Vector2& point) const
    {
        const float distSq = DistanceSquaredToPoint(point);
        return std::sqrt(distSq);
    }

    void Ray2D::Normalize(const Ray2D& value, Ray2D& result)
    {
        result = Ray2D(value.Origin, Vector2::Normalize(value.Direction));
    }

    Ray2D Ray2D::Normalize(const Ray2D& value)
    {
        Ray2D result;
        Normalize(value, result);
        return result;
    }

    void Ray2D::Normalize()
    {
        Direction = Vector2::Normalize(Direction);
    }

    void Ray2D::Deconstruct(Vector2& origin, Vector2& direction) const
    {
        origin = Origin;
        direction = Direction;
    }

    bool Ray2D::Equals(const Ray2D& other) const
    {
        return Origin == other.Origin && Direction == other.Direction;
    }

    int Ray2D::GetHashCode() const
    {
        return Origin.GetHashCode() ^ Direction.GetHashCode();
    }

    std::string Ray2D::ToString() const
    {
        return "{Origin:" + Origin.ToString() + " Direction:" + Direction.ToString() + "}";
    }
}
