// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapGroupLayer.hpp"

#include <algorithm>

namespace CNA::Extended::Tilemaps
{
    bool TilemapGroupLayer::RemoveLayer(TilemapLayer* layer)
    {
        const auto it = std::find(childLayers_.begin(), childLayers_.end(), layer);
        if (it == childLayers_.end())
        {
            return false;
        }
        childLayers_.erase(it);
        return true;
    }

    TilemapLayer* TilemapGroupLayer::GetLayer(const std::string& name) const
    {
        if (name.empty())
        {
            return nullptr;
        }

        for (TilemapLayer* layer : childLayers_)
        {
            if (layer->getNameProperty() == name)
            {
                return layer;
            }
        }

        for (TilemapLayer* layer : childLayers_)
        {
            if (auto* groupLayer = dynamic_cast<TilemapGroupLayer*>(layer))
            {
                TilemapLayer* found = groupLayer->GetLayer(name);
                if (found != nullptr)
                {
                    return found;
                }
            }
        }

        return nullptr;
    }

    Rectangle TilemapGroupLayer::getBoundsProperty() const
    {
        if (childLayers_.empty())
        {
            return Rectangle::Empty;
        }

        Rectangle bounds = childLayers_[0]->getBoundsProperty();

        for (std::size_t i = 1; i < childLayers_.size(); ++i)
        {
            bounds = Rectangle::Union(bounds, childLayers_[i]->getBoundsProperty());
        }

        return bounds;
    }
}
