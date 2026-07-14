// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// TryGetBoxSphereCollision's "sphere center inside the box" branch previously used a
// deliberate simplification (box-center-to-sphere-center direction, PenetrationDepth ==
// Radius) that turned out to be a real correctness bug, not just a simplification: for a
// center inside the box, that direction is not necessarily an outward face normal, and the
// returned depth never accounts for the distance from the center to the box surface at
// all -- so the computed MTV does not actually separate the shapes in general. Found by an
// independent audit (audit.md, finding A-01) and independently re-verified before fixing
// (this project's standing "verify before trusting" discipline). Fixed per plan3d.md's
// Phase 10 entry: find the nearest of the box's 6 faces (the exact axis-aligned-box
// equivalent of "nearest point on the surface" when the query point is inside), use that
// face's outward normal, and set depth to the distance to that face *plus* the sphere's
// full radius -- the box must move that far to clear the sphere entirely, not just until
// the sphere's center reaches the face.
#include "CNA/Extended/World3DEXT/CollisionShape3DEXT.hpp"

#include <algorithm>

namespace CNA::Extended::World3DEXT
{
    CollisionShape3DEXT::CollisionShape3DEXT(const BoundingBox& box)
        : kind_(CollisionShapeKind3DEXT::Box), boundingBox_(box)
    {
    }

    CollisionShape3DEXT::CollisionShape3DEXT(const BoundingSphere& sphere)
        : kind_(CollisionShapeKind3DEXT::Sphere), boundingBox_(BoundingBox::CreateFromSphere(sphere)), sphere_(sphere)
    {
    }

    bool CollisionShape3DEXT::Intersects(const CollisionShape3DEXT& other) const
    {
        if (kind_ == CollisionShapeKind3DEXT::None || other.kind_ == CollisionShapeKind3DEXT::None)
        {
            return false;
        }

        if (kind_ == CollisionShapeKind3DEXT::Box)
        {
            return other.kind_ == CollisionShapeKind3DEXT::Box ? boundingBox_.Intersects(other.boundingBox_)
                                                                 : boundingBox_.Intersects(other.sphere_);
        }

        return other.kind_ == CollisionShapeKind3DEXT::Sphere ? sphere_.Intersects(other.sphere_)
                                                                : sphere_.Intersects(other.boundingBox_);
    }

    bool CollisionShape3DEXT::TryGetCollision(const CollisionShape3DEXT& other, CollisionResult3DEXT& result) const
    {
        if (kind_ == CollisionShapeKind3DEXT::None || other.kind_ == CollisionShapeKind3DEXT::None)
        {
            result = CollisionResult3DEXT::None;
            return false;
        }

        if (kind_ == CollisionShapeKind3DEXT::Box && other.kind_ == CollisionShapeKind3DEXT::Box)
        {
            return TryGetBoxBoxCollision(boundingBox_, other.boundingBox_, result);
        }

        if (kind_ == CollisionShapeKind3DEXT::Sphere && other.kind_ == CollisionShapeKind3DEXT::Sphere)
        {
            return TryGetSphereSphereCollision(sphere_, other.sphere_, result);
        }

        if (kind_ == CollisionShapeKind3DEXT::Box && other.kind_ == CollisionShapeKind3DEXT::Sphere)
        {
            return TryGetBoxSphereCollision(boundingBox_, other.sphere_, result);
        }

        // kind_ == Sphere, other.kind_ == Box: "moves this(sphere) out of other(box)" is the
        // inverse of "moves the box out of the sphere", which TryGetBoxSphereCollision already
        // computes.
        if (TryGetBoxSphereCollision(other.boundingBox_, sphere_, result))
        {
            result = result.Invert();
            return true;
        }
        return false;
    }

