// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ICollisionBroadphase3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::ICollisionBroadphase2D, same shape with
// Microsoft::Xna::Framework::BoundingBox in place of BoundingBox2D.
#pragma once

#include <vector>

namespace Microsoft::Xna::Framework
{
    struct BoundingBox;
}

namespace CNA::Extended::World3DEXT
{
    class ICollisionActor3DEXT;

    /** @brief Defines a broadphase structure for 3D collision actors. */
    class ICollisionBroadphase3DEXT
    {
    public:
        virtual ~ICollisionBroadphase3DEXT() = default;

        /** @brief Inserts the specified actor into the broadphase structure. */
        virtual void Insert(ICollisionActor3DEXT* actor) = 0;

        /** @brief Removes the specified actor from the broadphase structure. Returns true if the actor was removed. */
        virtual bool Remove(ICollisionActor3DEXT* actor) = 0;

        /** @brief Returns the actors whose broadphase bounds overlap the specified world-space bounds. */
        [[nodiscard]] virtual std::vector<ICollisionActor3DEXT*> Query(const Microsoft::Xna::Framework::BoundingBox& bounds) const = 0;

        /** @brief Returns the actors currently stored in the broadphase structure. */
        [[nodiscard]] virtual const std::vector<ICollisionActor3DEXT*>& GetActors() const = 0;

        /** @brief Rebuilds or refreshes the broadphase structure using the actors' current broadphase bounds. */
        virtual void Reset() = 0;
    };
}
