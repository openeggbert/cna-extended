// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapObjectLayer.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

namespace CNA::Extended::Tilemaps
{
    TilemapObjectLayer::TilemapObjectLayer(std::string name) : TilemapLayer(std::move(name))
    {
    }

    bool TilemapObjectLayer::RemoveObject(const TilemapObject* obj)
    {
        const auto it = std::find_if(
            objects_.begin(), objects_.end(), [obj](const std::unique_ptr<TilemapObject>& candidate) { return candidate.get() == obj; });
        if (it == objects_.end())
        {
            return false;
        }
        objects_.erase(it);
        return true;
    }

    TilemapObject* TilemapObjectLayer::GetObject(int id) const
    {
        for (const std::unique_ptr<TilemapObject>& obj : objects_)
        {
            if (obj->getIdProperty() == id)
            {
                return obj.get();
            }
        }
        return nullptr;
    }

    std::vector<TilemapObject*> TilemapObjectLayer::GetObjectsInRegion(const BoundingBox2D& region) const
    {
        std::vector<TilemapObject*> result;
        for (const std::unique_ptr<TilemapObject>& obj : objects_)
        {
            if (region.Intersects(obj->getBoundsProperty()))
            {
                result.push_back(obj.get());
            }
        }
        return result;
    }

    Rectangle TilemapObjectLayer::getBoundsProperty() const
    {
        if (objects_.empty())
        {
            return Rectangle::Empty;
        }

        std::optional<BoundingBox2D> unionBounds;

        for (const std::unique_ptr<TilemapObject>& obj : objects_)
        {
            const BoundingBox2D objBounds = obj->getBoundsProperty();

            if (!unionBounds.has_value())
            {
                unionBounds = objBounds;
            }
            else
            {
                unionBounds = BoundingBox2D::CreateMerged(*unionBounds, objBounds);
            }
        }

        const BoundingBox2D bounds = *unionBounds;

        return Rectangle(static_cast<int>(bounds.Min.X), static_cast<int>(bounds.Min.Y),
            static_cast<int>(std::ceil(bounds.Max.X - bounds.Min.X)), static_cast<int>(std::ceil(bounds.Max.Y - bounds.Min.Y)));
    }
}