    bool CollisionShape3DEXT::TryGetBoxBoxCollision(const BoundingBox& first, const BoundingBox& second, CollisionResult3DEXT& result)
    {
        const float overlapX = std::min(first.Max.X, second.Max.X) - std::max(first.Min.X, second.Min.X);
        const float overlapY = std::min(first.Max.Y, second.Max.Y) - std::max(first.Min.Y, second.Min.Y);
        const float overlapZ = std::min(first.Max.Z, second.Max.Z) - std::max(first.Min.Z, second.Min.Z);

        if (overlapX <= 0.0f || overlapY <= 0.0f || overlapZ <= 0.0f)
        {
            result = CollisionResult3DEXT::None;
            return false;
        }

        const Vector3 firstCenter = (first.Min + first.Max) * 0.5f;
        const Vector3 secondCenter = (second.Min + second.Max) * 0.5f;
        const Vector3 delta = firstCenter - secondCenter;

        Vector3 normal;
        float depth;
        if (overlapX <= overlapY && overlapX <= overlapZ)
        {
            depth = overlapX;
            normal = Vector3(delta.X >= 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
        }
        else if (overlapY <= overlapZ)
        {
            depth = overlapY;
            normal = Vector3(0.0f, delta.Y >= 0.0f ? 1.0f : -1.0f, 0.0f);
        }
        else
        {
            depth = overlapZ;
            normal = Vector3(0.0f, 0.0f, delta.Z >= 0.0f ? 1.0f : -1.0f);
        }

        result = CollisionResult3DEXT(true, normal, depth, normal * depth);
        return true;
    }

    bool CollisionShape3DEXT::TryGetSphereSphereCollision(const BoundingSphere& first, const BoundingSphere& second, CollisionResult3DEXT& result)
    {
        const Vector3 delta = first.Center - second.Center;
        const float dist = delta.Length();
        const float radiusSum = first.Radius + second.Radius;

        if (dist >= radiusSum)
        {
            result = CollisionResult3DEXT::None;
            return false;
        }

        const Vector3 normal = dist > 1e-6f ? delta / dist : Vector3(0.0f, 1.0f, 0.0f);
        const float depth = radiusSum - dist;
        result = CollisionResult3DEXT(true, normal, depth, normal * depth);
        return true;
    }

    bool CollisionShape3DEXT::TryGetBoxSphereCollision(const BoundingBox& box, const BoundingSphere& sphere, CollisionResult3DEXT& result)
    {
        const Vector3 closest = GetClosestPoint(box, sphere.Center);
        const Vector3 delta = closest - sphere.Center;
        const float dist = delta.Length();

        if (dist > 1e-6f)
        {
            // Sphere center is strictly outside the box: closest is the real nearest
            // surface point, and delta already points from the sphere toward the box --
            // the correct MTV direction to move the box away from the sphere.
            if (dist >= sphere.Radius)
            {
                result = CollisionResult3DEXT::None;
                return false;
            }

            const Vector3 normal = delta / dist;
            const float depth = sphere.Radius - dist;
            result = CollisionResult3DEXT(true, normal, depth, normal * depth);
            return true;
        }

        // Sphere center is inside (or exactly on the surface of) the box: GetClosestPoint
        // degenerates to the center itself, so no "nearest surface point" direction exists
        // from that alone. Find the nearest of the box's 6 faces instead -- the box must
        // move out through that face by the distance to it, plus the sphere's full radius,
        // so the sphere clears the face entirely rather than merely touching it.
        const float distToMaxX = box.Max.X - sphere.Center.X;
        const float distToMinX = sphere.Center.X - box.Min.X;
        const float distToMaxY = box.Max.Y - sphere.Center.Y;
        const float distToMinY = sphere.Center.Y - box.Min.Y;
        const float distToMaxZ = box.Max.Z - sphere.Center.Z;
        const float distToMinZ = sphere.Center.Z - box.Min.Z;

        float faceDistance = distToMaxX;
        Vector3 normal(1.0f, 0.0f, 0.0f);
        if (distToMinX < faceDistance)
        {
            faceDistance = distToMinX;
            normal = Vector3(-1.0f, 0.0f, 0.0f);
        }
        if (distToMaxY < faceDistance)
        {
            faceDistance = distToMaxY;
            normal = Vector3(0.0f, 1.0f, 0.0f);
        }
        if (distToMinY < faceDistance)
        {
            faceDistance = distToMinY;
            normal = Vector3(0.0f, -1.0f, 0.0f);
        }
        if (distToMaxZ < faceDistance)
        {
            faceDistance = distToMaxZ;
            normal = Vector3(0.0f, 0.0f, 1.0f);
        }
        if (distToMinZ < faceDistance)
        {
            faceDistance = distToMinZ;
            normal = Vector3(0.0f, 0.0f, -1.0f);
        }

        const float depth = faceDistance + sphere.Radius;
        result = CollisionResult3DEXT(true, normal, depth, normal * depth);
        return true;
    }

    Vector3 CollisionShape3DEXT::GetClosestPoint(const BoundingBox& box, const Vector3& point)
    {
        return Vector3(std::clamp(point.X, box.Min.X, box.Max.X), std::clamp(point.Y, box.Min.Y, box.Max.Y),
                        std::clamp(point.Z, box.Min.Z, box.Max.Z));
    }
}
