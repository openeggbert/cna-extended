// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/CollisionWorld2D.cs: the primary query-oriented
// actor/world collision API, storing actors in named layers and coordinating broadphase,
// narrowphase, and pair-query operations across them. Completes Phase 2 task 2, deferred since
// entry (25) pending Layer/LayerPair (task 4) and the broadphase implementations (task 3), both
// now landed.
//
// Ownership: `Dictionary<string, Layer> _layers` -> `std::unordered_map<std::string,
// std::unique_ptr<Layer>>` (CollisionWorld2D owns every Layer it's given, matching upstream's own
// intent -- a Layer is constructed specifically to be handed to a CollisionWorld2D and nothing
// else ever holds a second reference to it). `Dictionary<ICollisionActor, string>
// _actorLayerNames` -> `std::unordered_map<ICollisionActor*, std::string>` (non-owning actor
// pointers, matching this whole module's established ICollisionActor* convention).
// `HashSet<LayerPair> _layerCollision` -> `std::unordered_set<LayerPair>`.
//
// C#'s `null` layerName parameters (meaning "use the default layer", resolved by GetLayer's
// `layerName ?? DefaultLayerName`) have no direct std::string equivalent. Translated using an
// empty string as the "not specified" sentinel throughout (default argument `= ""` where upstream
// defaults to `null`, or an always-required-but-null-tolerant `const std::string&` parameter
// elsewhere) -- GetLayer resolves an empty layerName to DefaultLayerName, mirroring upstream's
// null-coalescing exactly. This is deliberately narrower than std::string::empty() might suggest:
// only GetLayer treats "" specially; methods that validate their layerName argument BEFORE calling
// GetLayer (Insert, MoveToLayer) still reject "" outright via
// System::ArgumentException::ThrowIfNullOrWhiteSpace, exactly matching upstream's own
// ArgumentException.ThrowIfNullOrWhiteSpace(layerName) calls in those methods -- so "" never
// actually reaches GetLayer's resolution logic from Insert/MoveToLayer, only from the methods that
// truly allow an omitted/unspecified layer name (QueryCandidates, QueryCollisions,
// QueryCollisionPairs, EnableCollisionBetweenLayers/DisableCollisionBetweenLayers/
// IsCollisionEnabledBetweenLayers). One deliberately-accepted, narrow fidelity gap: upstream's
// `??` only coalesces true C# null, not an explicitly-passed empty string -- so
// `QueryCandidates(bounds, "")` in real C# would look up a literal ""-named layer and throw
// UndefinedLayerException, whereas this port resolves it to the default layer. No legitimate use
// case can hit this (AddLayer/RemoveLayer already reject "" as a layer name outright, so a
// ""-named layer can never exist to be found either way), and upstream's own test suite never
// exercises it.
//
// `IEnumerable<ICollisionActor>`/`IEnumerable<CollisionEvent2D>`/`IEnumerable<CollisionPair2D>`
// return types (including two upstream `yield return` generators, QueryCollisions and
// QueryCollisionPairs) -> eagerly-built `std::vector<T>`, matching the convention already
// established in ICollisionBroadphase2D.hpp for this whole module.
//
// `System.Data.DuplicateNameException` (thrown by AddLayer on a duplicate name) has no
// sharp-runtime equivalent -- checked and confirmed missing, not assumed; sharp-runtime is a
// sibling repository this project does not modify. Substituted with
// `System::ArgumentException`, matching real .NET BCL precedent for duplicate-key scenarios
// (Dictionary<TKey,TValue>.Add itself throws ArgumentException for a duplicate key), and
// documented here as a deliberate substitution, not a silent scope decision.
#pragma once

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/Collisions/CollisionEvent2D.hpp"
#include "CNA/Extended/Collisions/CollisionPair2D.hpp"
#include "CNA/Extended/Collisions/LayerPair.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace CNA::Extended::Collisions
{
    class ICollisionActor;
    class Layer;

    /**
     * @brief Stores collision actors in named layers and provides broadphase, narrowphase, and
     * pair-query operations.
     * @remark Low-level collision math remains in Collision2D and the bounding volume types.
     * Layer filtering is explicit: collisions are considered only for layer pairs that have been
     * enabled.
     * @see ICollisionActor, the interface your own game-object types implement to participate in
     * a CollisionWorld2D.
     * @see Collision2D for the low-level shape-vs-shape intersection/containment tests this class
     * builds broadphase/narrowphase queries on top of.
     * @code
     * #include <CNA/Extended/Collisions/CollisionWorld2D.hpp>
     * #include <CNA/Extended/Collisions/ICollisionActor.hpp>
     *
     * using CNA::Extended::BoundingBox2D;
     * using CNA::Extended::CollisionShape2D;
     * using CNA::Extended::Collisions::CollisionWorld2D;
     * using CNA::Extended::Collisions::ICollisionActor;
     * using Microsoft::Xna::Framework::Vector2;
     *
     * class BoxActor final : public ICollisionActor
     * {
     * public:
     *     BoxActor(int id, const BoundingBox2D& box) : id_(id), box_(box) {}
     *     [[nodiscard]] int getIdProperty() const override { return id_; }
     *     [[nodiscard]] CollisionShape2D getShapeProperty() const override { return CollisionShape2D(box_); }
     * private:
     *     int id_;
     *     BoundingBox2D box_;
     * };
     *
     * void FindNearbyActors(CollisionWorld2D& world)
     * {
     *     BoxActor player(1, BoundingBox2D(Vector2(0.0f, 0.0f), Vector2(32.0f, 32.0f)));
     *     world.Insert(&player); // default layer
     *
     *     const auto nearby = world.QueryCandidates(BoundingBox2D(Vector2(-16.0f, -16.0f), Vector2(48.0f, 48.0f)));
     * }
     * @endcode
     */
    class CollisionWorld2D
    {
    public:
        /** @brief The name of the default collision layer. */
        static const std::string DefaultLayerName;

        CollisionWorld2D() = default;

        /**
         * @brief Initializes a new CollisionWorld2D with a default layer.
         * @param defaultLayer The layer used for actors inserted into DefaultLayerName. Must not be null.
         */
        explicit CollisionWorld2D(std::unique_ptr<Layer> defaultLayer);

        /** @brief Gets the registered collision layers by name. */
        [[nodiscard]] const std::unordered_map<std::string, std::unique_ptr<Layer>>& getLayersProperty() const { return layers_; }

        /**
         * @brief Sets the default collision layer.
         * @param layer The layer used for actors inserted into DefaultLayerName. Must not be null.
         * @remark When a default layer is set, self-collision is enabled for that layer and
         * cross-layer collision is enabled between the default layer and every currently
         * registered layer. Adding later non-default layers enables collision between those
         * layers and the current default layer.
         */
        void SetDefaultLayer(std::unique_ptr<Layer> layer);

        /**
         * @brief Adds a named collision layer.
         * @param name The unique layer name.
         * @param layer The layer to add. Must not be null.
         * @remark New non-default layers automatically enable self-collision. If the default
         * layer already exists, collision is also enabled between the new layer and the default
         * layer.
         */
        void AddLayer(const std::string& name, std::unique_ptr<Layer> layer);

        /**
         * @brief Removes a named collision layer.
         * @param name The layer name to remove.
         * @return true if the layer was removed; otherwise, false.
         */
        bool RemoveLayer(const std::string& name);

        /** @brief Inserts an actor into the default collision layer. */
        void Insert(ICollisionActor* actor);

        /**
         * @brief Inserts an actor into the specified collision layer.
         * @param actor The actor to insert.
         * @param layerName The name of the registered layer that will contain the actor.
         */
        void Insert(ICollisionActor* actor, const std::string& layerName);

        /** @brief Returns whether the specified actor is present in this collision world. */
        [[nodiscard]] bool Contains(ICollisionActor* actor) const;

        /**
         * @brief Tries to get the name of the layer that currently contains the specified actor.
         * @param actor The actor to look up.
         * @param layerName Receives the actor's current layer name if present; otherwise cleared.
         * @return true if the actor is present in this collision world; otherwise, false.
         */
        [[nodiscard]] bool TryGetLayerName(ICollisionActor* actor, std::string& layerName) const;

        /** @brief Gets the name of the layer that currently contains the specified actor. */
        [[nodiscard]] std::string GetLayerName(ICollisionActor* actor) const;

        /** @brief Moves an actor from its current collision layer into another registered layer in this world. */
        void MoveToLayer(ICollisionActor* actor, const std::string& layerName);

        /** @brief Removes an actor from its assigned collision layer. */
        bool Remove(ICollisionActor* actor);

        /**
         * @brief Rebuilds every dynamic layer in this collision world.
         * @remark A convenience wrapper over Layer::Reset() for each registered layer.
         */
        void RebuildDynamicLayers();

        /**
         * @brief Queries one collision layer for broadphase candidates whose bounds overlap the specified area.
         * @param bounds The axis-aligned query bounds in world space.
         * @param layerName The layer name to query, or empty to query the default layer.
         */
        [[nodiscard]] std::vector<ICollisionActor*> QueryCandidates(const BoundingBox2D& bounds, const std::string& layerName = "") const;

        /**
         * @brief Queries a target layer for broadphase candidates for the specified actor after applying layer filtering.
         * @param actor The actor whose broadphase bounds are used for the query.
         * @param otherLayerName The target layer name, or empty to query the default layer.
         * @return The actors whose broadphase bounds overlap actor in the target layer when
         * collision is enabled between the two layers; otherwise, an empty sequence.
         */
        [[nodiscard]] std::vector<ICollisionActor*> QueryCandidates(ICollisionActor* actor, const std::string& otherLayerName) const;

        /**
         * @brief Queries one collision layer for actors that collide with the specified actor and
         * returns collision results relative to that actor.
         * @remark Each returned CollisionEvent2D::Result follows the receiver-relative direction
         * convention: its minimum translation vector moves actor out of the returned
         * CollisionEvent2D::Other.
         */
        [[nodiscard]] std::vector<CollisionEvent2D> QueryCollisions(ICollisionActor* actor, const std::string& otherLayerName) const;

        /**
         * @brief Queries two layers for colliding actor pairs and returns collision results when needed.
         * @remark Each unordered actor pair is returned at most once per query, even when both
         * actors are in the same layer or when broadphase storage places them in multiple
         * candidate buckets. CollisionPair2D::FirstResult moves CollisionPair2D::First out of
         * CollisionPair2D::Second.
         */
        [[nodiscard]] std::vector<CollisionPair2D> QueryCollisionPairs(const std::string& firstLayerName, const std::string& secondLayerName) const;

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
        void EnableCollisionBetweenLayers(Layer& firstLayer, Layer& secondLayer);
        [[nodiscard]] Layer& GetActorLayer(ICollisionActor* actor) const;
        [[nodiscard]] Layer& GetLayer(const std::string& layerName) const;

        std::unordered_map<std::string, std::unique_ptr<Layer>> layers_;
        std::unordered_map<ICollisionActor*, std::string> actorLayerNames_;
        std::unordered_set<LayerPair> layerCollision_;
    };
}
