// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CollisionWorld3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::CollisionWorld2D, brought to parity
// (2026-07-14, user-requested): named-Layer/LayerPair cross-layer filtering, mirroring
// CollisionWorld2D.hpp/.cpp method-for-method (Layer3DEXT/LayerPair3DEXT in place of
// Layer/LayerPair; CNA::Extended::Collisions::UndefinedLayerException reused directly --
// it's a generic std::string-message type with no 2D-specific logic, so a small
// World3DEXT -> Collisions include was judged an acceptable trade against duplicating an
// otherwise-identical file).
//
// Two deliberate 3D-specific deviations from the 2D 1:1 mirror, both for backward
// compatibility with this project's own pre-existing (pre-layer) CollisionWorld3DEXT call
// sites:
//   1. QueryCandidates(actor, otherLayerName)/QueryCollisions(actor, otherLayerName)/
//      QueryCollisionPairs(firstLayerName, secondLayerName) default every layer-name
//      parameter to "" (-> the default layer), where CollisionWorld2D leaves them required.
//      This keeps QueryCollisionPairs() -- today's zero-arg call -- compiling unchanged.
//   2. The no-arg CollisionWorld3DEXT() constructor auto-registers a "default" layer
//      wrapping SpatialHash3DEXT(16.0f) (today's exact default broadphase). This is NOT
//      what CollisionWorld2D() does -- verified directly against CollisionWorld2D.cpp:
//      `CollisionWorld2D() = default;` leaves `layers_` empty, so a bare
//      `CollisionWorld2D world;` followed by `world.Insert(actor)` would itself throw
//      UndefinedLayerException until SetDefaultLayer (or the explicit
//      CollisionWorld2D(unique_ptr<Layer>) constructor) is used. That's a real, if
//      easy-to-miss, 2D API trap this port deliberately does not reproduce: every existing
//      3D caller already does `CollisionWorld3DEXT()` then `Insert(actor)`/
//      `QueryCandidates(bounds)` with no layer awareness at all, and preserving that
//      zero-setup behavior matters more here than exact-only-in-a-technicality parity with
//      an upstream default-constructor quirk.
#pragma once

