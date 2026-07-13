// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collision2D.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::MathHelper;

    // ---- Helpers ----

    bool Collision2D::IsValidPolygon(const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals)
    {
        const std::size_t count = vertices.size();
        return count >= 3 && normals.size() == count;
    }

    bool Collision2D::ClipInterval(float& tMin, float& tMax, const float clipMin, const float clipMax)
    {
        if (clipMin > tMin)
        {
            tMin = clipMin;
        }
        if (clipMax < tMax)
        {
            tMax = clipMax;
        }
        return tMin <= tMax;
    }

    bool Collision2D::ClipInterval(
        const float tEnter, const float tExit, const float tLower, const float tUpper, float& clippedEnter, float& clippedExit)
    {
        clippedEnter = std::max(tEnter, tLower);
        clippedExit = std::min(tExit, tUpper);
        return clippedEnter <= clippedExit;
    }

    bool Collision2D::IntervalsOverlap(const float minA, const float maxA, const float minB, const float maxB)
    {
        // touch counts as overlap
        if (maxA < minB - Epsilon)
        {
            return false;
        }
        if (maxB < minA - Epsilon)
        {
            return false;
        }
        return true;
    }

    bool Collision2D::TryGetProjectionOverlap(const float minA, const float maxA, const float minB, const float maxB, float& overlap)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (interval overlap reasoning for projected axes)
        const float lower = std::max(minA, minB);
        const float upper = std::min(maxA, maxB);
        const float depth = upper - lower;

        if (depth < -Epsilon)
        {
            overlap = 0.0f;
            return false;
        }

        overlap = std::max(0.0f, depth);
        return true;
    }

    bool Collision2D::UpdateMinimumOverlap(const float overlap, const Vector2& axis, float& minimumOverlap, Vector2& minimumOverlapAxis)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (track minimum overlap across tested axes)
        if (overlap >= minimumOverlap)
        {
            return false;
        }

        minimumOverlap = overlap;
        minimumOverlapAxis = axis;
        return true;
    }

    Vector2 Collision2D::OrientNormal(const Vector2& normal, const Vector2& centerA, const Vector2& centerB)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (choose the separating axis direction for response)
        const Vector2 centerDelta = centerA - centerB;
        if (Vector2::Dot(centerDelta, normal) < 0.0f)
        {
            return -normal;
        }

        return normal;
    }

    // ---- Containment (AABB Contains) ----

    ContainmentType Collision2D::ContainsAabbPoint(const Vector2& point, const Vector2& min, const Vector2& max)
    {
        if (point.X < min.X - Epsilon)
        {
            return ContainmentType::Disjoint;
        }
        if (point.X > max.X + Epsilon)
        {
            return ContainmentType::Disjoint;
        }
        if (point.Y < min.Y - Epsilon)
        {
            return ContainmentType::Disjoint;
        }
        if (point.Y > max.Y + Epsilon)
        {
            return ContainmentType::Disjoint;
        }
        return ContainmentType::Contains;
    }

    ContainmentType Collision2D::ContainsAabbAabb(const Vector2& aMin, const Vector2& aMax, const Vector2& bMin, const Vector2& bMax)
    {
        // Disjoint (touch counts as NOT disjoint)
        if (bMax.X < aMin.X - Epsilon || bMin.X > aMax.X + Epsilon || bMax.Y < aMin.Y - Epsilon || bMin.Y > aMax.Y + Epsilon)
        {
            return ContainmentType::Disjoint;
        }

        // Contains (inclusive)
        if (bMin.X >= aMin.X - Epsilon && bMax.X <= aMax.X + Epsilon && bMin.Y >= aMin.Y - Epsilon && bMax.Y <= aMax.Y + Epsilon)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsAabbCircle(const Vector2& boxMin, const Vector2& boxMax, const Vector2& circleCenter, const float circleRadius)
    {
        if (circleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        // Disjoint check: circle completely outside
        if (circleCenter.X - circleRadius > boxMax.X + Epsilon || circleCenter.X + circleRadius < boxMin.X - Epsilon ||
            circleCenter.Y - circleRadius > boxMax.Y + Epsilon || circleCenter.Y + circleRadius < boxMin.Y - Epsilon)
        {
            return ContainmentType::Disjoint;
        }

        // Contains (inclusive)
        if (circleCenter.X - circleRadius >= boxMin.X - Epsilon && circleCenter.X + circleRadius <= boxMax.X + Epsilon &&
            circleCenter.Y - circleRadius >= boxMin.Y - Epsilon && circleCenter.Y + circleRadius <= boxMax.Y + Epsilon)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsAabbObb(const Vector2& aabbMin, const Vector2& aabbMax, const Vector2& obbCenter, const Vector2& obbAxisX,
        const Vector2& obbAxisY, const Vector2& obbHalfExtents)
    {
        const Vector2 aabbCenter = (aabbMin + aabbMax) * 0.5f;
        const Vector2 aabbHalf = (aabbMax - aabbMin) * 0.5f;

        // Disjoint check
        if (!IntersectsAabbObb(aabbCenter, aabbHalf, obbCenter, obbAxisX, obbAxisY, obbHalfExtents))
        {
            return ContainmentType::Disjoint;
        }

        // Contains: all 4 OBB corners inside AABB
        const Vector2 ex = obbAxisX * obbHalfExtents.X;
        const Vector2 ey = obbAxisY * obbHalfExtents.Y;

        const Vector2 c0 = obbCenter - ex - ey;
        const Vector2 c1 = obbCenter + ex - ey;
        const Vector2 c2 = obbCenter + ex + ey;
        const Vector2 c3 = obbCenter - ex + ey;

        if (ContainsAabbPoint(c0, aabbMin, aabbMax) == ContainmentType::Contains && ContainsAabbPoint(c1, aabbMin, aabbMax) == ContainmentType::Contains &&
            ContainsAabbPoint(c2, aabbMin, aabbMax) == ContainmentType::Contains && ContainsAabbPoint(c3, aabbMin, aabbMax) == ContainmentType::Contains)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsAabbCapsule(
        const Vector2& boxMin, const Vector2& boxMax, const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius)
    {
        if (capsuleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        // If they don't even intersect, containment is impossible
        if (!IntersectsAabbCapsule(boxMin, boxMax, capsuleA, capsuleB, capsuleRadius))
        {
            return ContainmentType::Disjoint;
        }

        // For full containment, the capsule's segment must lie inside the AABB contracted (shrunk) by the capsule radius
        const Vector2 innerMin(boxMin.X + capsuleRadius, boxMin.Y + capsuleRadius);
        const Vector2 innerMax(boxMax.X - capsuleRadius, boxMax.Y - capsuleRadius);

        // If the contracted AABB is invalid, it can't contain any capsule with this radius
        if (innerMin.X > innerMax.X + Epsilon || innerMin.Y > innerMax.Y + Epsilon)
        {
            return ContainmentType::Disjoint;
        }

        // Segment inside convex set
        if (ContainsAabbPoint(capsuleA, innerMin, innerMax) == ContainmentType::Contains &&
            ContainsAabbPoint(capsuleB, innerMin, innerMax) == ContainmentType::Contains)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsAabbConvexPolygon(
        const Vector2& boxMin, const Vector2& boxMax, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals)
    {
        if (!IsValidPolygon(vertices, normals))
        {
            return ContainmentType::Disjoint;
        }

        // If they don't intersect at all, containment is impossible
        if (!IntersectsAabbConvexPolygon(boxMin, boxMax, vertices, normals))
        {
            return ContainmentType::Disjoint;
        }

        // Fast containment check: if all polygon vertices are inside the AABB, then the convex polygon is fully contained
        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            if (ContainsAabbPoint(vertices[i], boxMin, boxMax) == ContainmentType::Disjoint)
            {
                return ContainmentType::Intersects;
            }
        }

        return ContainmentType::Contains;
    }

    // ---- Containment (Circle Contains) ----

    ContainmentType Collision2D::ContainsCirclePoint(const Vector2& point, const Vector2& center, const float radius)
    {
        if (radius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        // Expand radius by epsilon to improve tests when point lies near boundary.
        const float r = radius + Epsilon;
        return Vector2::DistanceSquared(point, center) <= r * r ? ContainmentType::Contains : ContainmentType::Disjoint;
    }

    ContainmentType Collision2D::ContainsCircleAabb(const Vector2& circleCenter, const float circleRadius, const Vector2& aabbMin, const Vector2& aabbMax)
    {
        if (circleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        if (!IntersectsCircleAabb(circleCenter, circleRadius, aabbMin, aabbMax))
        {
            return ContainmentType::Disjoint;
        }

        // Contains if all 4 AABB corners are inside circle
        const Vector2 c0(aabbMin.X, aabbMin.Y);
        const Vector2 c1(aabbMax.X, aabbMin.Y);
        const Vector2 c2(aabbMax.X, aabbMax.Y);
        const Vector2 c3(aabbMin.X, aabbMax.Y);

        if (ContainsCirclePoint(c0, circleCenter, circleRadius) == ContainmentType::Contains &&
            ContainsCirclePoint(c1, circleCenter, circleRadius) == ContainmentType::Contains &&
            ContainsCirclePoint(c2, circleCenter, circleRadius) == ContainmentType::Contains &&
            ContainsCirclePoint(c3, circleCenter, circleRadius) == ContainmentType::Contains)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsCircleCircle(const Vector2& aCenter, const float aRadius, const Vector2& bCenter, const float bRadius)
    {
        if (aRadius < 0.0f || bRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        const Vector2 d = bCenter - aCenter;
        const float d2 = d.LengthSquared();

        // Disjoint if centers are farther apart than sum of radii
        const float sum = aRadius + bRadius + Epsilon;
        if (d2 > sum * sum)
        {
            return ContainmentType::Disjoint;
        }

        // Contains if B fits entirely within A (inclusive)
        const float rhs = (aRadius - bRadius) + Epsilon;
        if (rhs >= 0.0f && d2 <= rhs * rhs)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsCircleObb(const Vector2& circleCenter, const float circleRadius, const Vector2& obbCenter,
        const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalfExtents)
    {
        if (circleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        // Disjoint check first (touch counts as NOT disjoint)
        if (!IntersectsCircleObb(circleCenter, circleRadius, obbCenter, obbAxisX, obbAxisY, obbHalfExtents))
        {
            return ContainmentType::Disjoint;
        }

        // Contains if all 4 OBB corners are inside circle
        const Vector2 ex = obbAxisX * obbHalfExtents.X;
        const Vector2 ey = obbAxisY * obbHalfExtents.Y;

        const Vector2 c0 = obbCenter - ex - ey;
        const Vector2 c1 = obbCenter + ex - ey;
        const Vector2 c2 = obbCenter + ex + ey;
        const Vector2 c3 = obbCenter - ex + ey;

        if (ContainsCirclePoint(c0, circleCenter, circleRadius) == ContainmentType::Contains &&
            ContainsCirclePoint(c1, circleCenter, circleRadius) == ContainmentType::Contains &&
            ContainsCirclePoint(c2, circleCenter, circleRadius) == ContainmentType::Contains &&
            ContainsCirclePoint(c3, circleCenter, circleRadius) == ContainmentType::Contains)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsCircleCapsule(
        const Vector2& circleCenter, const float circleRadius, const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius)
    {
        if (circleRadius < 0.0f || capsuleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        if (!IntersectsCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius))
        {
            return ContainmentType::Disjoint;
        }

        const float inner = (circleRadius - capsuleRadius) + Epsilon;
        if (inner < 0.0f)
        {
            return ContainmentType::Intersects;
        }

        const float inner2 = inner * inner;

        // For the capsule to be fully contained, both endpoints must be within the inner circle
        if (Vector2::DistanceSquared(circleCenter, capsuleA) <= inner2 && Vector2::DistanceSquared(circleCenter, capsuleB) <= inner2)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsCircleConvexPolygon(
        const Vector2& circleCenter, const float circleRadius, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals)
    {
        if (circleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        if (!IsValidPolygon(vertices, normals))
        {
            return ContainmentType::Disjoint;
        }

        if (!IntersectsCircleConvexPolygon(circleCenter, circleRadius, vertices, normals))
        {
            return ContainmentType::Disjoint;
        }

        // Contains: all polygon vertices inside circle
        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            if (ContainsCirclePoint(vertices[i], circleCenter, circleRadius) == ContainmentType::Disjoint)
            {
                return ContainmentType::Intersects;
            }
        }

        return ContainmentType::Contains;
    }

    // ---- Containment (OBB Contains) ----

    ContainmentType Collision2D::ContainsObbPoint(
        const Vector2& point, const Vector2& center, const Vector2& axisX, const Vector2& axisY, const Vector2& halfExtents)
    {
        // Transform point into OBB local space by projecting it onto axes
        const Vector2 d = point - center;

        const float distX = Vector2::Dot(d, axisX);
        if (std::abs(distX) > halfExtents.X + Epsilon)
        {
            return ContainmentType::Disjoint;
        }

        const float distY = Vector2::Dot(d, axisY);
        if (std::abs(distY) > halfExtents.Y + Epsilon)
        {
            return ContainmentType::Disjoint;
        }

        return ContainmentType::Contains;
    }

    ContainmentType Collision2D::ContainsObbAabb(const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY,
        const Vector2& obbHalfExtents, const Vector2& aabbMin, const Vector2& aabbMax)
    {
        const Vector2 aabbCenter = (aabbMin + aabbMax) * 0.5f;
        const Vector2 aabbHalf = (aabbMax - aabbMin) * 0.5f;

        // Disjoint check
        if (!IntersectsAabbObb(aabbCenter, aabbHalf, obbCenter, obbAxisX, obbAxisY, obbHalfExtents))
        {
            return ContainmentType::Disjoint;
        }

        // Contains: all 4 AABB corners inside OBB
        const Vector2 c0(aabbMin.X, aabbMin.Y);
        const Vector2 c1(aabbMax.X, aabbMin.Y);
        const Vector2 c2(aabbMax.X, aabbMax.Y);
        const Vector2 c3(aabbMin.X, aabbMax.Y);

        if (ContainsObbPoint(c0, obbCenter, obbAxisX, obbAxisY, obbHalfExtents) == ContainmentType::Contains &&
            ContainsObbPoint(c1, obbCenter, obbAxisX, obbAxisY, obbHalfExtents) == ContainmentType::Contains &&
            ContainsObbPoint(c2, obbCenter, obbAxisX, obbAxisY, obbHalfExtents) == ContainmentType::Contains &&
            ContainsObbPoint(c3, obbCenter, obbAxisX, obbAxisY, obbHalfExtents) == ContainmentType::Contains)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsObbCircle(const Vector2& boxCenter, const Vector2& axisX, const Vector2& axisY, const Vector2& halfExtents,
        const Vector2& circleCenter, const float circleRadius)
    {
        if (circleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        // Disjoint check
        if (!IntersectsCircleObb(circleCenter, circleRadius, boxCenter, axisX, axisY, halfExtents))
        {
            return ContainmentType::Disjoint;
        }

        // Containment: circle must fit within OBB extents in local space
        const Vector2 d = circleCenter - boxCenter;
        const float localX = Vector2::Dot(d, axisX);
        const float localY = Vector2::Dot(d, axisY);

        const float r = circleRadius + Epsilon;

        if (std::abs(localX) + r <= halfExtents.X && std::abs(localY) + r <= halfExtents.Y)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsObbObb(const Vector2& aCenter, const Vector2& aAxisX, const Vector2& aAxisY, const Vector2& aHalf,
        const Vector2& bCenter, const Vector2& bAxisX, const Vector2& bAxisY, const Vector2& bHalf)
    {
        if (!IntersectsObbObb(aCenter, aAxisX, aAxisY, aHalf, bCenter, bAxisX, bAxisY, bHalf))
        {
            return ContainmentType::Disjoint;
        }

        // Contains all 4 B corners inside A
        const Vector2 bx = bAxisX * bHalf.X;
        const Vector2 by = bAxisY * bHalf.Y;

        const Vector2 c0 = bCenter - bx - by;
        const Vector2 c1 = bCenter + bx - by;
        const Vector2 c2 = bCenter + bx + by;
        const Vector2 c3 = bCenter - bx + by;

        if (ContainsObbPoint(c0, aCenter, aAxisX, aAxisY, aHalf) == ContainmentType::Contains &&
            ContainsObbPoint(c1, aCenter, aAxisX, aAxisY, aHalf) == ContainmentType::Contains &&
            ContainsObbPoint(c2, aCenter, aAxisX, aAxisY, aHalf) == ContainmentType::Contains &&
            ContainsObbPoint(c3, aCenter, aAxisX, aAxisY, aHalf) == ContainmentType::Contains)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsObbCapsule(const Vector2& boxCenter, const Vector2& axisX, const Vector2& axisY, const Vector2& halfExtents,
        const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius)
    {
        if (capsuleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        if (!IntersectsObbCapsule(boxCenter, axisX, axisY, halfExtents, capsuleA, capsuleB, capsuleRadius))
        {
            return ContainmentType::Disjoint;
        }

        const Vector2 innerHalf(halfExtents.X - capsuleRadius, halfExtents.Y - capsuleRadius);

        // If the contracted OBB is invalid, it can't contain a capsule of this radius
        if (innerHalf.X < -Epsilon || innerHalf.Y < -Epsilon)
        {
            return ContainmentType::Intersects;
        }

        if (ContainsObbPoint(capsuleA, boxCenter, axisX, axisY, innerHalf) == ContainmentType::Contains &&
            ContainsObbPoint(capsuleB, boxCenter, axisX, axisY, innerHalf) == ContainmentType::Contains)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsObbConvexPolygon(const Vector2& boxCenter, const Vector2& axisX, const Vector2& axisY,
        const Vector2& halfExtents, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals)
    {
        if (!IsValidPolygon(vertices, normals))
        {
            return ContainmentType::Disjoint;
        }

        if (!IntersectsObbConvexPolygon(boxCenter, axisX, axisY, halfExtents, vertices, normals))
        {
            return ContainmentType::Disjoint;
        }

        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            if (ContainsObbPoint(vertices[i], boxCenter, axisX, axisY, halfExtents) == ContainmentType::Disjoint)
            {
                return ContainmentType::Intersects;
            }
        }

        return ContainmentType::Contains;
    }

    // ---- Containment (Capsule Contains) ----

    ContainmentType Collision2D::ContainsCapsulePoint(const Vector2& point, const Vector2& a, const Vector2& b, const float radius)
    {
        if (radius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        const float r = radius + Epsilon;
        const float rr = r * r;

        float t;
        Vector2 closestPoint;
        const float d2 = DistanceSquaredPointSegment(point, a, b, t, closestPoint);
        return d2 <= rr ? ContainmentType::Contains : ContainmentType::Disjoint;
    }

    ContainmentType Collision2D::ContainsCapsuleAabb(
        const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius, const Vector2& aabbMin, const Vector2& aabbMax)
    {
        if (capsuleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        if (!IntersectsAabbCapsule(aabbMin, aabbMax, capsuleA, capsuleB, capsuleRadius))
        {
            return ContainmentType::Disjoint;
        }

        const Vector2 c0(aabbMin.X, aabbMin.Y);
        const Vector2 c1(aabbMax.X, aabbMin.Y);
        const Vector2 c2(aabbMax.X, aabbMax.Y);
        const Vector2 c3(aabbMin.X, aabbMax.Y);

        if (ContainsCapsulePoint(c0, capsuleA, capsuleB, capsuleRadius) == ContainmentType::Contains &&
            ContainsCapsulePoint(c1, capsuleA, capsuleB, capsuleRadius) == ContainmentType::Contains &&
            ContainsCapsulePoint(c2, capsuleA, capsuleB, capsuleRadius) == ContainmentType::Contains &&
            ContainsCapsulePoint(c3, capsuleA, capsuleB, capsuleRadius) == ContainmentType::Contains)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsCapsuleCircle(
        const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius, const Vector2& circleCenter, const float circleRadius)
    {
        if (capsuleRadius < 0.0f || circleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        // Disjoint check first (touch counts as NOT disjoint)
        if (!IntersectsCircleCapsule(circleCenter, circleRadius, capsuleA, capsuleB, capsuleRadius))
        {
            return ContainmentType::Disjoint;
        }

        // Contains if the circle fits fully inside the capsule
        const float inner = (capsuleRadius - circleRadius) + Epsilon;
        if (inner < 0.0f)
        {
            return ContainmentType::Intersects;
        }

        const float inner2 = inner * inner;
        float t;
        Vector2 closestPoint;
        const float d2 = DistanceSquaredPointSegment(circleCenter, capsuleA, capsuleB, t, closestPoint);

        return d2 <= inner2 ? ContainmentType::Contains : ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsCapsuleObb(const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius,
        const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalf)
    {
        if (capsuleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        if (!IntersectsObbCapsule(obbCenter, obbAxisX, obbAxisY, obbHalf, capsuleA, capsuleB, capsuleRadius))
        {
            return ContainmentType::Disjoint;
        }

        const Vector2 ex = obbAxisX * obbHalf.X;
        const Vector2 ey = obbAxisY * obbHalf.Y;

        const Vector2 c0 = obbCenter - ex - ey;
        const Vector2 c1 = obbCenter + ex - ey;
        const Vector2 c2 = obbCenter + ex + ey;
        const Vector2 c3 = obbCenter - ex + ey;

        if (ContainsCapsulePoint(c0, capsuleA, capsuleB, capsuleRadius) == ContainmentType::Contains &&
            ContainsCapsulePoint(c1, capsuleA, capsuleB, capsuleRadius) == ContainmentType::Contains &&
            ContainsCapsulePoint(c2, capsuleA, capsuleB, capsuleRadius) == ContainmentType::Contains &&
            ContainsCapsulePoint(c3, capsuleA, capsuleB, capsuleRadius) == ContainmentType::Contains)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsCapsuleCapsule(
        const Vector2& a0, const Vector2& a1, const float aRadius, const Vector2& b0, const Vector2& b1, const float bRadius)
    {
        if (aRadius < 0.0f || bRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        if (!IntersectsCapsuleCapsule(a0, a1, aRadius, b0, b1, bRadius))
        {
            return ContainmentType::Disjoint;
        }

        // If B is larger, A can't contain it, but they do intersect
        const float inner = (aRadius - bRadius) + Epsilon;
        if (inner < 0.0f)
        {
            return ContainmentType::Intersects;
        }

        const float inner2 = inner * inner;

        float t;
        Vector2 closestPoint;
        const float d20 = DistanceSquaredPointSegment(b0, a0, a1, t, closestPoint);
        if (d20 > inner2)
        {
            return ContainmentType::Intersects;
        }

        const float d21 = DistanceSquaredPointSegment(b1, a0, a1, t, closestPoint);
        if (d21 > inner2)
        {
            return ContainmentType::Intersects;
        }

        return ContainmentType::Contains;
    }

    ContainmentType Collision2D::ContainsCapsuleConvexPolygon(const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius,
        const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals)
    {
        if (capsuleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        if (!IsValidPolygon(pVertices, pNormals))
        {
            return ContainmentType::Disjoint;
        }

        if (!IntersectsCapsuleConvexPolygon(capsuleA, capsuleB, capsuleRadius, pVertices, pNormals))
        {
            return ContainmentType::Disjoint;
        }

        for (std::size_t i = 0; i < pVertices.size(); i++)
        {
            if (ContainsCapsulePoint(pVertices[i], capsuleA, capsuleB, capsuleRadius) == ContainmentType::Disjoint)
            {
                return ContainmentType::Intersects;
            }
        }

        return ContainmentType::Contains;
    }

    // ---- Containment (Convex Polygon Contains) ----

    ContainmentType Collision2D::ContainsConvexPolygonPoint(const Vector2& point, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.4.1 "Testing Point in Polygon" (half-space tests; adapted to convex polygon with outward normals)

        if (!IsValidPolygon(vertices, normals))
        {
            return ContainmentType::Disjoint;
        }

        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            const Vector2& n = normals[i];
            const float planeD = Vector2::Dot(n, vertices[i]);

            // Inside halfspace if Dot(n, p) <= planeD
            if (Vector2::Dot(n, point) > planeD + Epsilon)
            {
                return ContainmentType::Disjoint;
            }
        }

        return ContainmentType::Contains;
    }

    ContainmentType Collision2D::ContainsConvexPolygonAabb(
        const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals, const Vector2& aabbMin, const Vector2& aabbMax)
    {
        if (!IsValidPolygon(pVertices, pNormals))
        {
            return ContainmentType::Disjoint;
        }

        const Vector2 aabbCenter = (aabbMin + aabbMax) * 0.5f;
        const Vector2 aabbHalf = (aabbMax - aabbMin) * 0.5f;

        if (!IntersectsAabbConvexPolygon(aabbCenter, aabbHalf, pVertices, pNormals))
        {
            return ContainmentType::Disjoint;
        }

        // Contains: all AABB corners inside polygon
        const Vector2 c0(aabbMin.X, aabbMin.Y);
        const Vector2 c1(aabbMax.X, aabbMin.Y);
        const Vector2 c2(aabbMax.X, aabbMax.Y);
        const Vector2 c3(aabbMin.X, aabbMax.Y);

        if (ContainsConvexPolygonPoint(c0, pVertices, pNormals) == ContainmentType::Contains &&
            ContainsConvexPolygonPoint(c1, pVertices, pNormals) == ContainmentType::Contains &&
            ContainsConvexPolygonPoint(c2, pVertices, pNormals) == ContainmentType::Contains &&
            ContainsConvexPolygonPoint(c3, pVertices, pNormals) == ContainmentType::Contains)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsConvexPolygonCircle(
        const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals, const Vector2& circleCenter, const float circleRadius)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.2 "Testing Sphere Against Plane" (2D reduction: circle vs line)
        // Related: Section 5.2.8 "Testing Sphere Against Polygon" (apply plane test to all polygon edge half-spaces)

        if (circleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        if (!IsValidPolygon(pVertices, pNormals))
        {
            return ContainmentType::Disjoint;
        }

        // If they don't intersect at all, containment is impossible
        if (!IntersectsCircleConvexPolygon(circleCenter, circleRadius, pVertices, pNormals))
        {
            return ContainmentType::Disjoint;
        }

        // Contains if the circle is fully inside every polygon halfspace:
        // Dot(n, center) + radius <= planeD
        //
        // NOTE: this assumes normals are outward facing unit normals
        const float r = circleRadius + Epsilon;

        for (std::size_t i = 0; i < pVertices.size(); i++)
        {
            const Vector2& n = pNormals[i];
            const float planeD = Vector2::Dot(n, pVertices[i]);

            if (Vector2::Dot(n, circleCenter) + r > planeD)
            {
                return ContainmentType::Intersects;
            }
        }

        return ContainmentType::Contains;
    }

    ContainmentType Collision2D::ContainsConvexPolygonCapsule(const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals,
        const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.2 "Testing Sphere Against Plane" (2D reduction: circle vs line)

        if (capsuleRadius < 0.0f)
        {
            return ContainmentType::Disjoint;
        }

        if (!IsValidPolygon(pVertices, pNormals))
        {
            return ContainmentType::Disjoint;
        }

        if (!IntersectsCapsuleConvexPolygon(capsuleA, capsuleB, capsuleRadius, pVertices, pNormals))
        {
            return ContainmentType::Disjoint;
        }

        const float r = capsuleRadius + Epsilon;

        for (std::size_t i = 0; i < pVertices.size(); i++)
        {
            const Vector2& n = pNormals[i];
            const float planeD = Vector2::Dot(n, pVertices[i]);

            const float da = Vector2::Dot(n, capsuleA);
            const float db = Vector2::Dot(n, capsuleB);
            const float maxProjection = da > db ? da : db;

            if (maxProjection + r > planeD)
            {
                return ContainmentType::Intersects;
            }
        }

        return ContainmentType::Contains;
    }

    ContainmentType Collision2D::ContainsConvexPolygonObb(const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals,
        const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalf)
    {
        if (!IsValidPolygon(pVertices, pNormals))
        {
            return ContainmentType::Disjoint;
        }

        if (!IntersectsObbConvexPolygon(obbCenter, obbAxisX, obbAxisY, obbHalf, pVertices, pNormals))
        {
            return ContainmentType::Disjoint;
        }

        // Contains: all 4 OBB corners inside polygon
        const Vector2 ex = obbAxisX * obbHalf.X;
        const Vector2 ey = obbAxisY * obbHalf.Y;

        const Vector2 c0 = obbCenter - ex - ey;
        const Vector2 c1 = obbCenter + ex - ey;
        const Vector2 c2 = obbCenter + ex + ey;
        const Vector2 c3 = obbCenter - ex + ey;

        if (ContainsConvexPolygonPoint(c0, pVertices, pNormals) == ContainmentType::Contains &&
            ContainsConvexPolygonPoint(c1, pVertices, pNormals) == ContainmentType::Contains &&
            ContainsConvexPolygonPoint(c2, pVertices, pNormals) == ContainmentType::Contains &&
            ContainsConvexPolygonPoint(c3, pVertices, pNormals) == ContainmentType::Contains)
        {
            return ContainmentType::Contains;
        }

        return ContainmentType::Intersects;
    }

    ContainmentType Collision2D::ContainsConvexPolygonConvexPolygon(const std::vector<Vector2>& aVertices, const std::vector<Vector2>& aNormals,
        const std::vector<Vector2>& bVertices, const std::vector<Vector2>& bNormals)
    {
        if (!IsValidPolygon(aVertices, aNormals) || !IsValidPolygon(bVertices, bNormals))
        {
            return ContainmentType::Disjoint;
        }

        // If they don't intersect at all, containment is impossible
        if (!IntersectsConvexPolygonConvexPolygon(aVertices, aNormals, bVertices, bNormals))
        {
            return ContainmentType::Disjoint;
        }

        // A contains B if all of B's vertices are inside A
        for (std::size_t i = 0; i < bVertices.size(); i++)
        {
            if (ContainsConvexPolygonPoint(bVertices[i], aVertices, aNormals) == ContainmentType::Disjoint)
            {
                return ContainmentType::Intersects;
            }
        }

        return ContainmentType::Contains;
    }

    // ---- Projection ----

    void Collision2D::ProjectOntoAxis(const std::vector<Vector2>& vertices, const Vector2& axis, float& min, float& max)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (2D adaptation)

        min = std::numeric_limits<float>::max();
        max = std::numeric_limits<float>::lowest();

        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            const float p = Vector2::Dot(vertices[i], axis);
            if (p < min)
            {
                min = p;
            }
            if (p > max)
            {
                max = p;
            }
        }
    }

    void Collision2D::ProjectAabbOntoAxis(const Vector2& center, const Vector2& halfExtents, const Vector2& axis, float& min, float& max)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (2D adaptation)

        // Axis does not need to be normalized as long as both shapes use the same axis
        const float c = Vector2::Dot(center, axis);
        const float r = halfExtents.X * std::abs(axis.X) + halfExtents.Y * std::abs(axis.Y);

        min = c - r;
        max = c + r;
    }

    void Collision2D::ProjectObbOntoAxis(
        const Vector2& center, const Vector2& axisX, const Vector2& axisY, const Vector2& halfExtents, const Vector2& axis, float& min, float& max)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (2D adaptation)

        const float c = Vector2::Dot(center, axis);

        // Radius is sum of projected half-extents onto the axis
        const float r = halfExtents.X * std::abs(Vector2::Dot(axisX, axis)) + halfExtents.Y * std::abs(Vector2::Dot(axisY, axis));

        min = c - r;
        max = c + r;
    }

    bool Collision2D::OverlapOnAxis(const std::vector<Vector2>& aVerts, const std::vector<Vector2>& bVerts, const Vector2& axis)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (2D adaptation)

        float minA;
        float maxA;
        float minB;
        float maxB;
        ProjectOntoAxis(aVerts, axis, minA, maxA);
        ProjectOntoAxis(bVerts, axis, minB, maxB);
        return IntervalsOverlap(minA, maxA, minB, maxB);
    }

    bool Collision2D::OverlapOnAxis(const Vector2& aabbCenter, const Vector2& aabbHalfExtents, const std::vector<Vector2>& polygonVertices, const Vector2& axis)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (2D adaptation)

        float minA;
        float maxA;
        float minB;
        float maxB;
        ProjectAabbOntoAxis(aabbCenter, aabbHalfExtents, axis, minA, maxA);
        ProjectOntoAxis(polygonVertices, axis, minB, maxB);
        return IntervalsOverlap(minA, maxA, minB, maxB);
    }

    // ---- Distance / Closest Point ----

    float Collision2D::DistanceSquaredPointSegment(const Vector2& point, const Vector2& a, const Vector2& b, float& t, Vector2& closestPoint)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.1.2 "Closest Point on Line Segment to Point"

        const Vector2 ab = b - a;
        const float denom = Vector2::Dot(ab, ab);

        // Check if segment is degenerate
        if (denom <= EpsilonSq)
        {
            // Segment is degenerate, treat it as a point
            t = 0.0f;
            closestPoint = a;
            const Vector2 d = point - a;
            return d.X * d.X + d.Y * d.Y;
        }

        t = Vector2::Dot(point - a, ab) / denom;
        t = MathHelper::Clamp(t, 0.0f, 1.0f);
        closestPoint = a + ab * t;

        const Vector2 diff = point - closestPoint;
        return diff.LengthSquared();
    }

    float Collision2D::DistanceSquaredSegmentSegment(
        const Vector2& p1, const Vector2& q1, const Vector2& p2, const Vector2& q2, float& s, float& t, Vector2& c1, Vector2& c2)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Derived from Section 5.1.9 "Closest Points of Two Line Segments" (ray/segment domain adaptation)

        const Vector2 d1 = q1 - p1; // Direction vector of segment S1
        const Vector2 d2 = q2 - p2; // Direction vector of segment S2
        const Vector2 r = p1 - p2;

        const float a = Vector2::Dot(d1, d1); // Squared length of segment S1
        const float e = Vector2::Dot(d2, d2); // Squared length of segment S2
        const float f = Vector2::Dot(d2, r);

        // Default outputs
        s = 0.0f;
        t = 0.0f;

        // Check if either or both segments degenerate into points
        if (a <= Epsilon && e <= Epsilon)
        {
            c1 = p1;
            c2 = p2;
            const Vector2 d = c1 - c2;
            return d.LengthSquared();
        }

        if (a <= Epsilon)
        {
            // First segment degenerates into a point
            s = 0.0f;
            t = f / e;
            t = MathHelper::Clamp(t, 0.0f, 1.0f);
        }
        else
        {
            const float c = Vector2::Dot(d1, r);

            if (e <= Epsilon)
            {
                // Second segment degenerates into a point
                t = 0.0f;
                s = MathHelper::Clamp(-c / a, 0.0f, 1.0f);
            }
            else
            {
                // General nondegenerate case starts here
                const float b = Vector2::Dot(d1, d2);
                const float denom = a * e - b * b; // Always nonnegative

                // If segments not parallel, compute closest point on L1, to L2 and clamp to segment s! Else pick arbitrary s (here 0)
                if (std::abs(denom) > Epsilon)
                {
                    s = MathHelper::Clamp((b * f - c * e) / denom, 0.0f, 1.0f);
                }
                else
                {
                    s = 0.0f;
                }

                // Compute point on L2 closest to S1(s) using t = Dot((P1 + D1*s) - P2,D2) / Dot(D2,D2) = (b*s + f) / e
                t = (b * s + f) / e;

                // If t in [0,1] do nothing. Else clamp t, recompute s for the new value of t using
                // s = Dot((P2 + D2*t) - P1,D1) / Dot(D1,D1) = (t*b - c) / a and clamp s to [0,1]
                if (t < 0.0f)
                {
                    t = 0.0f;
                    s = MathHelper::Clamp(-c / a, 0.0f, 1.0f);
                }
                else if (t > 1.0f)
                {
                    t = 1.0f;
                    s = MathHelper::Clamp((b - c) / a, 0.0f, 1.0f);
                }
            }
        }

        c1 = p1 + d1 * s;
        c2 = p2 + d2 * t;
        const Vector2 diff = c1 - c2;
        return diff.LengthSquared();
    }

    float Collision2D::DistanceSquaredPointAabb(const Vector2& point, const Vector2& min, const Vector2& max)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.1.3 "Closest Point on AABB to Point"

        const float cx = MathHelper::Clamp(point.X, min.X, max.X);
        const float cy = MathHelper::Clamp(point.Y, min.Y, max.Y);

        const float dx = point.X - cx;
        const float dy = point.Y - cy;

        return dx * dx + dy * dy;
    }

    float Collision2D::DistanceSquaredPointObb(
        const Vector2& point, const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalfExtents)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.1.4 "Closest Point on OBB to Point"

        const Vector2 d = point - obbCenter;

        // Point in local space
        const float x = Vector2::Dot(d, obbAxisX);
        const float y = Vector2::Dot(d, obbAxisY);

        const float cx = MathHelper::Clamp(x, -obbHalfExtents.X, obbHalfExtents.X);
        const float cy = MathHelper::Clamp(y, -obbHalfExtents.Y, obbHalfExtents.Y);

        const float dx = x - cx;
        const float dy = y - cy;

        return dx * dx + dy * dy;
    }

    float Collision2D::DistanceSquaredPointConvexPolygon(const Vector2& p, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Derived from:
        //  - Section 5.2.1 "Separating-axis Test" (convex point containment)
        //  - Section 5.1.2 "Closest Point on Line Segment to Point"
        // 2D reduction of convex polyhedron distance computation

        if (ContainsConvexPolygonPoint(p, vertices, normals) == ContainmentType::Contains)
        {
            return 0.0f;
        }

        float best = std::numeric_limits<float>::max();

        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            const std::size_t j = (i + 1) % vertices.size();
            float t;
            Vector2 closestPoint;
            const float d2 = DistanceSquaredPointSegment(p, vertices[i], vertices[j], t, closestPoint);
            if (d2 < best)
            {
                best = d2;
            }
        }

        return best;
    }

    float Collision2D::DistanceSquaredSegmentAabb(const Vector2& a, const Vector2& b, const Vector2& min, const Vector2& max)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Derived from:
        //  - Section 5.1.3 "Closest Point on AABB to Point" (endpoint distance)
        //  - Section 5.1.9 "Closest Points of Two Line Segments" (segment-edge distance)
        //  - Parametric clipping against convex regions (segment/AABB intersection test; 2D reduction)

        const Vector2 d = b - a;
        const float dd = Vector2::Dot(d, d);

        // Check for degenerate segment
        if (dd < EpsilonSq)
        {
            // Segment is degenerate, treat as point
            return DistanceSquaredPointAabb(a, min, max);
        }

        // If the segment intersects the AABB, distance is zero.
        float tEnterUnused;
        float tExitUnused;
        if (ClipLineToAabb(a, d, min, max, 0.0f, 1.0f, tEnterUnused, tExitUnused))
        {
            return 0.0f;
        }

        // Otherwise, minimum of: endpoints to AABB, segment to each AABB edge segment
        float best = DistanceSquaredPointAabb(a, min, max);
        const float db = DistanceSquaredPointAabb(b, min, max);
        if (db < best)
        {
            best = db;
        }

        // AABB corners
        const Vector2 c0(min.X, min.Y);
        const Vector2 c1(max.X, min.Y);
        const Vector2 c2(max.X, max.Y);
        const Vector2 c3(min.X, max.Y);

        // Distance segment-to-edge
        float s;
        float t;
        Vector2 seg1;
        Vector2 seg2;
        float dist;

        dist = DistanceSquaredSegmentSegment(a, b, c0, c1, s, t, seg1, seg2);
        if (dist < best)
        {
            best = dist;
        }

        dist = DistanceSquaredSegmentSegment(a, b, c1, c2, s, t, seg1, seg2);
        if (dist < best)
        {
            best = dist;
        }

        dist = DistanceSquaredSegmentSegment(a, b, c2, c3, s, t, seg1, seg2);
        if (dist < best)
        {
            best = dist;
        }

        dist = DistanceSquaredSegmentSegment(a, b, c3, c0, s, t, seg1, seg2);
        if (dist < best)
        {
            best = dist;
        }

        return best;
    }

    float Collision2D::DistanceSquaredSegmentConvexPolygon(const Vector2& a, const Vector2& b, const std::vector<Vector2>& vertices, const std::vector<Vector2>& normals)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Derived from:
        //  - Section 5.2.1 "Separating Axis Test" (convex polygon half-space representation and clipping)
        //  - Section 5.1.9 "Closest Points of Two Line Segments" (segment-edge distance)
        //  - Section 5.1.2 "Closest Point on Line Segment to Point" (degenerate segment handling)

        if (!IsValidPolygon(vertices, normals))
        {
            return std::numeric_limits<float>::max();
        }

        const Vector2 d = b - a;
        const float dd = Vector2::Dot(d, d);

        // Check for degenerate segment
        if (dd <= EpsilonSq)
        {
            // Treat segment as point
            return DistanceSquaredPointConvexPolygon(a, vertices, normals);
        }

        // If segment intersects polygon, distance is zero
        float tEnterUnused;
        float tExitUnused;
        if (ClipLineToConvexPolygon(a, d, vertices, normals, 0.0f, 1.0f, tEnterUnused, tExitUnused))
        {
            return 0.0f;
        }

        // Otherwise min distance to edges
        float best = std::numeric_limits<float>::max();
        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            const std::size_t j = (i + 1) % vertices.size();
            float s;
            float t;
            Vector2 seg1;
            Vector2 seg2;
            const float d2 = DistanceSquaredSegmentSegment(a, b, vertices[i], vertices[j], s, t, seg1, seg2);
            if (d2 < best)
            {
                best = d2;
            }
        }

        return best;
    }

    // ---- Clipping (subset needed by the Distance region) ----

    bool Collision2D::ClipLineToAabb(const Vector2& origin, const Vector2& direction, const Vector2& min, const Vector2& max, const float tLower,
        const float tUpper, float& tEnter, float& tExit)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.3.3 "Intersecting Ray or Segment Against Box" (slab method / parametric interval clipping).
        // Note: equivalent in form to Liang-Barsky style parametric line clipping against axis-aligned boundaries.

        float tMin = tLower;
        float tMax = tUpper;

        // X Slab
        if (std::abs(direction.X) < Epsilon)
        {
            // parallel to X planes; must be within slab
            if (origin.X < min.X - Epsilon || origin.X > max.X + Epsilon)
            {
                tEnter = tExit = 0.0f;
                return false;
            }
        }
        else
        {
            const float inv = 1.0f / direction.X;
            float t1 = (min.X - origin.X) * inv;
            float t2 = (max.X - origin.X) * inv;
            if (t1 > t2)
            {
                std::swap(t1, t2);
            }

            if (!ClipInterval(tMin, tMax, t1, t2))
            {
                tEnter = tExit = 0.0f;
                return false;
            }
        }

        // Y slab
        if (std::abs(direction.Y) < Epsilon)
        {
            // Parallel to Y planes, must be within slab
            if (origin.Y < min.Y - Epsilon || origin.Y > max.Y + Epsilon)
            {
                tEnter = tExit = 0.0f;
                return false;
            }
        }
        else
        {
            const float inv = 1.0f / direction.Y;
            float t1 = (min.Y - origin.Y) * inv;
            float t2 = (max.Y - origin.Y) * inv;
            if (t1 > t2)
            {
                std::swap(t1, t2);
            }

            if (!ClipInterval(tMin, tMax, t1, t2))
            {
                tEnter = tExit = 0.0f;
                return false;
            }
        }

        tEnter = tMin;
        tExit = tMax;
        return true;
    }

    bool Collision2D::ClipLineToConvexPolygon(const Vector2& origin, const Vector2& direction, const std::vector<Vector2>& vertices,
        const std::vector<Vector2>& normals, const float tLower, const float tUpper, float& tEnter, float& tExit)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Derived from Section 5.3.8 "Intersecting Ray or Segment Against Convex Polyhedron" (2D reduction).
        // Also known as Cyrus-Beck clipping (parametric line clipping against a convex polygon via half-space constraints).
        // Half-space: dot(n, X) <= dot(n, v); Parametric: X(t) = origin + t * direction

        if (!IsValidPolygon(vertices, normals))
        {
            tEnter = tExit = 0.0f;
            return false;
        }

        float tMin = tLower;
        float tMax = tUpper;

        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            const Vector2& n = normals[i];
            const Vector2& v = vertices[i];

            // Half space: Dot(n, X) <= Dot(n, v)
            const float planeD = Vector2::Dot(n, v);
            const float dist = planeD - Vector2::Dot(n, origin);
            const float denom = Vector2::Dot(n, direction);

            // Parallel to plane
            if (std::abs(denom) < Epsilon)
            {
                // If outside (dist < 0) no intersection with the convex region
                if (dist < -Epsilon)
                {
                    tEnter = tExit = 0.0f;
                    return false;
                }

                // Otherwise, line is inside/on this half-space; no constraint from edge
                continue;
            }

            const float t = dist / denom;

            // With outward normals: denom > 0 => exiting the inside half-space; denom < 0 => entering
            if (denom > 0.0f)
            {
                if (t < tMax)
                {
                    tMax = t;
                }
            }
            else
            {
                if (t > tMin)
                {
                    tMin = t;
                }
            }

            if (tMin > tMax)
            {
                tEnter = tExit = 0.0f;
                return false;
            }
        }

        tEnter = tMin;
        tExit = tMax;
        return true;
    }

    // ---- Intersections (bool-only overloads) ----

    bool Collision2D::IntersectsAabbAabb(const Vector2& aMin, const Vector2& aMax, const Vector2& bMin, const Vector2& bMax)
    {
        // Exit with no intersection if separated along any axis
        if (aMax.X < bMin.X || aMin.X > bMax.X)
        {
            return false;
        }
        if (aMax.Y < bMin.Y || aMin.Y > bMax.Y)
        {
            return false;
        }

        return true;
    }

    bool Collision2D::IntersectsAabbCapsule(const Vector2& boxMin, const Vector2& boxMax, const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius)
    {
        const float radiusSq = capsuleRadius * capsuleRadius;
        const float distSq = DistanceSquaredSegmentAabb(capsuleA, capsuleB, boxMin, boxMax);
        return distSq <= radiusSq;
    }

    bool Collision2D::IntersectsAabbConvexPolygon(
        const Vector2& aabbCenter, const Vector2& aabbHalfExtents, const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (2D adaptation)

        if (!IsValidPolygon(pVertices, pNormals))
        {
            return false;
        }

        // Test polygon edge normals
        for (std::size_t i = 0; i < pNormals.size(); i++)
        {
            if (!OverlapOnAxis(aabbCenter, aabbHalfExtents, pVertices, pNormals[i]))
            {
                return false;
            }
        }

        // Test AABB axes
        if (!OverlapOnAxis(aabbCenter, aabbHalfExtents, pVertices, Vector2::UnitX))
        {
            return false;
        }

        if (!OverlapOnAxis(aabbCenter, aabbHalfExtents, pVertices, Vector2::UnitY))
        {
            return false;
        }

        return true;
    }

    bool Collision2D::IntersectsAabbObb(const Vector2& aabbCenter, const Vector2& aabbHalfExtents, const Vector2& obbCenter, const Vector2& obbAxisX,
        const Vector2& obbAxisY, const Vector2& obbHalfExtents)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (2D adaptation)

        float minA;
        float maxA;
        float minB;
        float maxB;

        // AABB axis x
        ProjectAabbOntoAxis(aabbCenter, aabbHalfExtents, Vector2::UnitX, minA, maxA);
        ProjectObbOntoAxis(obbCenter, obbAxisX, obbAxisY, obbHalfExtents, Vector2::UnitX, minB, maxB);
        if (!IntervalsOverlap(minA, maxA, minB, maxB))
        {
            return false;
        }

        // AABB axis y
        ProjectAabbOntoAxis(aabbCenter, aabbHalfExtents, Vector2::UnitY, minA, maxA);
        ProjectObbOntoAxis(obbCenter, obbAxisX, obbAxisY, obbHalfExtents, Vector2::UnitY, minB, maxB);
        if (!IntervalsOverlap(minA, maxA, minB, maxB))
        {
            return false;
        }

        // OBB axis X
        ProjectAabbOntoAxis(aabbCenter, aabbHalfExtents, obbAxisX, minA, maxA);
        ProjectObbOntoAxis(obbCenter, obbAxisX, obbAxisY, obbHalfExtents, obbAxisX, minB, maxB);
        if (!IntervalsOverlap(minA, maxA, minB, maxB))
        {
            return false;
        }

        // OBB axis Y
        ProjectAabbOntoAxis(aabbCenter, aabbHalfExtents, obbAxisY, minA, maxA);
        ProjectObbOntoAxis(obbCenter, obbAxisX, obbAxisY, obbHalfExtents, obbAxisY, minB, maxB);
        if (!IntervalsOverlap(minA, maxA, minB, maxB))
        {
            return false;
        }

        return true;
    }

    bool Collision2D::IntersectsObbObb(const Vector2& aCenter, const Vector2& aAxisX, const Vector2& aAxisY, const Vector2& aHalf, const Vector2& bCenter,
        const Vector2& bAxisX, const Vector2& bAxisY, const Vector2& bHalf)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (2D adaptation)

        float minA;
        float maxA;
        float minB;
        float maxB;

        // A axis X
        ProjectObbOntoAxis(aCenter, aAxisX, aAxisY, aHalf, aAxisX, minA, maxA);
        ProjectObbOntoAxis(bCenter, bAxisX, bAxisY, bHalf, aAxisX, minB, maxB);
        if (!IntervalsOverlap(minA, maxA, minB, maxB))
        {
            return false;
        }

        // A axis Y
        ProjectObbOntoAxis(aCenter, aAxisX, aAxisY, aHalf, aAxisY, minA, maxA);
        ProjectObbOntoAxis(bCenter, bAxisX, bAxisY, bHalf, aAxisY, minB, maxB);
        if (!IntervalsOverlap(minA, maxA, minB, maxB))
        {
            return false;
        }

        // B axis X
        ProjectObbOntoAxis(aCenter, aAxisX, aAxisY, aHalf, bAxisX, minA, maxA);
        ProjectObbOntoAxis(bCenter, bAxisX, bAxisY, bHalf, bAxisX, minB, maxB);
        if (!IntervalsOverlap(minA, maxA, minB, maxB))
        {
            return false;
        }

        // B axis Y
        ProjectObbOntoAxis(aCenter, aAxisX, aAxisY, aHalf, bAxisY, minA, maxA);
        ProjectObbOntoAxis(bCenter, bAxisX, bAxisY, bHalf, bAxisY, minB, maxB);
        if (!IntervalsOverlap(minA, maxA, minB, maxB))
        {
            return false;
        }

        return true;
    }

    bool Collision2D::IntersectsObbCapsule(const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalfExtents,
        const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius)
    {
        const float radiusSq = capsuleRadius * capsuleRadius;

        // Transform capsule segment into OBB local space (OBB becomes AABB)
        const Vector2 a = capsuleA - obbCenter;
        const Vector2 b = capsuleB - obbCenter;

        const Vector2 localA(Vector2::Dot(a, obbAxisX), Vector2::Dot(a, obbAxisY));
        const Vector2 localB(Vector2::Dot(b, obbAxisX), Vector2::Dot(b, obbAxisY));

        const Vector2 min = -obbHalfExtents;
        const Vector2 max = obbHalfExtents;

        const float distSq = DistanceSquaredSegmentAabb(localA, localB, min, max);
        return distSq <= radiusSq;
    }

    bool Collision2D::IntersectsObbConvexPolygon(const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalfExtents,
        const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (2D adaptation)

        if (!IsValidPolygon(pVertices, pNormals))
        {
            return false;
        }

        float minP;
        float maxP;
        float minB;
        float maxB;

        // Test polygon edge normals
        for (std::size_t i = 0; i < pVertices.size(); i++)
        {
            const Vector2& axis = pNormals[i];

            ProjectOntoAxis(pVertices, axis, minP, maxP);
            ProjectObbOntoAxis(obbCenter, obbAxisX, obbAxisY, obbHalfExtents, axis, minB, maxB);
            if (!IntervalsOverlap(minP, maxP, minB, maxB))
            {
                return false;
            }
        }

        // OBB X Axis
        ProjectOntoAxis(pVertices, obbAxisX, minP, maxP);
        ProjectObbOntoAxis(obbCenter, obbAxisX, obbAxisY, obbHalfExtents, obbAxisX, minB, maxB);
        if (!IntervalsOverlap(minP, maxP, minB, maxB))
        {
            return false;
        }

        // OBB Y Axis
        ProjectOntoAxis(pVertices, obbAxisY, minP, maxP);
        ProjectObbOntoAxis(obbCenter, obbAxisX, obbAxisY, obbHalfExtents, obbAxisY, minB, maxB);
        if (!IntervalsOverlap(minP, maxP, minB, maxB))
        {
            return false;
        }

        return true;
    }

    bool Collision2D::IntersectsCapsuleConvexPolygon(
        const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius, const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals)
    {
        const float radiusSq = capsuleRadius * capsuleRadius;
        const float d2 = DistanceSquaredSegmentConvexPolygon(capsuleA, capsuleB, pVertices, pNormals);
        return d2 <= radiusSq;
    }

    bool Collision2D::IntersectsCircleCircle(const Vector2& aCenter, const float aRadius, const Vector2& bCenter, const float bRadius)
    {
        const float r = aRadius + bRadius;
        const float sqDist = Vector2::DistanceSquared(aCenter, bCenter);
        return sqDist <= r * r;
    }

    bool Collision2D::IntersectsCircleAabb(const Vector2& cCenter, const float cRadius, const Vector2& boxMin, const Vector2& boxMax)
    {
        const float d2 = DistanceSquaredPointAabb(cCenter, boxMin, boxMax);
        return d2 <= cRadius * cRadius;
    }

    bool Collision2D::IntersectsCircleObb(
        const Vector2& cCenter, const float cRadius, const Vector2& obbCenter, const Vector2& obbAxisX, const Vector2& obbAxisY, const Vector2& obbHalfExtents)
    {
        const float d2 = DistanceSquaredPointObb(cCenter, obbCenter, obbAxisX, obbAxisY, obbHalfExtents);
        return d2 <= cRadius * cRadius;
    }

    bool Collision2D::IntersectsCircleConvexPolygon(
        const Vector2& cCenter, const float cRadius, const std::vector<Vector2>& pVertices, const std::vector<Vector2>& pNormals)
    {
        if (!IsValidPolygon(pVertices, pNormals))
        {
            return false;
        }

        const float d2 = DistanceSquaredPointConvexPolygon(cCenter, pVertices, pNormals);
        return d2 <= cRadius * cRadius;
    }

    bool Collision2D::IntersectsCircleCapsule(const Vector2& circleCenter, const float circleRadius, const Vector2& capsuleA, const Vector2& capsuleB, const float capsuleRadius)
    {
        const float r = circleRadius + capsuleRadius;
        float t;
        Vector2 closestPoint;
        const float d2 = DistanceSquaredPointSegment(circleCenter, capsuleA, capsuleB, t, closestPoint);
        return d2 <= r * r;
    }

    bool Collision2D::IntersectsCapsuleCapsule(const Vector2& a0, const Vector2& a1, const float aRadius, const Vector2& b0, const Vector2& b1, const float bRadius)
    {
        const float r = aRadius + bRadius;
        const float rr = r * r;

        float s;
        float t;
        Vector2 c1;
        Vector2 c2;
        const float d2 = DistanceSquaredSegmentSegment(a0, a1, b0, b1, s, t, c1, c2);
        return d2 <= rr;
    }

    bool Collision2D::IntersectsConvexPolygonConvexPolygon(
        const std::vector<Vector2>& aVertices, const std::vector<Vector2>& aNormals, const std::vector<Vector2>& bVertices, const std::vector<Vector2>& bNormals)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.2.1 "Separating-axis Test" (2D adaptation)

        if (!IsValidPolygon(aVertices, aNormals) || !IsValidPolygon(bVertices, bNormals))
        {
            return false;
        }

        // Test A's axis
        for (std::size_t i = 0; i < aVertices.size(); i++)
        {
            if (!OverlapOnAxis(aVertices, bVertices, aNormals[i]))
            {
                return false;
            }
        }

        // Test B's axis
        for (std::size_t i = 0; i < bVertices.size(); i++)
        {
            if (!OverlapOnAxis(aVertices, bVertices, bNormals[i]))
            {
                return false;
            }
        }

        return true;
    }
}
