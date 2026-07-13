// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/CollisionShape2D.hpp"

#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/Line2D.hpp"
#include "CNA/Extended/LineSegment2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "CNA/Extended/Ray2D.hpp"
#include "CNA/Extended/Vector2Extensions.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <algorithm>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::MathHelper;

    CollisionShape2D::CollisionShape2D(const BoundingBox2D& box) : kind_(CollisionShapeKind2D::Box), boundingBox_(box)
    {
    }

    CollisionShape2D::CollisionShape2D(const BoundingCircle2D& circle)
        : kind_(CollisionShapeKind2D::Circle), primary_(circle.Center), scalar_(circle.Radius)
    {
        const Vector2 radiusVector(circle.Radius, circle.Radius);
        boundingBox_ = BoundingBox2D::CreateFromMinMax(circle.Center - radiusVector, circle.Center + radiusVector);
    }

    CollisionShape2D::CollisionShape2D(const OrientedBoundingBox2D& box)
        : kind_(CollisionShapeKind2D::OrientedBox), primary_(box.Center), secondary_(box.AxisX), tertiary_(box.HalfExtents)
    {
        boundingBox_ = BoundingBox2D::CreateFromPoints(box.GetCorners());
    }

    CollisionShape2D::CollisionShape2D(const BoundingCapsule2D& capsule)
        : kind_(CollisionShapeKind2D::Capsule), primary_(capsule.PointA), secondary_(capsule.PointB), scalar_(capsule.Radius)
    {
        const Vector2 radiusVector(capsule.Radius, capsule.Radius);
        const Vector2 min = Vector2::Min(capsule.PointA, capsule.PointB) - radiusVector;
        const Vector2 max = Vector2::Max(capsule.PointA, capsule.PointB) + radiusVector;
        boundingBox_ = BoundingBox2D::CreateFromMinMax(min, max);
    }

    CollisionShape2D::CollisionShape2D(const BoundingPolygon2D& polygon)
        : kind_(CollisionShapeKind2D::Polygon), polygonVertices_(polygon.Vertices), polygonNormals_(polygon.Normals)
    {
        boundingBox_ = BoundingBox2D::CreateFromPoints(polygon.Vertices);
    }

    BoundingCircle2D CollisionShape2D::getCircleProperty() const
    {
        return BoundingCircle2D(primary_, scalar_);
    }

    OrientedBoundingBox2D CollisionShape2D::getOrientedBoxProperty() const
    {
        const Vector2 axisY(-secondary_.Y, secondary_.X);
        return OrientedBoundingBox2D(primary_, secondary_, axisY, tertiary_);
    }

    BoundingCapsule2D CollisionShape2D::getCapsuleProperty() const
    {
        return BoundingCapsule2D(primary_, secondary_, scalar_);
    }

    BoundingPolygon2D CollisionShape2D::getPolygonProperty() const
    {
        return BoundingPolygon2D(polygonVertices_, polygonNormals_);
    }

    bool CollisionShape2D::Intersects(const CollisionShape2D& other) const
    {
        if (kind_ == CollisionShapeKind2D::None || other.kind_ == CollisionShapeKind2D::None)
        {
            return false;
        }

        switch (kind_)
        {
            case CollisionShapeKind2D::Box:
                switch (other.kind_)
                {
                    case CollisionShapeKind2D::Box:
                        return boundingBox_.Intersects(other.boundingBox_);
                    case CollisionShapeKind2D::Circle:
                        return boundingBox_.Intersects(other.getCircleProperty());
                    case CollisionShapeKind2D::OrientedBox:
                        return boundingBox_.Intersects(other.getOrientedBoxProperty());
                    case CollisionShapeKind2D::Capsule:
                        return boundingBox_.Intersects(other.getCapsuleProperty());
                    case CollisionShapeKind2D::Polygon:
                        return boundingBox_.Intersects(other.getPolygonProperty());
                    default:
                        return false;
                }

            case CollisionShapeKind2D::Circle:
                switch (other.kind_)
                {
                    case CollisionShapeKind2D::Box:
                        return getCircleProperty().Intersects(other.boundingBox_);
                    case CollisionShapeKind2D::Circle:
                        return getCircleProperty().Intersects(other.getCircleProperty());
                    case CollisionShapeKind2D::OrientedBox:
                        return getCircleProperty().Intersects(other.getOrientedBoxProperty());
                    case CollisionShapeKind2D::Capsule:
                        return getCircleProperty().Intersects(other.getCapsuleProperty());
                    case CollisionShapeKind2D::Polygon:
                        return getCircleProperty().Intersects(other.getPolygonProperty());
                    default:
                        return false;
                }

            case CollisionShapeKind2D::OrientedBox:
                switch (other.kind_)
                {
                    case CollisionShapeKind2D::Box:
                        return getOrientedBoxProperty().Intersects(other.boundingBox_);
                    case CollisionShapeKind2D::Circle:
                        return getOrientedBoxProperty().Intersects(other.getCircleProperty());
                    case CollisionShapeKind2D::OrientedBox:
                        return getOrientedBoxProperty().Intersects(other.getOrientedBoxProperty());
                    case CollisionShapeKind2D::Capsule:
                        return getOrientedBoxProperty().Intersects(other.getCapsuleProperty());
                    case CollisionShapeKind2D::Polygon:
                        return getOrientedBoxProperty().Intersects(other.getPolygonProperty());
                    default:
                        return false;
                }

            case CollisionShapeKind2D::Capsule:
                switch (other.kind_)
                {
                    case CollisionShapeKind2D::Box:
                        return getCapsuleProperty().Intersects(other.boundingBox_);
                    case CollisionShapeKind2D::Circle:
                        return getCapsuleProperty().Intersects(other.getCircleProperty());
                    case CollisionShapeKind2D::OrientedBox:
                        return getCapsuleProperty().Intersects(other.getOrientedBoxProperty());
                    case CollisionShapeKind2D::Capsule:
                        return getCapsuleProperty().Intersects(other.getCapsuleProperty());
                    case CollisionShapeKind2D::Polygon:
                        return getCapsuleProperty().Intersects(other.getPolygonProperty());
                    default:
                        return false;
                }

            case CollisionShapeKind2D::Polygon:
                switch (other.kind_)
                {
                    case CollisionShapeKind2D::Box:
                        return getPolygonProperty().Intersects(other.boundingBox_);
                    case CollisionShapeKind2D::Circle:
                        return getPolygonProperty().Intersects(other.getCircleProperty());
                    case CollisionShapeKind2D::OrientedBox:
                        return getPolygonProperty().Intersects(other.getOrientedBoxProperty());
                    case CollisionShapeKind2D::Capsule:
                        return getPolygonProperty().Intersects(other.getCapsuleProperty());
                    case CollisionShapeKind2D::Polygon:
                        return getPolygonProperty().Intersects(other.getPolygonProperty());
                    default:
                        return false;
                }

            default:
                return false;
        }
    }

    bool CollisionShape2D::Intersects(const Ray2D& ray) const
    {
        if (kind_ == CollisionShapeKind2D::None)
        {
            return false;
        }
        switch (kind_)
        {
            case CollisionShapeKind2D::Box:
                return ray.Intersects(boundingBox_);
            case CollisionShapeKind2D::Circle:
                return ray.Intersects(getCircleProperty());
            case CollisionShapeKind2D::OrientedBox:
                return ray.Intersects(getOrientedBoxProperty());
            case CollisionShapeKind2D::Capsule:
                return ray.Intersects(getCapsuleProperty());
            case CollisionShapeKind2D::Polygon:
                return ray.Intersects(getPolygonProperty());
            default:
                return false;
        }
    }

    bool CollisionShape2D::Intersects(const Ray2D& ray, std::optional<float>& tMin, std::optional<float>& tMax) const
    {
        tMin = 0.0f;
        tMax = 0.0f;
        if (kind_ == CollisionShapeKind2D::None)
        {
            return false;
        }
        switch (kind_)
        {
            case CollisionShapeKind2D::Box:
                return ray.Intersects(boundingBox_, tMin, tMax);
            case CollisionShapeKind2D::Circle:
                return ray.Intersects(getCircleProperty(), tMin, tMax);
            case CollisionShapeKind2D::OrientedBox:
                return ray.Intersects(getOrientedBoxProperty(), tMin, tMax);
            case CollisionShapeKind2D::Capsule:
                return ray.Intersects(getCapsuleProperty(), tMin, tMax);
            default:
                return false;
        }
    }

    bool CollisionShape2D::Intersects(const Line2D& line) const
    {
        if (kind_ == CollisionShapeKind2D::None)
        {
            return false;
        }
        switch (kind_)
        {
            case CollisionShapeKind2D::Box:
                return line.Intersects(boundingBox_);
            case CollisionShapeKind2D::Circle:
                return line.Intersects(getCircleProperty());
            case CollisionShapeKind2D::OrientedBox:
                return line.Intersects(getOrientedBoxProperty());
            case CollisionShapeKind2D::Capsule:
                return line.Intersects(getCapsuleProperty());
            case CollisionShapeKind2D::Polygon:
                return line.Intersects(getPolygonProperty());
            default:
                return false;
        }
    }

    bool CollisionShape2D::Intersects(const LineSegment2D& lineSegment) const
    {
        if (kind_ == CollisionShapeKind2D::None)
        {
            return false;
        }
        switch (kind_)
        {
            case CollisionShapeKind2D::Box:
                return lineSegment.Intersects(boundingBox_);
            case CollisionShapeKind2D::Circle:
                return lineSegment.Intersects(getCircleProperty());
            case CollisionShapeKind2D::OrientedBox:
                return lineSegment.Intersects(getOrientedBoxProperty());
            case CollisionShapeKind2D::Capsule:
                return lineSegment.Intersects(getCapsuleProperty());
            case CollisionShapeKind2D::Polygon:
                return lineSegment.Intersects(getPolygonProperty());
            default:
                return false;
        }
    }

    bool CollisionShape2D::Intersects(const LineSegment2D& lineSegment, std::optional<float>& tMin, std::optional<float>& tMax) const
    {
        tMin = 0.0f;
        tMax = 0.0f;
        if (kind_ == CollisionShapeKind2D::None)
        {
            return false;
        }
        switch (kind_)
        {
            case CollisionShapeKind2D::Box:
                return lineSegment.Intersects(boundingBox_, tMin, tMax);
            case CollisionShapeKind2D::Circle:
                return lineSegment.Intersects(getCircleProperty(), tMin, tMax);
            case CollisionShapeKind2D::OrientedBox:
                return lineSegment.Intersects(getOrientedBoxProperty(), tMin, tMax);
            case CollisionShapeKind2D::Capsule:
                return lineSegment.Intersects(getCapsuleProperty(), tMin, tMax);
            default:
                return false;
        }
    }

    bool CollisionShape2D::TryGetCollision(const CollisionShape2D& other, CollisionResult2D& result) const
    {
        if (kind_ == CollisionShapeKind2D::None || other.kind_ == CollisionShapeKind2D::None)
        {
            result = CollisionResult2D::None;
            return false;
        }

        switch (kind_)
        {
            case CollisionShapeKind2D::Box:
                switch (other.kind_)
                {
                    case CollisionShapeKind2D::Box:
                        return boundingBox_.TryGetCollision(other.boundingBox_, result);
                    case CollisionShapeKind2D::Circle:
                        return boundingBox_.TryGetCollision(other.getCircleProperty(), result);
                    case CollisionShapeKind2D::OrientedBox:
                        return boundingBox_.TryGetCollision(other.getOrientedBoxProperty(), result);
                    case CollisionShapeKind2D::Polygon:
                        return boundingBox_.TryGetCollision(other.getPolygonProperty(), result);
                    default:
                        result = CollisionResult2D::None;
                        return false;
                }

            case CollisionShapeKind2D::Circle:
                switch (other.kind_)
                {
                    case CollisionShapeKind2D::Box:
                        return getCircleProperty().TryGetCollision(other.boundingBox_, result);
                    case CollisionShapeKind2D::Circle:
                        return getCircleProperty().TryGetCollision(other.getCircleProperty(), result);
                    case CollisionShapeKind2D::Capsule:
                        return getCircleProperty().TryGetCollision(other.getCapsuleProperty(), result);
                    case CollisionShapeKind2D::OrientedBox:
                        return getCircleProperty().TryGetCollision(other.getOrientedBoxProperty(), result);
                    default:
                        result = CollisionResult2D::None;
                        return false;
                }

            case CollisionShapeKind2D::OrientedBox:
                switch (other.kind_)
                {
                    case CollisionShapeKind2D::Box:
                        return getOrientedBoxProperty().TryGetCollision(other.boundingBox_, result);
                    case CollisionShapeKind2D::Circle:
                        return getOrientedBoxProperty().TryGetCollision(other.getCircleProperty(), result);
                    case CollisionShapeKind2D::OrientedBox:
                        return getOrientedBoxProperty().TryGetCollision(other.getOrientedBoxProperty(), result);
                    case CollisionShapeKind2D::Polygon:
                        return getOrientedBoxProperty().TryGetCollision(other.getPolygonProperty(), result);
                    default:
                        result = CollisionResult2D::None;
                        return false;
                }

            case CollisionShapeKind2D::Capsule:
                switch (other.kind_)
                {
                    case CollisionShapeKind2D::Circle:
                        return getCapsuleProperty().TryGetCollision(other.getCircleProperty(), result);
                    default:
                        result = CollisionResult2D::None;
                        return false;
                }

            case CollisionShapeKind2D::Polygon:
                switch (other.kind_)
                {
                    case CollisionShapeKind2D::Box:
                        return getPolygonProperty().TryGetCollision(other.boundingBox_, result);
                    case CollisionShapeKind2D::OrientedBox:
                        return getPolygonProperty().TryGetCollision(other.getOrientedBoxProperty(), result);
                    case CollisionShapeKind2D::Polygon:
                        return getPolygonProperty().TryGetCollision(other.getPolygonProperty(), result);
                    default:
                        result = CollisionResult2D::None;
                        return false;
                }

            default:
                result = CollisionResult2D::None;
                return false;
        }
    }

    bool CollisionShape2D::TryGetLegacyPenetrationVector(const CollisionShape2D& other, Vector2& penetrationVector) const
    {
        if (kind_ == CollisionShapeKind2D::Circle && other.kind_ == CollisionShapeKind2D::Circle)
        {
            penetrationVector = GetLegacyCircleCirclePenetrationVector(getCircleProperty(), other.getCircleProperty());
            return true;
        }

        if (kind_ == CollisionShapeKind2D::Circle && other.kind_ == CollisionShapeKind2D::Box)
        {
            penetrationVector = GetLegacyCircleBoxPenetrationVector(getCircleProperty(), other.boundingBox_);
            return true;
        }

        if (kind_ == CollisionShapeKind2D::Box && other.kind_ == CollisionShapeKind2D::Circle)
        {
            penetrationVector = -GetLegacyCircleBoxPenetrationVector(other.getCircleProperty(), boundingBox_);
            return true;
        }

        if (kind_ == CollisionShapeKind2D::Box && other.kind_ == CollisionShapeKind2D::Box)
        {
            penetrationVector = GetLegacyBoxBoxPenetrationVector(boundingBox_, other.boundingBox_);
            return true;
        }

        penetrationVector = Vector2::Zero;
        return false;
    }

    Vector2 CollisionShape2D::GetLegacyCircleCirclePenetrationVector(const BoundingCircle2D& first, const BoundingCircle2D& second)
    {
        const Vector2 displacement = first.Center - second.Center;
        Vector2 desiredDisplacement;

        if (displacement != Vector2::Zero)
        {
            desiredDisplacement = NormalizedCopy(displacement) * (first.Radius + second.Radius);
        }
        else
        {
            desiredDisplacement = -Vector2::UnitY * (first.Radius + second.Radius);
        }

        return displacement - desiredDisplacement;
    }

    Vector2 CollisionShape2D::GetLegacyCircleBoxPenetrationVector(const BoundingCircle2D& circle, const BoundingBox2D& box)
    {
        const Vector2 collisionPoint = GetClosestPoint(box, circle.Center);
        const Vector2 circleToCollisionPoint = collisionPoint - circle.Center;

        if (Contains(box, circle.Center) || circleToCollisionPoint == Vector2::Zero)
        {
            Vector2 displacement = circle.Center - box.getCenterProperty();
            Vector2 desiredDisplacement;

            if (displacement != Vector2::Zero)
            {
                Vector2 displacementX(displacement.X, 0.0f);
                Vector2 displacementY(0.0f, displacement.Y);
                displacementX.Normalize();
                displacementY.Normalize();

                displacementX = displacementX * (circle.Radius + box.getWidthProperty() * 0.5f);
                displacementY = displacementY * (circle.Radius + box.getHeightProperty() * 0.5f);

                if (displacementX.LengthSquared() < displacementY.LengthSquared())
                {
                    desiredDisplacement = displacementX;
                    displacement.Y = 0.0f;
                }
                else
                {
                    desiredDisplacement = displacementY;
                    displacement.X = 0.0f;
                }
            }
            else
            {
                desiredDisplacement = -Vector2::UnitY * (circle.Radius + box.getHeightProperty() * 0.5f);
            }

            return displacement - desiredDisplacement;
        }

        return circle.Radius * NormalizedCopy(circleToCollisionPoint) - circleToCollisionPoint;
    }

    Vector2 CollisionShape2D::GetLegacyBoxBoxPenetrationVector(const BoundingBox2D& first, const BoundingBox2D& second)
    {
        const BoundingBox2D intersection(
            Vector2(std::max(first.Min.X, second.Min.X), std::max(first.Min.Y, second.Min.Y)),
            Vector2(std::min(first.Max.X, second.Max.X), std::min(first.Max.Y, second.Max.Y)));

        if (intersection.getWidthProperty() < intersection.getHeightProperty())
        {
            const float displacement = first.getCenterProperty().X < second.getCenterProperty().X ? intersection.getWidthProperty()
                                                                                                     : -intersection.getWidthProperty();

            return Vector2(displacement, 0.0f);
        }

        {
            const float displacement = first.getCenterProperty().Y < second.getCenterProperty().Y ? intersection.getHeightProperty()
                                                                                                     : -intersection.getHeightProperty();

            return Vector2(0.0f, displacement);
        }
    }

    bool CollisionShape2D::Contains(const BoundingBox2D& box, const Vector2& point)
    {
        return point.X >= box.Min.X && point.X <= box.Max.X && point.Y >= box.Min.Y && point.Y <= box.Max.Y;
    }

    Vector2 CollisionShape2D::GetClosestPoint(const BoundingBox2D& box, const Vector2& point)
    {
        const float x = MathHelper::Clamp(point.X, box.Min.X, box.Max.X);
        const float y = MathHelper::Clamp(point.Y, box.Min.Y, box.Max.Y);
        return Vector2(x, y);
    }
}
