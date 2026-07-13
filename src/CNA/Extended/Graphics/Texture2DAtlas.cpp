// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/Texture2DAtlas.hpp"

#include "CNA/Extended/Graphics/Sprite.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/ObjectDisposedException.hpp"

#include <algorithm>
#include <stdexcept>

namespace CNA::Extended::Graphics
{
    Texture2DAtlas::Texture2DAtlas(Texture2D* texture) : Texture2DAtlas("", texture)
    {
    }

    Texture2DAtlas::Texture2DAtlas(const std::string& name, Texture2D* texture)
    {
        if (texture == nullptr)
        {
            throw std::invalid_argument("texture must not be null.");
        }
        System::ObjectDisposedException::ThrowIf(texture->getIsDisposedProperty(), "texture");

        name_ = name.empty() ? (texture->getNameProperty() + "Atlas") : name;
        texture_ = texture;
    }

    std::shared_ptr<Texture2DRegion> Texture2DAtlas::CreateRegion(int x, int y, int width, int height)
    {
        return CreateRegion(Rectangle(x, y, width, height), "");
    }

    std::shared_ptr<Texture2DRegion> Texture2DAtlas::CreateRegion(int x, int y, int width, int height, const std::string& name)
    {
        return CreateRegion(Rectangle(x, y, width, height), name);
    }

    std::shared_ptr<Texture2DRegion> Texture2DAtlas::CreateRegion(const Point& location, const Size& size)
    {
        return CreateRegion(Rectangle(location.X, location.Y, size.Width, size.Height), "");
    }

    std::shared_ptr<Texture2DRegion> Texture2DAtlas::CreateRegion(const std::string& name, const Point& location, const Size& size)
    {
        return CreateRegion(Rectangle(location.X, location.Y, size.Width, size.Height), name);
    }

    std::shared_ptr<Texture2DRegion> Texture2DAtlas::CreateRegion(const Rectangle& bounds)
    {
        return CreateRegion(bounds, "");
    }

    std::shared_ptr<Texture2DRegion> Texture2DAtlas::CreateRegion(const Rectangle& bounds, const std::string& name)
    {
        auto region = std::make_shared<Texture2DRegion>(texture_, bounds, name);
        AddRegion(region);
        return region;
    }

    std::shared_ptr<Texture2DRegion> Texture2DAtlas::CreateRegion(const Rectangle& bounds, bool isRotated,
        const Size& originalSize, const Vector2& trimOffset, const std::optional<Vector2>& originNormalized, const std::string& name)
    {
        auto region = std::make_shared<Texture2DRegion>(texture_, bounds.X, bounds.Y, bounds.Width, bounds.Height,
            isRotated, originalSize, trimOffset, originNormalized, name);
        AddRegion(region);
        return region;
    }

