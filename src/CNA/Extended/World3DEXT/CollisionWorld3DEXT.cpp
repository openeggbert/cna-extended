// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/CollisionWorld3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ICollisionBroadphase3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ActorPairKey3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Layer3DEXT.hpp"
#include "CNA/Extended/World3DEXT/SpatialHash3DEXT.hpp"
#include "CNA/Extended/Collisions/UndefinedLayerException.hpp"
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/String.hpp"

#include <stdexcept>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;

    const std::string CollisionWorld3DEXT::DefaultLayerName = "default";

    CollisionWorld3DEXT::CollisionWorld3DEXT() : CollisionWorld3DEXT(std::make_unique<SpatialHash3DEXT>(16.0f))
    {
    }

    CollisionWorld3DEXT::CollisionWorld3DEXT(std::unique_ptr<ICollisionBroadphase3DEXT> broadphase)
        : CollisionWorld3DEXT(std::make_unique<Layer3DEXT>(std::move(broadphase)))
    {
    }

    CollisionWorld3DEXT::CollisionWorld3DEXT(std::unique_ptr<Layer3DEXT> defaultLayer)
    {
        if (!defaultLayer)
        {
            throw std::invalid_argument("defaultLayer must not be null.");
        }
        SetDefaultLayer(std::move(defaultLayer));
    }

    CollisionWorld3DEXT::~CollisionWorld3DEXT() = default;

    void CollisionWorld3DEXT::SetDefaultLayer(std::unique_ptr<Layer3DEXT> layer)
    {
        if (!layer)
        {
            throw std::invalid_argument("layer must not be null.");
        }

        if (layers_.contains(DefaultLayerName))
        {
            RemoveLayer(DefaultLayerName);
        }

        Layer3DEXT* layerPtr = layer.get();
        layers_[DefaultLayerName] = std::move(layer);

        for (const auto& [name, otherLayer] : layers_)
        {
            EnableCollisionBetweenLayers(*layerPtr, *otherLayer);
        }
    }

    void CollisionWorld3DEXT::AddLayer(const std::string& name, std::unique_ptr<Layer3DEXT> layer)
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

        Layer3DEXT* layerPtr = layer.get();
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

    bool CollisionWorld3DEXT::RemoveLayer(const std::string& name)
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

        Layer3DEXT* layerPtr = layerIt->second.get();

        std::vector<ICollisionActor3DEXT*> actorsToRemove;
        for (const auto& [actor, layerName] : actorLayerNames_)
        {
            if (layerName == name)
            {
                actorsToRemove.push_back(actor);
            }
        }
        for (ICollisionActor3DEXT* actor : actorsToRemove)
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

    void CollisionWorld3DEXT::Insert(ICollisionActor3DEXT* actor)
    {
        Insert(actor, DefaultLayerName);
    }

    void CollisionWorld3DEXT::Insert(ICollisionActor3DEXT* actor, const std::string& layerName)
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

        Layer3DEXT& layer = GetLayer(layerName);
        layer.getSpaceProperty().Insert(actor);
        actorLayerNames_[actor] = layerName;
    }

    bool CollisionWorld3DEXT::Contains(ICollisionActor3DEXT* actor) const
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }
        return actorLayerNames_.contains(actor);
    }

    bool CollisionWorld3DEXT::TryGetLayerName(ICollisionActor3DEXT* actor, std::string& layerName) const
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

    std::string CollisionWorld3DEXT::GetLayerName(ICollisionActor3DEXT* actor) const
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

    void CollisionWorld3DEXT::MoveToLayer(ICollisionActor3DEXT* actor, const std::string& layerName)
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

        Layer3DEXT& currentLayer = GetLayer(currentLayerName);
        Layer3DEXT& targetLayer = GetLayer(layerName);
        currentLayer.getSpaceProperty().Remove(actor);
        targetLayer.getSpaceProperty().Insert(actor);
        actorLayerNames_[actor] = layerName;
    }

    bool CollisionWorld3DEXT::Remove(ICollisionActor3DEXT* actor)
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

        Layer3DEXT& layer = GetLayer(it->second);
        actorLayerNames_.erase(it);
        return layer.getSpaceProperty().Remove(actor);
    }

    void CollisionWorld3DEXT::RebuildDynamicLayers()
    {
        for (const auto& [name, layer] : layers_)
        {
            layer->Reset();
        }
    }

    std::vector<ICollisionActor3DEXT*> CollisionWorld3DEXT::QueryCandidates(const BoundingBox& bounds, const std::string& layerName) const
    {
        Layer3DEXT& layer = GetLayer(layerName);
        return layer.getSpaceProperty().Query(bounds);
    }

    std::vector<ICollisionActor3DEXT*> CollisionWorld3DEXT::QueryCandidates(ICollisionActor3DEXT* actor, const std::string& otherLayerName) const
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }

        Layer3DEXT& actorLayer = GetActorLayer(actor);
        Layer3DEXT& otherLayer = GetLayer(otherLayerName);

        if (!layerCollision_.contains(LayerPair3DEXT(actorLayer, otherLayer)))
        {
            return {};
        }

        return otherLayer.getSpaceProperty().Query(actor->getShapeProperty().getBoundingBoxProperty());
    }

    std::vector<CollisionEvent3DEXT> CollisionWorld3DEXT::QueryCollisions(ICollisionActor3DEXT* actor, const std::string& otherLayerName) const
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }

        std::vector<CollisionEvent3DEXT> results;
        for (ICollisionActor3DEXT* candidate : QueryCandidates(actor, otherLayerName))
        {
            if (actor == candidate)
            {
                continue;
            }

            CollisionResult3DEXT result;
            if (!actor->getShapeProperty().TryGetCollision(candidate->getShapeProperty(), result))
            {
                continue;
            }

            results.emplace_back(*candidate, result);
        }
        return results;
    }

    std::vector<CollisionPair3DEXT> CollisionWorld3DEXT::QueryCollisionPairs(const std::string& firstLayerName, const std::string& secondLayerName) const
    {
        Layer3DEXT& firstLayer = GetLayer(firstLayerName);
        Layer3DEXT& secondLayer = GetLayer(secondLayerName);

        std::vector<CollisionPair3DEXT> results;
        if (!layerCollision_.contains(LayerPair3DEXT(firstLayer, secondLayer)))
        {
            return results;
        }

        std::unordered_set<ActorPairKey3DEXT> processedPairs;

        for (ICollisionActor3DEXT* actor : firstLayer.getSpaceProperty().GetActors())
        {
            for (ICollisionActor3DEXT* candidate : secondLayer.getSpaceProperty().Query(actor->getShapeProperty().getBoundingBoxProperty()))
            {
                if (actor == candidate)
                {
                    continue;
                }

                CollisionResult3DEXT result;
                if (!actor->getShapeProperty().TryGetCollision(candidate->getShapeProperty(), result))
                {
                    continue;
                }

                ActorPairKey3DEXT actorPair(*actor, *candidate);
                if (!processedPairs.insert(actorPair).second)
                {
                    continue;
                }

                results.emplace_back(*actor, *candidate, result);
            }
        }
        return results;
    }

    void CollisionWorld3DEXT::EnableCollisionBetweenLayers(const std::string& firstLayerName, const std::string& secondLayerName)
    {
        EnableCollisionBetweenLayers(GetLayer(firstLayerName), GetLayer(secondLayerName));
    }

    void CollisionWorld3DEXT::DisableCollisionBetweenLayers(const std::string& firstLayerName, const std::string& secondLayerName)
    {
        layerCollision_.erase(LayerPair3DEXT(GetLayer(firstLayerName), GetLayer(secondLayerName)));
    }

    bool CollisionWorld3DEXT::IsCollisionEnabledBetweenLayers(const std::string& firstLayerName, const std::string& secondLayerName) const
    {
        return layerCollision_.contains(LayerPair3DEXT(GetLayer(firstLayerName), GetLayer(secondLayerName)));
    }

    void CollisionWorld3DEXT::EnableCollisionBetweenLayers(Layer3DEXT& firstLayer, Layer3DEXT& secondLayer)
    {
        layerCollision_.insert(LayerPair3DEXT(firstLayer, secondLayer));
    }

    Layer3DEXT& CollisionWorld3DEXT::GetActorLayer(ICollisionActor3DEXT* actor) const
    {
        const auto it = actorLayerNames_.find(actor);
        if (it == actorLayerNames_.end())
        {
            throw System::InvalidOperationException("The actor is not present in this collision world.");
        }

        return GetLayer(it->second);
    }

    Layer3DEXT& CollisionWorld3DEXT::GetLayer(const std::string& layerName) const
    {
        const std::string& resolvedLayerName = layerName.empty() ? DefaultLayerName : layerName;

        const auto it = layers_.find(resolvedLayerName);
        if (it == layers_.end())
        {
            throw Collisions::UndefinedLayerException(resolvedLayerName);
        }

        return *it->second;
    }
}
