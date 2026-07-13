// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapLayers/TilemapImageLayer.cs. `Texture2D
// Texture` (nullable, externally-owned GPU resource) -> a non-owning, nullable `Texture2D*`,
// matching this project's established convention.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapLayer.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    /** @brief A layer that displays a single image. */
    class TilemapImageLayer : public TilemapLayer
    {
    public:
        /** @brief Creates an image layer with @p name, @p texture (non-owning, may be nullptr), and @p position. */
        TilemapImageLayer(std::string name, Texture2D* texture, const Vector2& position);

        /** @brief Gets/sets the image texture displayed by this layer (non-owning, may be nullptr). */
        [[nodiscard]] Texture2D* getTextureProperty() const { return texture_; }
        void setTextureProperty(Texture2D* value) { texture_ = value; }

        /** @brief Gets/sets the position of the image. */
        [[nodiscard]] const Vector2& getPositionProperty() const { return position_; }
        void setPositionProperty(const Vector2& value) { position_ = value; }

        /** @brief Gets/sets whether the image repeats horizontally. */
        [[nodiscard]] bool getRepeatXProperty() const { return repeatX_; }
        void setRepeatXProperty(bool value) { repeatX_ = value; }

        /** @brief Gets/sets whether the image repeats vertically. */
        [[nodiscard]] bool getRepeatYProperty() const { return repeatY_; }
        void setRepeatYProperty(bool value) { repeatY_ = value; }

        [[nodiscard]] Rectangle getBoundsProperty() const override;

    private:
        Texture2D* texture_;
        Vector2 position_;
        bool repeatX_ = false;
        bool repeatY_ = false;
    };
}
