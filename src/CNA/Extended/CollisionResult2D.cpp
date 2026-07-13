// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/CollisionResult2D.hpp"

namespace CNA::Extended
{
    const CollisionResult2D CollisionResult2D::None = CollisionResult2D();

    CollisionResult2D::CollisionResult2D(
        const bool intersects, const Vector2& normal, const float penetrationDepth, const Vector2& minimumTranslationVector)
        : Intersects(intersects), Normal(normal), PenetrationDepth(penetrationDepth), MinimumTranslationVector(minimumTranslationVector)
    {
    }

    CollisionResult2D CollisionResult2D::Invert() const
    {
        return {Intersects, -Normal, PenetrationDepth, -MinimumTranslationVector};
    }
}
