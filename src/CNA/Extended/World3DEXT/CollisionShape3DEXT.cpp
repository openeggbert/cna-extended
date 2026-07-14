// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// TryGetBoxSphereCollision's "sphere center exactly inside the box" branch is a deliberate
// simplification, not a bug: it falls back to a box-center-to-sphere-center direction (or
// an arbitrary axis if that is also degenerate) with PenetrationDepth == Radius, rather
// than computing the true nearest-face push-out distance a fully general SAT/GJK solver
// would. Matches this phase's "start with the simplest correct version" scope decision
// (plan3d.md section 4, Phase 5) -- the common real case (sphere center outside the box,
// which is what most collision responses actually hit) is handled exactly.
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

        if (dist >= sphere.Radius)
        {
            result = CollisionResult3DEXT::None;
            return false;
        }

        Vector3 normal;
        if (dist > 1e-6f)
        {
            normal = delta / dist;
        }
        else
        {
            const Vector3 boxCenter = (box.Min + box.Max) * 0.5f;
            const Vector3 fallback = sphere.Center - boxCenter;
            const float fallbackLength = fallback.Length();
            normal = fallbackLength > 1e-6f ? fallback / fallbackLength : Vector3(0.0f, 1.0f, 0.0f);
        }

        const float depth = sphere.Radius - dist;
        result = CollisionResult3DEXT(true, normal, depth, normal * depth);
        return true;
    }

    Vector3 CollisionShape3DEXT::GetClosestPoint(const BoundingBox& box, const Vector3& point)
    {
        return Vector3(std::clamp(point.X, box.Min.X, box.Max.X), std::clamp(point.Y, box.Min.Y, box.Max.Y),
                        std::clamp(point.Z, box.Min.Z, box.Max.Z));
    }
}
