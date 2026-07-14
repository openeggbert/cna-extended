// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CollisionShape3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::CollisionShape2D: a tagged union wrapping one of cna's
// real bounding-volume types (BoundingBox, BoundingSphere -- both already reused elsewhere
// in World3DEXT, e.g. RenderSystem3DEXT's frustum culling), scoped down to those two kinds
// rather than porting CollisionShape2D's full 5-kind set (see CollisionShapeKind3DEXT.hpp's
// header comment for why). Intersects() delegates entirely to BoundingBox/BoundingSphere's
// own real Intersects methods, matching CollisionShape2D's own delegation-only design.
// TryGetCollision()'s minimum-translation-vector math (Box/Box, Sphere/Sphere, Box/Sphere)
// is standard, well-known collision-resolution math (closest-point-on-box-to-sphere-center
// for Box/Sphere; per-axis overlap for Box/Box; center-distance-minus-radii for
// Sphere/Sphere) -- not adapted from any specific source, analogous to how CollisionShape2D
// implements its own legacy penetration-vector helpers as private static methods rather
// than delegating them elsewhere.
#pragma once

#include "CNA/Extended/World3DEXT/CollisionResult3DEXT.hpp"
#include "CNA/Extended/World3DEXT/CollisionShapeKind3DEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;
    using Microsoft::Xna::Framework::BoundingSphere;

    /**
     * @brief Represents a collision shape for three-dimensional intersection and collision queries.
     * @remark The default value represents a None shape and does not intersect any other shape.
     */
    class CollisionShape3DEXT
    {
    public:
        CollisionShape3DEXT() = default;

        /**
         * @brief Creates a collision shape from an axis-aligned bounding box.
         * @param box The axis-aligned bounding box to represent.
         * @remark The supplied box is stored directly and also serves as the broadphase bounds for this shape.
         */
        explicit CollisionShape3DEXT(const BoundingBox& box);

        /**
         * @brief Creates a collision shape from a bounding sphere.
         * @param sphere The bounding sphere to represent.
         * @remark The broadphase bounds are cached from the sphere via BoundingBox::CreateFromSphere.
         */
        explicit CollisionShape3DEXT(const BoundingSphere& sphere);

        /** @brief Gets which shape kind this instance currently represents. */
        [[nodiscard]] CollisionShapeKind3DEXT getKindProperty() const { return kind_; }

        /** @brief Gets the axis-aligned broadphase bounds for this collision shape. */
        [[nodiscard]] BoundingBox getBoundingBoxProperty() const { return boundingBox_; }

        /** @brief Gets the box this shape represents. Only meaningful when getKindProperty() == Box. */
        [[nodiscard]] const BoundingBox& getBoxProperty() const { return boundingBox_; }

        /** @brief Gets the sphere this shape represents. Only meaningful when getKindProperty() == Sphere. */
        [[nodiscard]] const BoundingSphere& getSphereProperty() const { return sphere_; }

        /**
         * @brief Determines whether this collision shape intersects with another collision shape.
         * @param other The other collision shape to test against.
         * @return true if the shapes overlap or touch; otherwise, false.
         * @remark Delegates to BoundingBox/BoundingSphere's own real Intersects methods. If either
         * shape is the default None shape, this method returns false.
         */
        [[nodiscard]] bool Intersects(const CollisionShape3DEXT& other) const;

        /**
         * @brief Tests whether this collision shape intersects with another, and returns collision
         * resolution data (normal, penetration depth, minimum translation vector) when it does.
         * @param other The other collision shape to test against.
         * @param result Receives the collision result whose minimum translation vector moves this
         * shape out of other when this method returns true; otherwise CollisionResult3DEXT::None.
         * @return true if the shapes overlap or touch; otherwise, false.
         * @remark If either shape is the default None shape, returns false and sets result to
         * CollisionResult3DEXT::None.
         */
        [[nodiscard]] bool TryGetCollision(const CollisionShape3DEXT& other, CollisionResult3DEXT& result) const;

    private:
        [[nodiscard]] static bool TryGetBoxBoxCollision(const BoundingBox& first, const BoundingBox& second, CollisionResult3DEXT& result);
        [[nodiscard]] static bool TryGetSphereSphereCollision(const BoundingSphere& first, const BoundingSphere& second, CollisionResult3DEXT& result);
        [[nodiscard]] static bool TryGetBoxSphereCollision(const BoundingBox& box, const BoundingSphere& sphere, CollisionResult3DEXT& result);
        [[nodiscard]] static Vector3 GetClosestPoint(const BoundingBox& box, const Vector3& point);

        CollisionShapeKind3DEXT kind_ = CollisionShapeKind3DEXT::None;
        BoundingBox boundingBox_;
        BoundingSphere sphere_;
    };
}
