// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapLayerCollection.hpp"

#include <algorithm>
#include <stdexcept>

namespace CNA::Extended::Tilemaps
{
    void TilemapLayerCollection::Add(std::unique_ptr<TilemapLayer> layer)
    {
        if (layer == nullptr)
        {
            throw std::invalid_argument("layer must not be null.");
        }

        TilemapLayer* raw = layer.get();
        layers_.push_back(std::move(layer));
        layersByName_[raw->getNameProperty()] = raw;
    }

    bool TilemapLayerCollection::Remove(const TilemapLayer* layer)
    {
        if (layer == nullptr)
        {
            return false;
        }

        const auto it = std::find_if(
            layers_.begin(), layers_.end(), [layer](const std::unique_ptr<TilemapLayer>& candidate) { return candidate.get() == layer; });
        if (it == layers_.end())
        {
            return false;
        }

        layersByName_.Remove((*it)->getNameProperty());
        layers_.erase(it);
        return true;
    }

    void TilemapLayerCollection::Clear()
    {
        layers_.clear();
        layersByName_.Clear();
    }

    bool TilemapLayerCollection::TryGetValue(const std::string& name, TilemapLayer*& layer) const
    {
        return layersByName_.TryGetValue(name, layer);
    }

    int TilemapLayerCollection::IndexOf(const TilemapLayer* layer) const
    {
        for (std::size_t i = 0; i < layers_.size(); ++i)
        {
            if (layers_[i].get() == layer)
            {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
}
