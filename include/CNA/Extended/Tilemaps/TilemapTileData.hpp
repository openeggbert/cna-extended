// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapTileData.cs. `List<TilemapObject>
// CollisionObjects` holds polymorphic instances with nothing else in this module aliasing
// them once created here -- translated as an owning `std::vector<std::unique_ptr<TilemapObject>>`.
// `TilemapTileAnimation Animation` (nullable: most tiles aren't animated) -> an owning, nullable
// `std::unique_ptr<TilemapTileAnimation>` for the same reason. `Texture2D CustomImage` (nullable,
// externally-owned GPU resource) -> a non-owning, nullable `Texture2D*`, matching this project's
// established convention for externally-owned GPU resources (e.g. Texture2DRegion's `Texture2D*`).
#pragma once

#include "CNA/Extended/Tilemaps/TilemapObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapProperties.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileAnimation.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    /** @brief Metadata and properties for a specific tile in a tileset: class, probability, custom properties, animation, collision shapes, and an optional custom image. */
    class TilemapTileData
    {
    public:
        /** @brief Creates tile data for local tile ID @p localId. Probability defaults to 1.0. */
        explicit TilemapTileData(int localId);

        /** @brief Gets the local tile ID within the tileset. */
        [[nodiscard]] int getLocalIdProperty() const { return localId_; }

        /** @brief Gets/sets the class/type identifier for the tile. */
        [[nodiscard]] const std::string& getClassProperty() const { return class_; }
        void setClassProperty(std::string value) { class_ = std::move(value); }

        /** @brief Gets/sets the probability of this tile being used in random tile generation, in [0, 1] (1.0 default). */
        [[nodiscard]] float getProbabilityProperty() const { return probability_; }
        void setProbabilityProperty(float value) { probability_ = value; }

        /** @brief Gets the custom properties of the tile. */
        [[nodiscard]] TilemapProperties& getPropertiesProperty() { return properties_; }
        [[nodiscard]] const TilemapProperties& getPropertiesProperty() const { return properties_; }

        /** @brief Gets the animation data for this tile, or nullptr if not animated. */
        [[nodiscard]] TilemapTileAnimation* getAnimationProperty() const { return animation_.get(); }
        /** @brief Sets (and takes ownership of) the animation data for this tile. Pass nullptr to clear it. */
        void setAnimationProperty(std::unique_ptr<TilemapTileAnimation> value) { animation_ = std::move(value); }

        /** @brief Gets the collision objects associated with this tile. */
        [[nodiscard]] std::vector<std::unique_ptr<TilemapObject>>& getCollisionObjectsProperty() { return collisionObjects_; }
        [[nodiscard]] const std::vector<std::unique_ptr<TilemapObject>>& getCollisionObjectsProperty() const { return collisionObjects_; }

        /** @brief Gets/sets a custom image texture overriding the tileset's atlas texture for this specific tile, or nullptr for none. Non-owning: the texture is externally owned. */
        [[nodiscard]] Texture2D* getCustomImageProperty() const { return customImage_; }
        void setCustomImageProperty(Texture2D* value) { customImage_ = value; }

    private:
        int localId_;
        std::string class_;
        float probability_ = 1.0f;
        TilemapProperties properties_;
        std::unique_ptr<TilemapTileAnimation> animation_;
        std::vector<std::unique_ptr<TilemapObject>> collisionObjects_;
        Texture2D* customImage_ = nullptr;
    };
}
