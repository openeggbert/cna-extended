// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's SimpleDrawableGameComponent.cs: adds IDrawable to
// SimpleGameComponent, the same way CNA's own DrawableGameComponent adds IDrawable to
// GameComponent -- but without requiring a Game/GraphicsDevice reference.
//
// Upstream also has an explicit-interface-implementation `bool IDrawable.Visible => _isVisible;`
// alongside a public `Visible` property -- unlike SimpleGameComponent's IsEnabled/Enabled split
// (see SimpleGameComponent.hpp), upstream gives BOTH members the exact same name (`Visible`),
// relying purely on C#'s explicit-interface-implementation syntax (resolved by the static type of
// the reference used to call it) to distinguish them; both getters return the same
// underlying field regardless. C++ has no way to give two members the identical name with
// different accessibility, so this collapses to a single public getVisibleProperty()/
// setVisibleProperty() pair that satisfies IDrawable's pure virtual directly -- a
// naming/visibility simplification only (upstream provided no distinct name to preserve here,
// unlike IsEnabled/Enabled), not a behavioral one.
#pragma once

#include "CNA/Extended/SimpleGameComponent.hpp"
#include "Microsoft/Xna/Framework/IDrawable.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::IDrawable;

    /** @brief A lighter-weight abstract base for drawable game components that don't require a Game reference. */
    class SimpleDrawableGameComponent : public SimpleGameComponent, public IDrawable
    {
    public:
        /** @brief Raised when DrawOrder changes. */
        System::EventHandler<System::EventArgs> DrawOrderChanged;

        /** @brief Raised when Visible changes. */
        System::EventHandler<System::EventArgs> VisibleChanged;

        SimpleDrawableGameComponent() = default;

        /** @brief Gets whether this component should be drawn. See this file's header comment about the Visible/IDrawable.Visible collapse. */
        [[nodiscard]] bool getVisibleProperty() const override;
        /** @brief Sets whether this component should be drawn. Raises VisibleChanged when the value changes. */
        void setVisibleProperty(bool value);

        [[nodiscard]] SharpRuntime::intcs getDrawOrderProperty() const override;
        /** @brief Sets the draw order. Raises DrawOrderChanged when the value changes. */
        void setDrawOrderProperty(SharpRuntime::intcs value);

        [[nodiscard]] System::EventHandler<System::EventArgs>& getDrawOrderChangedEvent() override;
        [[nodiscard]] System::EventHandler<System::EventArgs>& getVisibleChangedEvent() override;

        void Draw(const GameTime& gameTime) override = 0;

    private:
        bool isVisible_ = true;
        SharpRuntime::intcs drawOrder_ = 0;
    };
}
