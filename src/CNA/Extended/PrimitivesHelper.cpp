// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/PrimitivesHelper.hpp"

#include "CNA/Extended/MathExtended.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace CNA::Extended
{
    bool PrimitivesHelper::IntersectsSlab(const float positionCoordinate, const float directionCoordinate,
        const float slabMinimum, const float slabMaximum, float& rayMinimumDistance, float& rayMaximumDistance)
    {
        if (std::abs(directionCoordinate) < std::numeric_limits<float>::epsilon())
        {
            return (positionCoordinate >= slabMinimum) && (positionCoordinate <= slabMaximum);
        }

        // Compute intersection values of ray with near and far plane of slab
        float rayNearDistance = (slabMinimum - positionCoordinate) / directionCoordinate;
        float rayFarDistance = (slabMaximum - positionCoordinate) / directionCoordinate;

        if (rayNearDistance > rayFarDistance)
        {
            std::swap(rayNearDistance, rayFarDistance);
        }

        // Compute the intersection of slab intersection intervals
        rayMinimumDistance = rayNearDistance > rayMinimumDistance ? rayNearDistance : rayMinimumDistance;
        rayMaximumDistance = rayFarDistance < rayMaximumDistance ? rayFarDistance : rayMaximumDistance;

        // Exit with no collision as soon as slab intersection becomes empty
        return rayMinimumDistance <= rayMaximumDistance;
    }

    void PrimitivesHelper::CreateRectangleFromPoints(const std::vector<Vector2>& points, Vector2& minimum, Vector2& maximum)
    {
        if (points.empty())
        {
            minimum = Vector2::Zero;
            maximum = Vector2::Zero;
            return;
        }

        minimum = maximum = points[0];

        for (std::size_t index = points.size() - 1; index > 0; --index)
        {
            const Vector2& point = points[index];
            minimum = MathExtended::CalculateMinimumVector2(minimum, point);
            maximum = MathExtended::CalculateMaximumVector2(maximum, point);
        }
    }

    void PrimitivesHelper::TransformRectangle(Vector2& center, Vector2& halfExtents, const Matrix3x2& transformMatrix)
    {
        center = transformMatrix.Transform(center);
        const float xRadius = halfExtents.X;
        const float yRadius = halfExtents.Y;
        halfExtents.X = xRadius * std::abs(transformMatrix.M11) + yRadius * std::abs(transformMatrix.M12);
        halfExtents.Y = xRadius * std::abs(transformMatrix.M21) + yRadius * std::abs(transformMatrix.M22);
    }

    void PrimitivesHelper::TransformOrientedRectangle(Vector2& center, Matrix3x2& orientation, const Matrix3x2& transformMatrix)
    {
        center = transformMatrix.Transform(center);
        orientation = orientation * transformMatrix;
        // Reset the translation since orientation is only about rotation
        orientation.M31 = 0;
        orientation.M32 = 0;
    }

    float PrimitivesHelper::SquaredDistanceToPointFromRectangle(const Vector2& minimum, const Vector2& maximum, const Vector2& point)
    {
        float squaredDistance = 0.0f;

        // for each axis add up the excess distance outside the box

        // x-axis
        if (point.X < minimum.X)
        {
            const float distance = minimum.X - point.X;
            squaredDistance += distance * distance;
        }
        else if (point.X > maximum.X)
        {
            const float distance = maximum.X - point.X;
            squaredDistance += distance * distance;
        }

        // y-axis
        if (point.Y < minimum.Y)
        {
            const float distance = minimum.Y - point.Y;
            squaredDistance += distance * distance;
        }
        else if (point.Y > maximum.Y)
        {
            const float distance = maximum.Y - point.Y;
            squaredDistance += distance * distance;
        }
        return squaredDistance;
    }

    void PrimitivesHelper::ClosestPointToPointFromRectangle(const Vector2& minimum, const Vector2& maximum, const Vector2& point, Vector2& result)
    {
        result = point;

        // For each coordinate axis, if the point coordinate value is outside box, clamp it to the box, else keep it as is
        if (result.X < minimum.X)
        {
            result.X = minimum.X;
        }
        else if (result.X > maximum.X)
        {
            result.X = maximum.X;
        }

        if (result.Y < minimum.Y)
        {
            result.Y = minimum.Y;
        }
        else if (result.Y > maximum.Y)
        {
            result.Y = maximum.Y;
        }
    }
}
