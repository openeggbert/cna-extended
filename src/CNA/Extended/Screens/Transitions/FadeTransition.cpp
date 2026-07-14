// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Screens/Transitions/FadeTransition.hpp"

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

    FadeTransition::FadeTransition(GraphicsDevice& graphicsDevice, const Color& color, float duration)
        : Transition(duration), graphicsDevice_(&graphicsDevice), spriteBatch_(graphicsDevice), color_(color)
    {
    }

    void FadeTransition::Dispose()
    {
        spriteBatch_.Dispose();
    }

    void FadeTransition::Draw(const GameTime& gameTime)
    {
        (void)gameTime;
        SamplerState pointClamp = SamplerState::PointClamp;
        spriteBatch_.Begin(SpriteSortMode::Deferred, BlendState::AlphaBlend, &pointClamp, nullptr, nullptr);
        FillRectangle(spriteBatch_, 0.0f, 0.0f, static_cast<float>(graphicsDevice_->getViewportProperty().getWidthProperty()),
            static_cast<float>(graphicsDevice_->getViewportProperty().getHeightProperty()), color_ * getValueProperty());
        spriteBatch_.End();
    }

    const std::string& FadeTransition::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.Screens.Transitions.FadeTransition";
        return typeName;
    }
}
