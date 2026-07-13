// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapLayers/TilemapLayer.cs. `Color? TintColor` ->
// `std::optional<Color>`.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapProperties.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <optional>
#include <string>

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;

    /** @brief Abstract base for all tilemap layers (tile, object, image, group). */
    class TilemapLayer
    {
    public:
        virtual ~TilemapLayer() = default;

        /** @brief Gets the name of the layer. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }

        /** @brief Gets/sets the class/type identifier for this layer. */
        [[nodiscard]] const std::string& getClassProperty() const { return class_; }
        void setClassProperty(std::string value) { class_ = std::move(value); }

        /** @brief Gets/sets whether the layer is visible. */
        [[nodiscard]] bool getIsVisibleProperty() const { return isVisible_; }
        void setIsVisibleProperty(bool value) { isVisible_ = value; }

        /** @brief Gets/sets the opacity of the layer, in [0.0, 1.0]. */
        [[nodiscard]] float getOpacityProperty() const { return opacity_; }
        void setOpacityProperty(float value) { opacity_ = value; }

        /** @brief Gets/sets the tint color applied to the layer, or std::nullopt for none. */
        [[nodiscard]] const std::optional<Color>& getTintColorProperty() const { return tintColor_; }
        void setTintColorProperty(const std::optional<Color>& value) { tintColor_ = value; }

        /** @brief Gets/sets the rendering offset for the layer. */
        [[nodiscard]] const Vector2& getOffsetProperty() const { return offset_; }
        void setOffsetProperty(const Vector2& value) { offset_ = value; }

        /** @brief Gets/sets the parallax scrolling factor for this layer; (1,1) is normal scrolling. */
        [[nodiscard]] const Vector2& getParallaxFactorProperty() const { return parallaxFactor_; }
        void setParallaxFactorProperty(const Vector2& value) { parallaxFactor_ = value; }

        /** @brief Gets the custom properties of the layer. */
        [[nodiscard]] TilemapProperties& getPropertiesProperty() { return properties_; }
        [[nodiscard]] const TilemapProperties& getPropertiesProperty() const { return properties_; }

        /** @brief Gets the bounding rectangle of the layer in world coordinates. */
        [[nodiscard]] virtual Rectangle getBoundsProperty() const = 0;

    protected:
        explicit TilemapLayer(std::string name);

    private:
        std::string name_;
        std::string class_;
        bool isVisible_ = true;
        float opacity_ = 1.0f;
        std::optional<Color> tintColor_;
        Vector2 offset_;
        Vector2 parallaxFactor_ = Vector2::One;
        TilemapProperties properties_;
    };
}
