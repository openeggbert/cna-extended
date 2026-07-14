// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CollisionPair3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::CollisionPair2D, same shape with
// CollisionResult3DEXT in place of CollisionResult2D.
#pragma once

#include "CNA/Extended/World3DEXT/CollisionResult3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    class ICollisionActor3DEXT;

    /** @brief Represents a collision pair and the collision results for both actors. */
    class CollisionPair3DEXT
    {
    public:
        CollisionPair3DEXT(const ICollisionActor3DEXT& first, const ICollisionActor3DEXT& second, const CollisionResult3DEXT& firstResult)
            : first_(&first), second_(&second), firstResult_(firstResult)
        {
        }

        /** @brief Gets the first actor in the collision pair. */
        [[nodiscard]] const ICollisionActor3DEXT& getFirstProperty() const { return *first_; }

        /** @brief Gets the stable identity of the first actor in the collision pair. */
        [[nodiscard]] int getFirstIdProperty() const;

        /** @brief Gets the second actor in the collision pair. */
        [[nodiscard]] const ICollisionActor3DEXT& getSecondProperty() const { return *second_; }

        /** @brief Gets the stable identity of the second actor in the collision pair. */
        [[nodiscard]] int getSecondIdProperty() const;

        /** @brief Gets the collision result that moves First out of Second. */
        [[nodiscard]] CollisionResult3DEXT getFirstResultProperty() const { return firstResult_; }

        /** @brief Gets the collision result that moves Second out of First. */
        [[nodiscard]] CollisionResult3DEXT getSecondResultProperty() const { return firstResult_.Invert(); }

    private:
        const ICollisionActor3DEXT* first_;
        const ICollisionActor3DEXT* second_;
        CollisionResult3DEXT firstResult_;
    };
}
