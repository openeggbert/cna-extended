// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ICollisionActor3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::ICollisionActor, same shape with
// CollisionShape3DEXT in place of CollisionShape2D.
#pragma once

#include "CNA/Extended/World3DEXT/CollisionShape3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Defines an actor that participates in 3D collision queries. */
    class ICollisionActor3DEXT
    {
    public:
        virtual ~ICollisionActor3DEXT() = default;

        /** @brief Gets the stable identity of this actor for collision reporting. */
        [[nodiscard]] virtual int getIdProperty() const = 0;

        /** @brief Gets the collision shape used for broadphase and narrowphase collision queries. */
        [[nodiscard]] virtual CollisionShape3DEXT getShapeProperty() const = 0;
    };
}
