// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's (root-level) AnimationComponent.cs. NOTE: despite living at the
// package root in upstream's own source tree, its declared namespace is
// `MonoGame.Extended.Animations` -- the same namespace as everything else under Animations/, not
// the root `MonoGame.Extended` namespace its file location might suggest. This port follows the
// real namespace (CNA::Extended::Animations) and file layout
// (include/CNA/Extended/Animations/AnimationComponent.hpp), matching this project's "file layout
// mirrors namespace path" rule, rather than the root-level placement its upstream file path alone
// would imply.
//
// `List<AnimationController> Animations` is exposed as a mutable, non-owning
// `std::vector<AnimationController*>&`: Update() only prunes disposed entries
// (`RemoveAll(a => a.IsDisposed)`), it never disposes them itself, implying callers retain
// ownership and are responsible for disposal -- matching the ScreenManager/Screen* non-owning
// reference precedent used elsewhere in this port.
#pragma once

#include "CNA/Extended/Animations/AnimationController.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameComponent.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"

#include <vector>

namespace CNA::Extended::Animations
{
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::GameComponent;
    using Microsoft::Xna::Framework::GameTime;

    /** @brief A GameComponent that updates a collection of AnimationController instances each frame, pruning disposed ones. */
    class AnimationComponent : public GameComponent
    {
    public:
        /** @brief Creates the component, attaching it to the specified game. */
        explicit AnimationComponent(Game& game);

        /** @brief Gets the animation controllers updated by this component. Callers retain ownership and are responsible for disposal; disposed controllers are pruned automatically after each Update. */
        [[nodiscard]] std::vector<AnimationController*>& getAnimationsProperty() { return animations_; }
        /** @copydoc getAnimationsProperty() */
        [[nodiscard]] const std::vector<AnimationController*>& getAnimationsProperty() const { return animations_; }

        void Update(GameTime& gameTime) override;

        [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        std::vector<AnimationController*> animations_;
    };
}
