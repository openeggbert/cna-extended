// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CollisionEvent3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::CollisionEvent2D, same shape with
// CollisionResult3DEXT in place of CollisionResult2D.
#pragma once

#include "CNA/Extended/World3DEXT/CollisionResult3DEXT.hpp"
#include "System/EventArgs.hpp"

namespace CNA::Extended::World3DEXT
{
    class ICollisionActor3DEXT;

    /** @brief Represents collision data for an actor collision callback or query result. */
    class CollisionEvent3DEXT : public System::EventArgs
    {
    public:
        CollisionEvent3DEXT(const ICollisionActor3DEXT& other, const CollisionResult3DEXT& result) : Other_(&other), Result_(result)
        {
        }

        /** @brief Gets the other actor involved in the collision. */
        [[nodiscard]] const ICollisionActor3DEXT& getOtherProperty() const { return *Other_; }

        /** @brief Gets the stable identity of the other actor involved in the collision. */
        [[nodiscard]] int getOtherIdProperty() const;

        /** @brief Gets the collision result for this actor relative to Other. */
        [[nodiscard]] CollisionResult3DEXT getResultProperty() const { return Result_; }

    private:
        const ICollisionActor3DEXT* Other_;
        CollisionResult3DEXT Result_;
    };
}
