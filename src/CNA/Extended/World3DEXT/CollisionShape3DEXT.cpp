// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// TryGetBoxSphereCollision's "sphere center inside the box" branch has gone through two
// rounds of fixes for the same finding (audit.md A-01), both times independently
// re-verified before acting (this project's standing "verify before trusting" discipline
// -- extended here to a second round: re-verifying a *re-review*, not just the original
// audit):
//
// Round 1 replaced the original deliberate simplification (box-center-to-sphere-center
// direction, depth == radius -- wrong because that direction isn't necessarily an outward
// face normal, and the depth ignored the distance from the center to the box surface
// entirely) with "nearest face's own outward normal, depth = distance-to-that-face +
// radius". That direction was still wrong, caught by re-review: moving the box *through*
// its nearest face drags the box's OPPOSITE face across the sphere's entire width, which
// needs distance-to-the-*opposite*-face + radius, not distance-to-the-near-face + radius.
// Concrete counter-example that exposed it: box [-2,2] on X, sphere center x=1.5, radius 1
// -- round 1 returned +X/depth 1.5, translating the box to [-0.5, 3.5], which still
// overlaps the sphere's [0.5, 2.5] interval on that axis.
//
// Round 2 (this fix) keeps round 1's magnitude (nearest-face distance + radius is, in
// fact, already the correct minimal depth -- proven below) and flips only the direction:
// escaping *away* from the nearest face (through that face's own opposite side) is what
// costs distance-to-that-face + radius; escaping *through* it costs
// distance-to-the-opposite-face + radius, which is larger whenever that face really is the
// nearest one. Proof via direct SAT-style derivation: translating the box by t along +X
// requires box.Min.X + t >= sphere.Center.X + radius (box's leading edge must clear the
// sphere's far side), i.e. t >= (sphere.Center.X - box.Min.X) + radius -- the distance to
// the box's Min (-X) face, not the Max (+X) face. So the minimal t comes from whichever
// face is *nearer*, but moving in the direction *away* from it. This also matches the
// already-correct branch above (sphere strictly outside the box), whose established
// convention likewise moves the box away from the sphere's approach side, not through it.
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
        // from that alone.
        //
        // ROUND 2 FIX (audit.md A-01, caught by re-review after the first attempt): the
        // first fix moved the box *through* its nearest face -- e.g. for a box [-2,2] on X
        // with the sphere center at x=1.5, radius 1, it returned +X/depth 1.5, translating
        // the box to [-0.5, 3.5], which still overlaps the sphere's [0.5, 2.5] interval.
        // Proven by direct SAT-style derivation (moving the box by t along +X requires
        // Min.X+t >= sphere.Center+radius, i.e. t >= (sphere.Center-Min.X)+radius -- the
        // distance to the *opposite* face, not the near one): escaping through face F
        // costs distanceToOppositeFace(F)+radius, while escaping *away* from F (through F's
        // own opposite side) costs only distanceToF+radius -- the minimum of the two.  So
        // the nearest-face distance is the right magnitude (unchanged from the first fix),
        // but the escape direction is the *opposite* of that face's own outward normal --
        // matching the branch above, whose already-correct convention also moves the box
        // away from the sphere's approach side, not through it.
        const float distToMaxX = box.Max.X - sphere.Center.X;
        const float distToMinX = sphere.Center.X - box.Min.X;
        const float distToMaxY = box.Max.Y - sphere.Center.Y;
        const float distToMinY = sphere.Center.Y - box.Min.Y;
        const float distToMaxZ = box.Max.Z - sphere.Center.Z;
        const float distToMinZ = sphere.Center.Z - box.Min.Z;

        // normal is the *opposite* of the nearest face's own outward direction (see above).
        float faceDistance = distToMaxX;
        Vector3 normal(-1.0f, 0.0f, 0.0f);
        if (distToMinX < faceDistance)
        {
            faceDistance = distToMinX;
            normal = Vector3(1.0f, 0.0f, 0.0f);
        }
        if (distToMaxY < faceDistance)
        {
            faceDistance = distToMaxY;
            normal = Vector3(0.0f, -1.0f, 0.0f);
        }
        if (distToMinY < faceDistance)
        {
            faceDistance = distToMinY;
            normal = Vector3(0.0f, 1.0f, 0.0f);
        }
        if (distToMaxZ < faceDistance)
        {
            faceDistance = distToMaxZ;
            normal = Vector3(0.0f, 0.0f, -1.0f);
        }
        if (distToMinZ < faceDistance)
        {
            faceDistance = distToMinZ;
            normal = Vector3(0.0f, 0.0f, 1.0f);
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
