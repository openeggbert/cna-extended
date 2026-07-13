// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/ICollisionBroadphase2D.cs. Two C# BCL return types
// have no direct C++ equivalent and are translated for this collision-query hot path rather than
// wrapped in sharp-runtime's heap-allocating System::Collections::Generic::IEnumerable<T>
// machinery:
//   - `IEnumerable<ICollisionActor> Query(...)` -> `std::vector<ICollisionActor*>`. Upstream's
//     lazy, `yield return`-based query sequences are ported as eagerly-built vectors -- an
//     implementation-detail difference (laziness) forced by C++ lacking a built-in coroutine-based
//     generator as ergonomic as C#'s `yield return`, not a behavioral one (the set of actors
//     returned is identical either way).
//   - `List<ICollisionActor>.Enumerator GetEnumerator()` -> `const std::vector<ICollisionActor*>&
//     GetActors()`. Upstream's concrete enumerator type strongly implies both broadphase
//     implementations already keep their actors in a `List<ICollisionActor>` internally; exposing
//     that as a direct `const vector&` supports the same "enumerate everything currently stored"
//     use case (CollisionWorld2D's `foreach (ICollisionActor actor in layer.Space)`) without an
//     abstract-iterator-through-a-virtual-interface design that C++ has no lightweight idiom for.
// ICollisionActor is a reference-type interface in C#; actors are passed/returned as non-owning
// ICollisionActor* throughout, matching ObjectPool<T>'s IPoolable* convention elsewhere in this
// port. C++ pointers can be null (unlike this project's usual const-ref-for-non-nullable-parameter
// convention), but upstream's own ArgumentNullException.ThrowIfNull checks are still dropped here,
// matching how implementing broadphase types are expected to trust their caller per this project's
// "trust internal code and framework guarantees" convention -- Insert/Remove are always called
// from CollisionWorld2D with an already-null-checked actor.
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <vector>

namespace CNA::Extended
{
    struct BoundingBox2D;
}

namespace CNA::Extended::Collisions
{
    class ICollisionActor;

    /** @brief Defines a broadphase structure for collision actors. */
    class ICollisionBroadphase2D
    {
    public:
        virtual ~ICollisionBroadphase2D() = default;

        /** @brief Inserts the specified actor into the broadphase structure. */
        virtual void Insert(ICollisionActor* actor) = 0;

        /** @brief Removes the specified actor from the broadphase structure. Returns true if the actor was removed. */
        virtual bool Remove(ICollisionActor* actor) = 0;

        /** @brief Returns the actors whose broadphase bounds overlap the specified world-space bounds. */
        [[nodiscard]] virtual std::vector<ICollisionActor*> Query(const CNA::Extended::BoundingBox2D& bounds) const = 0;

        /** @brief Returns the actors currently stored in the broadphase structure. */
        [[nodiscard]] virtual const std::vector<ICollisionActor*>& GetActors() const = 0;

        /** @brief Rebuilds or refreshes the broadphase structure using the actors' current broadphase bounds. */
        virtual void Reset() = 0;
    };
}
