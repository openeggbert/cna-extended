// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's SimpleGameComponent.cs: a lighter-weight abstract base for
// update-only components that don't need a Game reference (unlike CNA's own GameComponent, which
// stores one). Implements CNA's IGameComponent/IUpdateable/System::IDisposable/
// System::IComparable<GameComponent>/System::IComparable<SimpleGameComponent>, following the same
// event-handling pattern established by GameComponent.hpp/.cpp and FramesPerSecondCounter.hpp/
// .cpp (System::Object base, public EventHandler<EventArgs> members, Raise(this,
// EventArgs::Empty)).
//
// C#'s explicit interface implementation (`bool IUpdateable.Enabled => _isEnabled;`, distinct
// from the public `IsEnabled` property of the same underlying field) has no direct C++
// equivalent, but is approximated here: getEnabledProperty() overrides IUpdateable's pure
// virtual as a *private* member (callable through an IUpdateable& reference via virtual dispatch,
// exactly like upstream's interface-only access, but not part of SimpleGameComponent's own public
// API surface), while getIsEnabledProperty()/setIsEnabledProperty() are the real public API,
// matching upstream's own choice of a different public name (IsEnabled, not Enabled). Both
// getters return the same underlying value -- this is purely a visibility/naming distinction in
// upstream too, not a behavioral one.
#pragma once

#include "Microsoft/Xna/Framework/GameComponent.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/IGameComponent.hpp"
#include "Microsoft/Xna/Framework/IUpdateable.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/EventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/IComparable.hpp"
#include "System/IDisposable.hpp"
#include "System/Object.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::GameComponent;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::IGameComponent;
    using Microsoft::Xna::Framework::IUpdateable;

    /**
     * @brief A lighter-weight abstract base for update-only game components that don't require a
     * Game reference.
     */
    class SimpleGameComponent : public System::Object,
                                 public IGameComponent,
                                 public IUpdateable,
                                 public System::IDisposable,
                                 public System::IComparable<GameComponent>,
                                 public System::IComparable<SimpleGameComponent>
    {
    public:
        /** @brief Raised when IsEnabled changes. */
        System::EventHandler<System::EventArgs> EnabledChanged;

        /** @brief Raised when UpdateOrder changes. */
        System::EventHandler<System::EventArgs> UpdateOrderChanged;

        SimpleGameComponent() = default;

        /** @brief Unloads content if this component was initialized. Idempotent. */
        void Dispose() override;

        /** @brief Gets whether this component should be updated. */
        [[nodiscard]] bool getIsEnabledProperty() const;
        /** @brief Sets whether this component should be updated. Raises EnabledChanged when the value changes. */
        void setIsEnabledProperty(bool value);

        /** @brief Loads content if this component was not already initialized. */
        void Initialize() override;

        [[nodiscard]] SharpRuntime::intcs getUpdateOrderProperty() const override;
        /** @brief Sets the update order. Raises UpdateOrderChanged when the value changes. */
        void setUpdateOrderProperty(SharpRuntime::intcs value);

        [[nodiscard]] System::EventHandler<System::EventArgs>& getEnabledChangedEvent() override;
        [[nodiscard]] System::EventHandler<System::EventArgs>& getUpdateOrderChangedEvent() override;

        void Update(GameTime& gameTime) override = 0;

        [[nodiscard]] int CompareTo(const GameComponent& other) const override;
        [[nodiscard]] int CompareTo(const SimpleGameComponent& other) const override;

    protected:
        /** @brief Called once, the first time this component is initialized. Override to load resources. */
        virtual void LoadContent();

        /** @brief Called when this component is disposed after having been initialized. Override to release resources. */
        virtual void UnloadContent();

    private:
        // See this file's header comment: approximates upstream's `bool IUpdateable.Enabled`
        // explicit interface implementation.
        [[nodiscard]] bool getEnabledProperty() const override;

        bool isInitialized_ = false;
        bool isEnabled_ = true;
        SharpRuntime::intcs updateOrder_ = 0;
    };
}
