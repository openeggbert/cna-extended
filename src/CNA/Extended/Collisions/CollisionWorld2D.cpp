// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collisions/CollisionWorld2D.hpp"

#include "CNA/Extended/CollisionShape2D.hpp"
#include "CNA/Extended/Collisions/ActorPairKey.hpp"
#include "CNA/Extended/Collisions/ICollisionActor.hpp"
#include "CNA/Extended/Collisions/ICollisionBroadphase2D.hpp"
#include "CNA/Extended/Collisions/Layer.hpp"
#include "CNA/Extended/Collisions/UndefinedLayerException.hpp"
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/String.hpp"

#include <stdexcept>

namespace CNA::Extended::Collisions
{
    const std::string CollisionWorld2D::DefaultLayerName = "default";

    CollisionWorld2D::CollisionWorld2D(std::unique_ptr<Layer> defaultLayer)
    {
        if (!defaultLayer)
        {
            throw std::invalid_argument("defaultLayer must not be null.");
        }
        SetDefaultLayer(std::move(defaultLayer));
    }

    void CollisionWorld2D::SetDefaultLayer(std::unique_ptr<Layer> layer)
    {
        if (!layer)
        {
            throw std::invalid_argument("layer must not be null.");
        }

        if (layers_.contains(DefaultLayerName))
        {
            RemoveLayer(DefaultLayerName);
        }

        Layer* layerPtr = layer.get();
        layers_[DefaultLayerName] = std::move(layer);

        for (const auto& [name, otherLayer] : layers_)
        {
            EnableCollisionBetweenLayers(*layerPtr, *otherLayer);
        }
    }

    void CollisionWorld2D::AddLayer(const std::string& name, std::unique_ptr<Layer> layer)
    {
        System::ArgumentException::ThrowIfNullOrWhiteSpace(name, "name");
        if (!layer)
        {
            throw std::invalid_argument("layer must not be null.");
        }

        if (layers_.contains(name))
        {
            throw System::ArgumentException("A layer named '" + name + "' is already registered.", "name");
        }

        Layer* layerPtr = layer.get();
        layers_[name] = std::move(layer);

        if (name != DefaultLayerName)
        {
            EnableCollisionBetweenLayers(*layerPtr, *layerPtr);

            const auto defaultIt = layers_.find(DefaultLayerName);
            if (defaultIt != layers_.end())
            {
                EnableCollisionBetweenLayers(*defaultIt->second, *layerPtr);
            }
        }
    }

    bool CollisionWorld2D::RemoveLayer(const std::string& name)
    {
        if (System::String::IsNullOrWhiteSpace(name))
        {
            throw System::ArgumentException("Layer name cannot be null or whitespace.", "name");
        }

        const auto layerIt = layers_.find(name);
        if (layerIt == layers_.end())
        {
            return false;
        }

        Layer* layerPtr = layerIt->second.get();

        std::vector<ICollisionActor*> actorsToRemove;
        for (const auto& [actor, layerName] : actorLayerNames_)
        {
            if (layerName == name)
            {
                actorsToRemove.push_back(actor);
            }
        }
        for (ICollisionActor* actor : actorsToRemove)
        {
            actorLayerNames_.erase(actor);
        }

        for (auto it = layerCollision_.begin(); it != layerCollision_.end();)
        {
            if (it->First == layerPtr || it->Second == layerPtr)
            {
                it = layerCollision_.erase(it);
            }
            else
            {
                ++it;
            }
        }

        layers_.erase(layerIt);
        return true;
    }

    void CollisionWorld2D::Insert(ICollisionActor* actor)
    {
        Insert(actor, DefaultLayerName);
    }

