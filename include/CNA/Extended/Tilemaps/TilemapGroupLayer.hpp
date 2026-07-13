// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapLayers/TilemapGroupLayer.cs. `List<TilemapLayer>
// _childLayers` holds C# references handed in via `AddLayer` -- child layers are owned by
// whatever owns the overall layer collection (e.g. `TilemapLayerCollection`), not by this group
// -- translated as a non-owning `std::vector<TilemapLayer*>`, matching
// `TilemapTilesetCollection`'s established non-owning-reference-collection convention.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapLayer.hpp"

#include <vector>

namespace CNA::Extended::Tilemaps
{
    /** @brief A layer that contains other layers as children. */
    class TilemapGroupLayer : public TilemapLayer
    {
    public:
        /** @brief Creates an empty group layer with @p name. */
        explicit TilemapGroupLayer(std::string name) : TilemapLayer(std::move(name)) {}

        /** @brief Gets the collection of child layers (non-owning). */
        [[nodiscard]] const std::vector<TilemapLayer*>& getChildLayersProperty() const { return childLayers_; }

        /** @brief Adds @p layer (referenced, not owned) as a child of this group. */
        void AddLayer(TilemapLayer* layer) { childLayers_.push_back(layer); }

        /** @brief Removes @p layer from this group's direct children. @return true if it was found and removed. */
        bool RemoveLayer(TilemapLayer* layer);

        /** @brief Finds the child layer (searched recursively through descendants) with @p name, or nullptr if not found or @p name is empty. */
        [[nodiscard]] TilemapLayer* GetLayer(const std::string& name) const;

        [[nodiscard]] Rectangle getBoundsProperty() const override;

    private:
        std::vector<TilemapLayer*> childLayers_;
    };
}
