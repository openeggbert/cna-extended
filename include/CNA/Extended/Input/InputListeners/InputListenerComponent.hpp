// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/InputListenerComponent.cs. Upstream's
// `: GameComponent, IUpdateable` is redundant in C# (GameComponent already implements
// IUpdateable) -- ported as a plain single-base derivation from CNA's GameComponent, which
// already implements IUpdateable transitively, avoiding a diamond re-declaration.
//
// Upstream's `params InputListener[] listeners` constructor (a C#-only varargs convenience with
// no direct C++ equivalent) -> a constructor taking `std::vector<std::unique_ptr<InputListener>>`
// by value; the caller builds the vector explicitly instead of passing a variadic argument list.
// This component owns every InputListener it's given (matching upstream's reference-type-handed-
// off-and-stored intent), so listeners are stored via std::unique_ptr<InputListener>.
#pragma once

#include "CNA/Extended/Input/InputListeners/InputListener.hpp"
#include "Microsoft/Xna/Framework/GameComponent.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::Input::InputListeners
{
    /** @brief A GameComponent that owns a set of InputListeners and drives their Update() each frame while the game is active. */
    class InputListenerComponent : public Microsoft::Xna::Framework::GameComponent
    {
    public:
        /**
         * @brief Initializes a new InputListenerComponent with no listeners.
         * @param game The game that owns this component.
         */
        explicit InputListenerComponent(Microsoft::Xna::Framework::Game& game);

        /**
         * @brief Initializes a new InputListenerComponent, taking ownership of the given listeners.
         * @param game The game that owns this component.
         * @param listeners The listeners this component will own and update.
         */
        InputListenerComponent(Microsoft::Xna::Framework::Game& game, std::vector<std::unique_ptr<InputListener>> listeners);

        /** @brief Gets the listeners owned by this component. */
        [[nodiscard]] std::vector<std::unique_ptr<InputListener>>& getListenersProperty() { return listeners_; }

        void Update(GameTime& gameTime) override;

    private:
        std::vector<std::unique_ptr<InputListener>> listeners_;
    };
}
