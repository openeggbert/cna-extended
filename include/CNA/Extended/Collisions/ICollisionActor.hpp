// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/ICollisionActor.cs. Upstream lives in the
// MonoGame.Extended.Collisions namespace (distinct from MonoGame.Extended itself, where
// Collision2D/CollisionShape2D/the bounding-volume types live) -- ported into the
// CNA::Extended::Collisions sub-namespace, matching this project's "sub-namespaced per module"
// convention and mirroring upstream's own namespace split, not a new decision made up for this
// file alone.
#pragma once

#include "CNA/Extended/CollisionShape2D.hpp"

namespace CNA::Extended::Collisions
{
    using CNA::Extended::CollisionShape2D;

    /** @brief Defines an actor that participates in collision queries. */
    class ICollisionActor
    {
    public:
        virtual ~ICollisionActor() = default;

        /** @brief Gets the stable identity of this actor for collision reporting. */
        [[nodiscard]] virtual int getIdProperty() const = 0;

        /** @brief Gets the collision shape used for broadphase and narrowphase collision queries. */
        [[nodiscard]] virtual CollisionShape2D getShapeProperty() const = 0;
    };
}
