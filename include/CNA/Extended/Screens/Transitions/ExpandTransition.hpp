// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Screens/Transitions/ExpandTransition.cs: expands/collapses a
// solid-color rectangle from the center of the viewport. Depends on `CNA::Extended::FillRectangle`
// (ShapeExtensions.cs) -- see FadeTransition.hpp's header comment for the same deferral history.
#pragma once

#include "CNA/Extended/Screens/Transitions/Transition.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
}

namespace CNA::Extended::Screens::Transitions
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;

    /** @brief A Transition that expands/collapses a solid-color rectangle from the center of the viewport. */
    class ExpandTransition : public Transition
    {
    public:
        /**
         * @brief Creates an expand transition to/from @p color over @p duration seconds (default 1.0).
         * @param graphicsDevice The device whose viewport is used to size the rectangle and that the internal SpriteBatch draws to.
         */
        explicit ExpandTransition(GraphicsDevice& graphicsDevice, const Color& color, float duration = 1.0f);

        void Dispose() override;

        /** @brief Gets the color of the expanding/collapsing rectangle. */
        [[nodiscard]] const Color& getColorProperty() const { return color_; }

        void Draw(const GameTime& gameTime) override;

        /** @brief Returns the fully qualified .NET type name of this class. */
        [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        GraphicsDevice* graphicsDevice_;
        SpriteBatch spriteBatch_;
        Color color_;
    };
}
