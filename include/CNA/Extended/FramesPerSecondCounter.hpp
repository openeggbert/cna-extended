// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's FramesPerSecondCounter.cs. Implements CNA's IUpdateable
// interface, following the same pattern CNA's own GameComponent.hpp/.cpp uses (System::Object
// base, EnabledChanged/UpdateOrderChanged as public System::EventHandler<System::EventArgs>
// members, getEnabledChangedEvent()/getUpdateOrderChangedEvent() returning references to them).
//
// *** LIKELY UPSTREAM BUG, PRESERVED FOR FIDELITY -- flagged prominently, not silently fixed ***
// Upstream's `UpdateOrder` property setter raises `EnabledChanged?.Invoke(...)` instead of
// `UpdateOrderChanged?.Invoke(...)` -- almost certainly a copy-paste error from the `Enabled`
// setter immediately above it. Reproduced exactly below: setUpdateOrderProperty() raises
// EnabledChanged, not UpdateOrderChanged. See the regression test in
// FramesPerSecondCounterTests.cpp that names this discrepancy explicitly. Per this project's
// "port 1:1, no simplification" requirement, this is not this port's call to fix.
//
// `_oneSecondTimeSpan` is `static readonly` upstream (a true constant); ported as a local value
// constructed where needed rather than a `static const System::TimeSpan` class member, to avoid
// any static-initialization-order risk (a real bug found and fixed earlier in this project's
// Matrix3x2::Identity) -- TimeSpan(0, 0, 1) has no observable per-instance-vs-static difference
// since it never changes, so this is a behavior-preserving simplification of storage duration
// only, not of logic.
#pragma once

#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/IUpdateable.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/EventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/Object.hpp"
#include "System/TimeSpan.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::IUpdateable;

    /** @brief Tracks the number of frames drawn per second by counting Draw() calls each second of Update() time. */
    class FramesPerSecondCounter : public System::Object, public IUpdateable
    {
    public:
        /** @brief Raised when Enabled changes. Also (see header comment) incorrectly raised by setUpdateOrderProperty(), matching upstream. */
        System::EventHandler<System::EventArgs> EnabledChanged;

        /** @brief Raised when UpdateOrder changes. Upstream never actually raises this event -- see header comment. */
        System::EventHandler<System::EventArgs> UpdateOrderChanged;

        FramesPerSecondCounter();

        [[nodiscard]] bool getEnabledProperty() const override;
        /** @brief Sets whether this counter participates in the update loop. Raises EnabledChanged when the value changes. */
        void setEnabledProperty(bool value);

        [[nodiscard]] SharpRuntime::intcs getUpdateOrderProperty() const override;
        /**
         * @brief Sets the update order. Raises EnabledChanged (not UpdateOrderChanged) when the
         * value changes -- see this file's header comment for why.
         */
        void setUpdateOrderProperty(SharpRuntime::intcs value);

        [[nodiscard]] System::EventHandler<System::EventArgs>& getEnabledChangedEvent() override;
        [[nodiscard]] System::EventHandler<System::EventArgs>& getUpdateOrderChangedEvent() override;

        /** @brief Gets the number of Draw() calls counted during the most recently completed one-second interval. */
        [[nodiscard]] int getFramesPerSecondProperty() const;

        /** @brief Advances the internal one-second timer and, once a second has elapsed, latches the frame count. */
        void Update(GameTime& gameTime) override;

        /** @brief Counts one drawn frame. */
        void Draw(const GameTime& gameTime);

        [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        bool enabled_ = false;
        SharpRuntime::intcs updateOrder_ = 0;
        int framesPerSecond_ = 0;
        int framesCounter_ = 0;
        System::TimeSpan timer_;
    };
}