    void CollisionWorld2D::Insert(ICollisionActor* actor, const std::string& layerName)
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }
        System::ArgumentException::ThrowIfNullOrWhiteSpace(layerName, "layerName");

        if (actorLayerNames_.contains(actor))
        {
            throw System::InvalidOperationException("The actor is already present in this collision world.");
        }

        Layer& layer = GetLayer(layerName);
        layer.getSpaceProperty().Insert(actor);
        actorLayerNames_[actor] = layerName;
    }

    bool CollisionWorld2D::Contains(ICollisionActor* actor) const
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }
        return actorLayerNames_.contains(actor);
    }

    bool CollisionWorld2D::TryGetLayerName(ICollisionActor* actor, std::string& layerName) const
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }

        const auto it = actorLayerNames_.find(actor);
        if (it == actorLayerNames_.end())
        {
            layerName.clear();
            return false;
        }

        layerName = it->second;
        return true;
    }

    std::string CollisionWorld2D::GetLayerName(ICollisionActor* actor) const
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }

        const auto it = actorLayerNames_.find(actor);
        if (it == actorLayerNames_.end())
        {
            throw System::InvalidOperationException("The actor is not present in this collision world.");
        }

        return it->second;
    }

    void CollisionWorld2D::MoveToLayer(ICollisionActor* actor, const std::string& layerName)
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }
        System::ArgumentException::ThrowIfNullOrWhiteSpace(layerName, "layerName");

        const std::string currentLayerName = GetLayerName(actor);
        if (currentLayerName == layerName)
        {
            return;
        }

        Layer& currentLayer = GetLayer(currentLayerName);
        Layer& targetLayer = GetLayer(layerName);
        currentLayer.getSpaceProperty().Remove(actor);
        targetLayer.getSpaceProperty().Insert(actor);
        actorLayerNames_[actor] = layerName;
    }

    bool CollisionWorld2D::Remove(ICollisionActor* actor)
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }

        const auto it = actorLayerNames_.find(actor);
        if (it == actorLayerNames_.end())
        {
            return false;
        }

        Layer& layer = GetLayer(it->second);
        actorLayerNames_.erase(it);
        return layer.getSpaceProperty().Remove(actor);
    }

    void CollisionWorld2D::RebuildDynamicLayers()
    {
        for (const auto& [name, layer] : layers_)
        {
            layer->Reset();
        }
    }

    std::vector<ICollisionActor*> CollisionWorld2D::QueryCandidates(const BoundingBox2D& bounds, const std::string& layerName) const
    {
        Layer& layer = GetLayer(layerName);
        return layer.getSpaceProperty().Query(bounds);
    }

    std::vector<ICollisionActor*> CollisionWorld2D::QueryCandidates(ICollisionActor* actor, const std::string& otherLayerName) const
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }

        Layer& actorLayer = GetActorLayer(actor);
        Layer& otherLayer = GetLayer(otherLayerName);

        if (!layerCollision_.contains(LayerPair(actorLayer, otherLayer)))
        {
            return {};
        }

        return otherLayer.getSpaceProperty().Query(actor->getShapeProperty().getBoundingBoxProperty());
    }

    std::vector<CollisionEvent2D> CollisionWorld2D::QueryCollisions(ICollisionActor* actor, const std::string& otherLayerName) const
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }

        std::vector<CollisionEvent2D> results;
        for (ICollisionActor* candidate : QueryCandidates(actor, otherLayerName))
        {
            if (actor == candidate)
            {
                continue;
            }

            CollisionResult2D result;
            if (!actor->getShapeProperty().TryGetCollision(candidate->getShapeProperty(), result))
            {
                continue;
            }

            results.emplace_back(*candidate, result);
        }
        return results;
    }

    std::vector<CollisionPair2D> CollisionWorld2D::QueryCollisionPairs(const std::string& firstLayerName, const std::string& secondLayerName) const
    {
        Layer& firstLayer = GetLayer(firstLayerName);
        Layer& secondLayer = GetLayer(secondLayerName);

        std::vector<CollisionPair2D> results;
        if (!layerCollision_.contains(LayerPair(firstLayer, secondLayer)))
        {
            return results;
        }

        std::unordered_set<ActorPairKey> processedPairs;

        for (ICollisionActor* actor : firstLayer.getSpaceProperty().GetActors())
        {
            for (ICollisionActor* candidate : secondLayer.getSpaceProperty().Query(actor->getShapeProperty().getBoundingBoxProperty()))
            {
                if (actor == candidate)
                {
                    continue;
                }

                CollisionResult2D result;
                if (!actor->getShapeProperty().TryGetCollision(candidate->getShapeProperty(), result))
                {
                    continue;
                }

                ActorPairKey actorPair(*actor, *candidate);
                if (!processedPairs.insert(actorPair).second)
                {
                    continue;
                }

                results.emplace_back(*actor, *candidate, result);
            }
        }
        return results;
    }

    void CollisionWorld2D::EnableCollisionBetweenLayers(const std::string& firstLayerName, const std::string& secondLayerName)
    {
        EnableCollisionBetweenLayers(GetLayer(firstLayerName), GetLayer(secondLayerName));
    }

    void CollisionWorld2D::DisableCollisionBetweenLayers(const std::string& firstLayerName, const std::string& secondLayerName)
    {
        layerCollision_.erase(LayerPair(GetLayer(firstLayerName), GetLayer(secondLayerName)));
    }

    bool CollisionWorld2D::IsCollisionEnabledBetweenLayers(const std::string& firstLayerName, const std::string& secondLayerName) const
    {
        return layerCollision_.contains(LayerPair(GetLayer(firstLayerName), GetLayer(secondLayerName)));
    }

    void CollisionWorld2D::EnableCollisionBetweenLayers(Layer& firstLayer, Layer& secondLayer)
    {
        layerCollision_.insert(LayerPair(firstLayer, secondLayer));
    }

    Layer& CollisionWorld2D::GetActorLayer(ICollisionActor* actor) const
    {
        const auto it = actorLayerNames_.find(actor);
        if (it == actorLayerNames_.end())
        {
            throw System::InvalidOperationException("The actor is not present in this collision world.");
        }

        return GetLayer(it->second);
    }

    Layer& CollisionWorld2D::GetLayer(const std::string& layerName) const
    {
        const std::string& resolvedLayerName = layerName.empty() ? DefaultLayerName : layerName;

        const auto it = layers_.find(resolvedLayerName);
        if (it == layers_.end())
        {
            throw UndefinedLayerException(resolvedLayerName);
        }

        return *it->second;
    }
}
