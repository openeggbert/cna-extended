// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CollisionWorld3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::CollisionWorld2D's core query surface
// (Insert/Remove/Contains/QueryCandidates/QueryCollisions/QueryCollisionPairs), scoped
// down deliberately: this first pass has a single implicit collision space, not
// CollisionWorld2D's full named-Layer/LayerPair cross-layer-filtering system. Layers
// were not called for by plan3d.md's Phase 5 bullet list ("registers actors, runs
// broadphase + narrow-phase collision detection per Update, raises collision events" --
// no mention of layer filtering), so adding that whole subsystem (Layer/LayerPair/
// UndefinedLayerException 3D counterparts) was judged out of scope for "start with the
// simplest correct version" -- can be added later as a real CollisionWorld3DEXT feature
// if a real multi-layer use case appears, the same way OctreeEXT.hpp documents true
// recursive octree subdivision as a future option.
#pragma once

#include "CNA/Extended/World3DEXT/CollisionEvent3DEXT.hpp"
#include "CNA/Extended/World3DEXT/CollisionPair3DEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"

#include <memory>
#include <unordered_set>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    class ICollisionActor3DEXT;
    class ICollisionBroadphase3DEXT;

    /**
     * @brief Stores 3D collision actors in a single broadphase and provides broadphase,
     * narrowphase, and pair-query operations.
     * @see ICollisionActor3DEXT, the interface your own game-object types implement to
     * participate in a CollisionWorld3DEXT.
     * @see CollisionShape3DEXT for the low-level shape-vs-shape intersection/collision
     * tests this class builds broadphase/narrowphase queries on top of.
     */
    class CollisionWorld3DEXT
    {
    public:
        /** @brief Initializes a new CollisionWorld3DEXT with a default OctreeEXT broadphase (16-unit cells). */
        CollisionWorld3DEXT();

        /**
         * @brief Initializes a new CollisionWorld3DEXT with an explicit broadphase.
         * @param broadphase The broadphase this world will own and query. Must not be null.
         */
        explicit CollisionWorld3DEXT(std::unique_ptr<ICollisionBroadphase3DEXT> broadphase);

        ~CollisionWorld3DEXT();
        CollisionWorld3DEXT(const CollisionWorld3DEXT&) = delete;
        CollisionWorld3DEXT& operator=(const CollisionWorld3DEXT&) = delete;

        /** @brief Inserts an actor into this collision world. */
        void Insert(ICollisionActor3DEXT* actor);

        /** @brief Removes an actor from this collision world. Returns true if the actor was present. */
        bool Remove(ICollisionActor3DEXT* actor);

        /** @brief Returns whether the specified actor is present in this collision world. */
        [[nodiscard]] bool Contains(ICollisionActor3DEXT* actor) const;

        /** @brief Rebuilds the broadphase using every actor's current broadphase bounds. */
        void Rebuild();

        /** @brief Queries for broadphase candidates whose bounds overlap the specified area. */
        [[nodiscard]] std::vector<ICollisionActor3DEXT*> QueryCandidates(const Microsoft::Xna::Framework::BoundingBox& bounds) const;

        /** @brief Queries for broadphase candidates whose bounds overlap the specified actor's bounds. */
        [[nodiscard]] std::vector<ICollisionActor3DEXT*> QueryCandidates(ICollisionActor3DEXT* actor) const;

        /**
         * @brief Queries for actors that collide with the specified actor and returns collision
         * results relative to that actor.
         * @remark Each returned CollisionEvent3DEXT::Result follows the receiver-relative direction
         * convention: its minimum translation vector moves actor out of the returned
         * CollisionEvent3DEXT::Other.
         */
        [[nodiscard]] std::vector<CollisionEvent3DEXT> QueryCollisions(ICollisionActor3DEXT* actor) const;

        /**
         * @brief Queries for all colliding actor pairs and returns collision results when needed.
         * @remark Each unordered actor pair is returned at most once, even when broadphase storage
         * places them in multiple candidate buckets. CollisionPair3DEXT::FirstResult moves
         * CollisionPair3DEXT::First out of CollisionPair3DEXT::Second.
         */
        [[nodiscard]] std::vector<CollisionPair3DEXT> QueryCollisionPairs() const;

    private:
        std::unique_ptr<ICollisionBroadphase3DEXT> broadphase_;
    };
}
