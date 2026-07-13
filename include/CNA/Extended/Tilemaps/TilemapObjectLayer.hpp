// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapLayers/TilemapObjectLayer.cs. Unlike
// `TilemapGroupLayer`'s child-layer references or `TilemapTilesetCollection`'s tileset
// references, nothing else in this data model references a specific placed `TilemapObject`
// instance once it's added to an object layer -- this layer is each object's sole, canonical
// owner, so `_objects` is translated as an owning `std::vector<std::unique_ptr<TilemapObject>>`
// (contrast with `TilemapTileData::CollisionObjects`, which is a *different*, also-owning
// collection of per-tile collision shapes -- the two are unrelated despite the shared element
// type). `IEnumerable<T> GetObjectsInRegion`/`GetObjects<T>` (C# generator methods using `yield
// return`) become eagerly-evaluated `std::vector` results, matching this project's established
// convention for LINQ/generator-style query methods.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapObjectDrawOrder.hpp"

#include <memory>
#include <type_traits>
#include <vector>

namespace CNA::Extended::Tilemaps
{
    /** @brief A layer containing drawable objects (points, shapes, tiles, text). */
    class TilemapObjectLayer : public TilemapLayer
    {
    public:
        /** @brief Creates an empty object layer with @p name. DrawOrder defaults to TopDown. */
        explicit TilemapObjectLayer(std::string name);

        /** @brief Gets the collection of objects in this layer. */
        [[nodiscard]] const std::vector<std::unique_ptr<TilemapObject>>& getObjectsProperty() const { return objects_; }

        /** @brief Gets/sets the rendering order for objects in this layer. */
        [[nodiscard]] TilemapObjectDrawOrder getDrawOrderProperty() const { return drawOrder_; }
        void setDrawOrderProperty(TilemapObjectDrawOrder value) { drawOrder_ = value; }

        [[nodiscard]] Rectangle getBoundsProperty() const override;

        /** @brief Adds (taking ownership of) @p obj to the layer. */
        void AddObject(std::unique_ptr<TilemapObject> obj) { objects_.push_back(std::move(obj)); }

        /** @brief Removes the object matching @p obj (by pointer identity) from the layer. @return true if it was found and removed. */
        bool RemoveObject(const TilemapObject* obj);

        /** @brief Gets the object with @p id, or nullptr if not found. */
        [[nodiscard]] TilemapObject* GetObject(int id) const;

        /** @brief Gets all objects whose bounds intersect @p region. */
        [[nodiscard]] std::vector<TilemapObject*> GetObjectsInRegion(const BoundingBox2D& region) const;

        /** @brief Gets all objects of type @p T (must derive from TilemapObject). */
        template <typename T>
        [[nodiscard]] std::vector<T*> GetObjects() const
        {
            static_assert(std::is_base_of_v<TilemapObject, T>, "T must derive from TilemapObject.");
            std::vector<T*> result;
            for (const std::unique_ptr<TilemapObject>& obj : objects_)
            {
                if (auto* typedObj = dynamic_cast<T*>(obj.get()))
                {
                    result.push_back(typedObj);
                }
            }
            return result;
        }

    private:
        std::vector<std::unique_ptr<TilemapObject>> objects_;
        TilemapObjectDrawOrder drawOrder_ = TilemapObjectDrawOrder::TopDown;
    };
}
