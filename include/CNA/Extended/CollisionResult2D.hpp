// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's CollisionResult2D.cs: the shared result model for collision
// queries that report resolution data (normal, penetration depth, minimum translation vector) in
// addition to intersection state. For static Collision2D methods, MinimumTranslationVector is the
// translation that moves the first queried shape out of the second; for instance methods on
// bounding types, it moves the receiver out of the argument (see each call site's own docs).
//
// `internal readonly CollisionResult2D Invert()` -- upstream's `internal` visibility has no C++
// equivalent; kept public, matching the precedent set elsewhere in this port for `internal`
// members that are still meaningful to call (e.g. PrimitivesHelper).
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief The result of a two-dimensional collision query. */
    struct CollisionResult2D
    {
        /** @brief A collision result representing no intersection (Intersects false, direction data zero). */
        static const CollisionResult2D None;

        /** @brief Whether the queried shapes intersect. */
        bool Intersects = false;

        /** @brief The collision normal used to resolve the intersection. Vector2::Zero for None. */
        Vector2 Normal;

        /** @brief The distance the queried shapes overlap along the collision normal, in world units. */
        float PenetrationDepth = 0.0f;

        /** @brief The translation vector that resolves the collision. Vector2::Zero for None. */
        Vector2 MinimumTranslationVector;

        CollisionResult2D() = default;

        /**
         * @brief Initializes a new CollisionResult2D.
         * @param intersects Whether the queried shapes intersect.
         * @param normal The collision normal used to resolve the intersection.
         * @param penetrationDepth The overlap distance along the collision normal, in world units.
         * @param minimumTranslationVector The translation vector that resolves the collision.
         */
        CollisionResult2D(bool intersects, const Vector2& normal, float penetrationDepth, const Vector2& minimumTranslationVector);

        /** @brief Returns a copy of this result with Normal and MinimumTranslationVector negated (the reversed-perspective result). */
        [[nodiscard]] CollisionResult2D Invert() const;
    };
}