#include "CNA/Extended/World3DEXT/CollisionEvent3DEXT.hpp"
#include "CNA/Extended/World3DEXT/CollisionPair3DEXT.hpp"
#include "CNA/Extended/World3DEXT/LayerPair3DEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    class ICollisionActor3DEXT;
    class ICollisionBroadphase3DEXT;
    class Layer3DEXT;

    /**
     * @brief Stores 3D collision actors in named layers and provides broadphase,
     * narrowphase, and pair-query operations.
     * @remark Layer filtering is explicit: collisions are considered only for layer pairs
     * that have been enabled.
     * @see ICollisionActor3DEXT, the interface your own game-object types implement to
     * participate in a CollisionWorld3DEXT.
     * @see CollisionShape3DEXT for the low-level shape-vs-shape intersection/collision
     * tests this class builds broadphase/narrowphase queries on top of.
     */
    class CollisionWorld3DEXT
    {
    public:
        /** @brief The name of the default collision layer. */
        static const std::string DefaultLayerName;

        /** @brief Initializes a new CollisionWorld3DEXT with a default layer wrapping a SpatialHash3DEXT(16.0f) broadphase. */
        CollisionWorld3DEXT();

        /**
         * @brief Initializes a new CollisionWorld3DEXT with an explicit broadphase, wrapped as the default layer.
         * @param broadphase The broadphase this world will own and query. Must not be null.
         */
        explicit CollisionWorld3DEXT(std::unique_ptr<ICollisionBroadphase3DEXT> broadphase);

        /**
         * @brief Initializes a new CollisionWorld3DEXT with a default layer.
         * @param defaultLayer The layer used for actors inserted into DefaultLayerName. Must not be null.
         */
        explicit CollisionWorld3DEXT(std::unique_ptr<Layer3DEXT> defaultLayer);

        ~CollisionWorld3DEXT();
        CollisionWorld3DEXT(const CollisionWorld3DEXT&) = delete;
        CollisionWorld3DEXT& operator=(const CollisionWorld3DEXT&) = delete;

        /** @brief Gets the registered collision layers by name. */
        [[nodiscard]] const std::unordered_map<std::string, std::unique_ptr<Layer3DEXT>>& getLayersProperty() const { return layers_; }

        /**
         * @brief Sets the default collision layer.
         * @param layer The layer used for actors inserted into DefaultLayerName. Must not be null.
         * @remark When a default layer is set, self-collision is enabled for that layer and
         * cross-layer collision is enabled between the default layer and every currently
         * registered layer. Adding later non-default layers enables collision between those
         * layers and the current default layer.
         */
        void SetDefaultLayer(std::unique_ptr<Layer3DEXT> layer);

        /**
         * @brief Adds a named collision layer.
         * @param name The unique layer name.
         * @param layer The layer to add. Must not be null.
         * @remark New non-default layers automatically enable self-collision. If the default
         * layer already exists, collision is also enabled between the new layer and the
         * default layer.
         */
        void AddLayer(const std::string& name, std::unique_ptr<Layer3DEXT> layer);

        /**
         * @brief Removes a named collision layer.
         * @param name The layer name to remove.
         * @return true if the layer was removed; otherwise, false.
         */
        bool RemoveLayer(const std::string& name);

        /** @brief Inserts an actor into the default collision layer. */
        void Insert(ICollisionActor3DEXT* actor);

        /**
         * @brief Inserts an actor into the specified collision layer.
         * @param actor The actor to insert.
         * @param layerName The name of the registered layer that will contain the actor.
         */
        void Insert(ICollisionActor3DEXT* actor, const std::string& layerName);

        /** @brief Returns whether the specified actor is present in this collision world. */
        [[nodiscard]] bool Contains(ICollisionActor3DEXT* actor) const;

        /**
         * @brief Tries to get the name of the layer that currently contains the specified actor.
         * @param actor The actor to look up.
         * @param layerName Receives the actor's current layer name if present; otherwise cleared.
         * @return true if the actor is present in this collision world; otherwise, false.
         */
        [[nodiscard]] bool TryGetLayerName(ICollisionActor3DEXT* actor, std::string& layerName) const;

        /** @brief Gets the name of the layer that currently contains the specified actor. */
        [[nodiscard]] std::string GetLayerName(ICollisionActor3DEXT* actor) const;

        /** @brief Moves an actor from its current collision layer into another registered layer in this world. */
        void MoveToLayer(ICollisionActor3DEXT* actor, const std::string& layerName);

        /** @brief Removes an actor from its assigned collision layer. Returns true if the actor was present. */
        bool Remove(ICollisionActor3DEXT* actor);

        /**
         * @brief Rebuilds every dynamic layer in this collision world using every actor's current broadphase bounds.
         * @remark A convenience wrapper over Layer3DEXT::Reset() for each registered layer.
         */
        void RebuildDynamicLayers();

        /**
         * @brief Queries one collision layer for broadphase candidates whose bounds overlap the specified area.
         * @param bounds The axis-aligned query bounds in world space.
         * @param layerName The layer name to query, or empty to query the default layer.
         */
        [[nodiscard]] std::vector<ICollisionActor3DEXT*> QueryCandidates(const Microsoft::Xna::Framework::BoundingBox& bounds,
                                                                          const std::string& layerName = "") const;

        /**
         * @brief Queries a target layer for broadphase candidates for the specified actor after applying layer filtering.
         * @param actor The actor whose broadphase bounds are used for the query.
         * @param otherLayerName The target layer name, or empty to query the default layer.
         * @return The actors whose broadphase bounds overlap actor in the target layer when
         * collision is enabled between the two layers; otherwise, an empty sequence.
         */
        [[nodiscard]] std::vector<ICollisionActor3DEXT*> QueryCandidates(ICollisionActor3DEXT* actor, const std::string& otherLayerName = "") const;

        /**
         * @brief Queries one collision layer for actors that collide with the specified actor and
         * returns collision results relative to that actor.
         * @param actor The actor whose broadphase bounds are used for the query.
         * @param otherLayerName The target layer name, or empty to query the default layer.
         * @remark Each returned CollisionEvent3DEXT::Result follows the receiver-relative
         * direction convention: its minimum translation vector moves actor out of the
         * returned CollisionEvent3DEXT::Other.
         */
        [[nodiscard]] std::vector<CollisionEvent3DEXT> QueryCollisions(ICollisionActor3DEXT* actor, const std::string& otherLayerName = "") const;

        /**
         * @brief Queries two layers for colliding actor pairs and returns collision results when needed.
         * @param firstLayerName The first layer name, or empty for the default layer.
         * @param secondLayerName The second layer name, or empty for the default layer.
         * @remark Each unordered actor pair is returned at most once per query, even when both
         * actors are in the same layer or when broadphase storage places them in multiple
         * candidate buckets. CollisionPair3DEXT::FirstResult moves CollisionPair3DEXT::First
         * out of CollisionPair3DEXT::Second.
         */
        [[nodiscard]] std::vector<CollisionPair3DEXT> QueryCollisionPairs(const std::string& firstLayerName = "",
                                                                           const std::string& secondLayerName = "") const;

        /**
         * @brief Enables collision between two layers.
         * @remark Layer ordering does not affect the stored rule. Enabling collision between two
         * layers also enables queries in the reversed order.
         */
        void EnableCollisionBetweenLayers(const std::string& firstLayerName, const std::string& secondLayerName);

        /**
         * @brief Disables collision between two layers.
         * @remark Layer ordering does not affect the stored rule.
         */
        void DisableCollisionBetweenLayers(const std::string& firstLayerName, const std::string& secondLayerName);

        /** @brief Returns whether collision is enabled between two layers. */
        [[nodiscard]] bool IsCollisionEnabledBetweenLayers(const std::string& firstLayerName, const std::string& secondLayerName) const;

    private:
        void EnableCollisionBetweenLayers(Layer3DEXT& firstLayer, Layer3DEXT& secondLayer);
        [[nodiscard]] Layer3DEXT& GetActorLayer(ICollisionActor3DEXT* actor) const;
        [[nodiscard]] Layer3DEXT& GetLayer(const std::string& layerName) const;

        std::unordered_map<std::string, std::unique_ptr<Layer3DEXT>> layers_;
        std::unordered_map<ICollisionActor3DEXT*, std::string> actorLayerNames_;
        std::unordered_set<LayerPair3DEXT> layerCollision_;
    };
}
