// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Screens/Transitions/ExpandTransition.hpp"

#include "CNA/Extended/RectangleF.hpp"
#include "CNA/Extended/ShapeExtensions.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

namespace CNA::Extended::Screens::Transitions
{
    using Microsoft::Xna::Framework::Graphics::BlendState;
    using Microsoft::Xna::Framework::Graphics::SamplerState;
    using Microsoft::Xna::Framework::Graphics::SpriteSortMode;

    ExpandTransition::ExpandTransition(GraphicsDevice& graphicsDevice, const Color& color, float duration)
        : Transition(duration), graphicsDevice_(&graphicsDevice), spriteBatch_(graphicsDevice), color_(color)
    {
    }

    void ExpandTransition::Dispose()
    {
        spriteBatch_.Dispose();
    }

    void ExpandTransition::Draw(const GameTime& gameTime)
    {
        (void)gameTime;
        const float viewportWidth = static_cast<float>(graphicsDevice_->getViewportProperty().getWidthProperty());
        const float viewportHeight = static_cast<float>(graphicsDevice_->getViewportProperty().getHeightProperty());
        const float value = getValueProperty();

        const float halfWidth = viewportWidth / 2.0f;
        const float halfHeight = viewportHeight / 2.0f;
        const float x = halfWidth * (1.0f - value);
        const float y = halfHeight * (1.0f - value);
        const float width = viewportWidth * value;
        const float height = viewportHeight * value;
        const RectangleF rectangle(x, y, width, height);

        SamplerState pointClamp = SamplerState::PointClamp;
        spriteBatch_.Begin(SpriteSortMode::Deferred, BlendState::AlphaBlend, &pointClamp, nullptr, nullptr);
        FillRectangle(spriteBatch_, rectangle, color_);
        spriteBatch_.End();
    }
}
