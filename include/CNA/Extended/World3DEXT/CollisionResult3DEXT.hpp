// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CollisionResult3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::CollisionResult2D, same shape (Intersects/Normal/
// PenetrationDepth/MinimumTranslationVector, plus Invert()) with Vector3 in place of
// Vector2.
#pragma once

#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    /** @brief The result of a three-dimensional collision query. */
    struct CollisionResult3DEXT
    {
        /** @brief A collision result representing no intersection (Intersects false, direction data zero). */
        static const CollisionResult3DEXT None;

        /** @brief Whether the queried shapes intersect. */
        bool Intersects = false;

        /** @brief The collision normal used to resolve the intersection. Vector3::Zero for None. */
        Vector3 Normal;

        /** @brief The distance the queried shapes overlap along the collision normal, in world units. */
        float PenetrationDepth = 0.0f;

        /** @brief The translation vector that resolves the collision. Vector3::Zero for None. */
        Vector3 MinimumTranslationVector;

        CollisionResult3DEXT() = default;

        /**
         * @brief Initializes a new CollisionResult3DEXT.
         * @param intersects Whether the queried shapes intersect.
         * @param normal The collision normal used to resolve the intersection.
         * @param penetrationDepth The overlap distance along the collision normal, in world units.
         * @param minimumTranslationVector The translation vector that resolves the collision.
         */
        CollisionResult3DEXT(bool intersects, const Vector3& normal, float penetrationDepth, const Vector3& minimumTranslationVector);

        /** @brief Returns a copy of this result with Normal and MinimumTranslationVector negated (the reversed-perspective result). */
        [[nodiscard]] CollisionResult3DEXT Invert() const;
    };
}
