// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Ray2.hpp"

#include "CNA/Extended/BoundingRectangle.hpp"
#include "CNA/Extended/PrimitivesHelper.hpp"

#include <limits>

namespace CNA::Extended
{
    Ray2::Ray2(const Vector2& position, const Vector2& direction) : Position(position), Direction(direction)
    {
    }

    bool Ray2::Intersects(const BoundingRectangle& boundingRectangle, float& rayNearDistance, float& rayFarDistance) const
    {
        // Real-Time Collision Detection, Christer Ericson, 2005. Chapter 5.3; Basic Primitive
        // Tests - Intersecting Lines, Rays, and (Directed Segments). pg 179-181
        const Vector2 minimum = boundingRectangle.Center - boundingRectangle.HalfExtents;
        const Vector2 maximum = boundingRectangle.Center + boundingRectangle.HalfExtents;

        // Set to the smallest possible value so the algorithm can find the first hit along the ray.
        float minimumDistanceAlongRay = std::numeric_limits<float>::lowest();
        // Set to the maximum possible value so the algorithm can find the last hit along the ray.
        float maximumDistanceAlongRay = std::numeric_limits<float>::max();

        // For all relevant slabs, which in this case is two.

        // The first, horizontal, slab.
        if (!PrimitivesHelper::IntersectsSlab(Position.X, Direction.X, minimum.X, maximum.X,
                                               minimumDistanceAlongRay, maximumDistanceAlongRay))
        {
            rayNearDistance = rayFarDistance = std::numeric_limits<float>::quiet_NaN();
            return false;
        }

        // The second, vertical, slab.
        if (!PrimitivesHelper::IntersectsSlab(Position.Y, Direction.Y, minimum.Y, maximum.Y,
                                               minimumDistanceAlongRay, maximumDistanceAlongRay))
        {
            rayNearDistance = rayFarDistance = std::numeric_limits<float>::quiet_NaN();
            return false;
        }

        // Ray intersects the 2 slabs.
        rayNearDistance = minimumDistanceAlongRay < 0.0f ? 0.0f : minimumDistanceAlongRay;
        rayFarDistance = maximumDistanceAlongRay;
        return true;
    }

    bool Ray2::Equals(const Ray2& ray) const
    {
        return (ray.Position == Position) && (ray.Direction == Direction);
    }

    int Ray2::GetHashCode() const
    {
        return static_cast<int>((static_cast<unsigned int>(Position.GetHashCode()) * 397) ^ static_cast<unsigned int>(Direction.GetHashCode()));
    }

    std::string Ray2::ToString() const
    {
        return "Position: " + Position.ToString() + ", Direction: " + Direction.ToString();
    }
}
