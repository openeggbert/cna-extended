// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapLayerCollection.cs. This collection is the
// canonical owner of every `TilemapLayer` it holds (nothing constructs a layer except via
// `Add`, and `TilemapGroupLayer`/`TilemapTilesetCollection` only ever *reference* layers/tilesets
// owned elsewhere) -- `layers_` is an owning `std::vector<std::unique_ptr<TilemapLayer>>`;
// `layersByName_` is a non-owning `TilemapLayer*` lookup index into the same instances, using
// this project's own `Dictionary<K,V>` wrapper (fine here since `TilemapLayer*` is a trivially
// copyable value type, unlike the move-only `TilemapTileData` case in `TilemapTileset.hpp`).
#pragma once

#include "CNA/Extended/Tilemaps/TilemapLayer.hpp"
#include "System/Collections/Generic/Dictionary.hpp"

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace CNA::Extended::Tilemaps
{
    /** @brief A collection of tilemap layers, indexed by both position and name. Owns every layer it holds. */
    class TilemapLayerCollection
    {
    public:
        TilemapLayerCollection() = default;

        /** @brief Gets the layer at @p index. */
        [[nodiscard]] TilemapLayer* operator[](int index) const { return layers_[static_cast<std::size_t>(index)].get(); }

        /** @brief Gets the layer named @p name. @throws System::Collections::Generic::KeyNotFoundException no layer with that name exists. */
        [[nodiscard]] TilemapLayer* operator[](const std::string& name) const { return layersByName_[name]; }

        /** @brief Gets the number of layers in the collection. */
        [[nodiscard]] int getCountProperty() const { return static_cast<int>(layers_.size()); }

        /** @brief Adds (taking ownership of) @p layer to the collection. @throws std::invalid_argument layer is nullptr. */
        void Add(std::unique_ptr<TilemapLayer> layer);

        /** @brief Removes the layer matching @p layer (by pointer identity) from the collection (O(n)). @return true if it was found and removed. */
        bool Remove(const TilemapLayer* layer);

        /** @brief Removes all layers from the collection. */
        void Clear();

        /** @brief Attempts to get the layer named @p name. Returns false, leaving @p layer set to nullptr, if not found. */
        bool TryGetValue(const std::string& name, TilemapLayer*& layer) const;

        /** @brief Gets the layer named @p name if it exists and is of type @p T, or nullptr otherwise. */
        template <typename T>
        [[nodiscard]] T* GetLayer(const std::string& name) const
        {
            static_assert(std::is_base_of_v<TilemapLayer, T>, "T must derive from TilemapLayer.");
            TilemapLayer* layer = nullptr;
            if (TryGetValue(name, layer))
            {
                return dynamic_cast<T*>(layer);
            }
            return nullptr;
        }

        /** @brief Gets all layers of type @p T. */
        template <typename T>
        [[nodiscard]] std::vector<T*> GetLayers() const
        {
            static_assert(std::is_base_of_v<TilemapLayer, T>, "T must derive from TilemapLayer.");
            std::vector<T*> result;
            for (const std::unique_ptr<TilemapLayer>& layer : layers_)
            {
                if (auto* typed = dynamic_cast<T*>(layer.get()))
                {
                    result.push_back(typed);
                }
            }
            return result;
        }

        /** @brief Gets the index of @p layer in the collection (by pointer identity), or -1 if not found. */
        [[nodiscard]] int IndexOf(const TilemapLayer* layer) const;

        [[nodiscard]] auto begin() const { return layers_.begin(); }
        [[nodiscard]] auto end() const { return layers_.end(); }

    private:
        std::vector<std::unique_ptr<TilemapLayer>> layers_;
        System::Collections::Generic::Dictionary<std::string, TilemapLayer*> layersByName_;
    };
}
