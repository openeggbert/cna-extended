// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/CollisionResult3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    const CollisionResult3DEXT CollisionResult3DEXT::None = CollisionResult3DEXT();

    CollisionResult3DEXT::CollisionResult3DEXT(
        const bool intersects, const Vector3& normal, const float penetrationDepth, const Vector3& minimumTranslationVector)
        : Intersects(intersects), Normal(normal), PenetrationDepth(penetrationDepth), MinimumTranslationVector(minimumTranslationVector)
    {
    }

    CollisionResult3DEXT CollisionResult3DEXT::Invert() const
    {
        return {Intersects, -Normal, PenetrationDepth, -MinimumTranslationVector};
    }
}
