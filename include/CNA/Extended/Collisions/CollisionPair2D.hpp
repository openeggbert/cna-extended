// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/CollisionPair2D.cs. C#'s `required ... { get; init; }`
// properties (must be set via object-initializer syntax at construction, immutable afterward) have
// no direct C++ property equivalent; translated as constructor parameters, matching how this
// project translates other init-only/required-at-construction C# members. ICollisionActor is a
// reference-type interface in C#; First/Second are stored as non-owning raw pointers, matching
// this project's established convention for referenced-but-not-owned polymorphic interface
// instances (see ObjectPool<T>'s IPoolable* intrusive-list nodes).
#pragma once

#include "CNA/Extended/CollisionResult2D.hpp"

namespace CNA::Extended::Collisions
{
    class ICollisionActor;

    /** @brief Represents a collision pair and the collision results for both actors. */
    class CollisionPair2D
    {
    public:
        CollisionPair2D(const ICollisionActor& first, const ICollisionActor& second, const CollisionResult2D& firstResult)
            : first_(&first), second_(&second), firstResult_(firstResult)
        {
        }

        /** @brief Gets the first actor in the collision pair. */
        [[nodiscard]] const ICollisionActor& getFirstProperty() const { return *first_; }

        /** @brief Gets the stable identity of the first actor in the collision pair. */
        [[nodiscard]] int getFirstIdProperty() const;

        /** @brief Gets the second actor in the collision pair. */
        [[nodiscard]] const ICollisionActor& getSecondProperty() const { return *second_; }

        /** @brief Gets the stable identity of the second actor in the collision pair. */
        [[nodiscard]] int getSecondIdProperty() const;

        /** @brief Gets the collision result that moves First out of Second. */
        [[nodiscard]] CollisionResult2D getFirstResultProperty() const { return firstResult_; }

        /** @brief Gets the collision result that moves Second out of First. */
        [[nodiscard]] CollisionResult2D getSecondResultProperty() const { return firstResult_.Invert(); }

    private:
        const ICollisionActor* first_;
        const ICollisionActor* second_;
        CollisionResult2D firstResult_;
    };
}