    int Texture2DAtlas::GetIndexOfRegion(const std::string& name) const
    {
        for (std::size_t i = 0; i < regionsByIndex_.size(); ++i)
        {
            if (regionsByIndex_[i]->getNameProperty() == name)
            {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    std::shared_ptr<Texture2DRegion> Texture2DAtlas::GetRegion(int index) const
    {
        return regionsByIndex_.at(static_cast<std::size_t>(index));
    }

    bool Texture2DAtlas::TryGetRegion(int index, std::shared_ptr<Texture2DRegion>& region) const
    {
        region = nullptr;
        if (index < 0 || static_cast<std::size_t>(index) >= regionsByIndex_.size())
        {
            return false;
        }
        region = regionsByIndex_[static_cast<std::size_t>(index)];
        return true;
    }

    std::vector<std::shared_ptr<Texture2DRegion>> Texture2DAtlas::GetRegions(const std::vector<int>& indexes) const
    {
        std::vector<std::shared_ptr<Texture2DRegion>> regions(indexes.size());
        for (std::size_t i = 0; i < indexes.size(); ++i)
        {
            regions[i] = GetRegion(indexes[i]);
        }
        return regions;
    }

    std::vector<std::shared_ptr<Texture2DRegion>> Texture2DAtlas::GetRegions(const std::vector<std::string>& names) const
    {
        std::vector<std::shared_ptr<Texture2DRegion>> regions(names.size());
        for (std::size_t i = 0; i < names.size(); ++i)
        {
            regions[i] = GetRegion(names[i]);
        }
        return regions;
    }

    bool Texture2DAtlas::RemoveRegion(int index)
    {
        std::shared_ptr<Texture2DRegion> region;
        if (TryGetRegion(index, region))
        {
            return RemoveRegionInternal(region);
        }
        return false;
    }

    bool Texture2DAtlas::RemoveRegion(const std::string& name)
    {
        std::shared_ptr<Texture2DRegion> region;
        if (TryGetRegion(name, region))
        {
            return RemoveRegionInternal(region);
        }
        return false;
    }

    void Texture2DAtlas::ClearRegions()
    {
        regionsByIndex_.clear();
        regionsByName_.Clear();
    }

    void Texture2DAtlas::AddRegion(const std::shared_ptr<Texture2DRegion>& region)
    {
        if (regionsByName_.ContainsKey(region->getNameProperty()))
        {
            throw std::invalid_argument(
                "This Texture2DAtlas already contains a Texture2DRegion with the name '" + region->getNameProperty() + "'");
        }

        regionsByIndex_.push_back(region);
        regionsByName_.Add(region->getNameProperty(), region);
    }

    bool Texture2DAtlas::RemoveRegionInternal(const std::shared_ptr<Texture2DRegion>& region)
    {
        const auto it = std::find(regionsByIndex_.begin(), regionsByIndex_.end(), region);
        if (it == regionsByIndex_.end())
        {
            return false;
        }
        regionsByIndex_.erase(it);
        return regionsByName_.Remove(region->getNameProperty());
    }

    Sprite Texture2DAtlas::CreateSprite(int regionIndex) const
    {
        return Sprite(GetRegion(regionIndex));
    }

    Sprite Texture2DAtlas::CreateSprite(const std::string& regionName) const
    {
        return Sprite(GetRegion(regionName));
    }

    std::vector<Texture2DAtlas::CalculatedRegion> Texture2DAtlas::CalculateRegions(const std::string& atlasName,
        int textureWidth, int textureHeight, int regionWidth, int regionHeight, int maxRegionCount, int margin, int spacing)
    {
        const int width = textureWidth - margin;
        const int height = textureHeight - margin;
        const int xIncrement = regionWidth + spacing;
        const int yIncrement = regionHeight + spacing;

        const int columns = (width - margin + spacing) / xIncrement;
        const int rows = (height - margin + spacing) / yIncrement;
        const int totalRegions = columns * rows;

        // We know what the final size of the collection will be so calculate it
        // and use it to prevent reallocations as items are added to the list
        std::vector<CalculatedRegion> regions;
        regions.reserve(static_cast<std::size_t>(std::min(totalRegions, maxRegionCount)));

        for (int i = 0; i < totalRegions; ++i)
        {
            const int x = margin + (i % columns) * xIncrement;
            const int y = margin + (i / columns) * yIncrement;

            if (x >= width || y >= height)
            {
                break;
            }

            regions.push_back(CalculatedRegion{Rectangle(x, y, regionWidth, regionHeight), atlasName + "_" + std::to_string(i)});

            if (static_cast<int>(regions.size()) >= maxRegionCount)
            {
                break;
            }
        }

        return regions;
    }

    Texture2DAtlas Texture2DAtlas::Create(const std::string& name, Texture2D* texture, int regionWidth, int regionHeight,
        int maxRegionCount, int margin, int spacing)
    {
        if (texture == nullptr)
        {
            throw std::invalid_argument("texture must not be null.");
        }

        const std::vector<CalculatedRegion> regions = CalculateRegions(
            name, texture->getWidthProperty(), texture->getHeightProperty(), regionWidth, regionHeight, maxRegionCount, margin, spacing);

        Texture2DAtlas textureAtlas(name, texture);
        for (const CalculatedRegion& region : regions)
        {
            textureAtlas.CreateRegion(region.bounds, region.name);
        }

        return textureAtlas;
    }
}
