// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Screens/Transitions/FadeTransition.cs: fades the screen to/from
// a solid color. Depends on `CNA::Extended::FillRectangle` (ShapeExtensions.cs), which is why this
// was deferred until Phase 5 landed ShapeExtensions -- see Transition.hpp's header comment and
// NEXT.md for the deferral history.
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

    /** @brief A Transition that fades the whole viewport to/from a solid color. */
    class FadeTransition : public Transition
    {
    public:
        /**
         * @brief Creates a fade transition to/from @p color over @p duration seconds (default 1.0).
         * @param graphicsDevice The device whose viewport is faded and that the internal SpriteBatch draws to.
         * @param color The solid color the viewport fades to/from.
         * @param duration The transition's duration, in seconds.
         */
        explicit FadeTransition(GraphicsDevice& graphicsDevice, const Color& color, float duration = 1.0f);

        void Dispose() override;

        /** @brief Gets the color faded to/from. */
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
