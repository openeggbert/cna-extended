// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/CollisionEvent2D.cs. Same `required ... { get; init; }`
// -> constructor-parameter translation as CollisionPair2D.hpp; Other stored as a non-owning raw
// pointer, matching CollisionPair2D's First/Second.
#pragma once

#include "CNA/Extended/CollisionResult2D.hpp"
#include "System/EventArgs.hpp"

namespace CNA::Extended::Collisions
{
    class ICollisionActor;

    /** @brief Represents collision data for an actor collision callback or query result. */
    class CollisionEvent2D : public System::EventArgs
    {
    public:
        CollisionEvent2D(const ICollisionActor& other, const CollisionResult2D& result) : Other_(&other), Result_(result)
        {
        }

        /** @brief Gets the other actor involved in the collision. */
        [[nodiscard]] const ICollisionActor& getOtherProperty() const { return *Other_; }

        /** @brief Gets the stable identity of the other actor involved in the collision. */
        [[nodiscard]] int getOtherIdProperty() const;

        /** @brief Gets the collision result for this actor relative to Other. */
        [[nodiscard]] CollisionResult2D getResultProperty() const { return Result_; }

    private:
        const ICollisionActor* Other_;
        CollisionResult2D Result_;
    };
}
