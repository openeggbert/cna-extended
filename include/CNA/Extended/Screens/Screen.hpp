// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Screens/Screen.cs: an abstract base for game screens managed by
// a ScreenManager. Upstream lives in the MonoGame.Extended.Screens namespace; ported into a
// matching CNA::Extended::Screens sub-namespace, per this project's sub-namespace-per-module
// convention.
//
// `ScreenManager ScreenManager { get; internal set; }` / `bool IsActive { get; internal set; }` --
// C#'s `internal set` (assembly-visible, but not publicly settable) has no direct C++ equivalent;
// approximated here via `friend class ScreenManager` + private setters, which is actually MORE
// restrictive than `internal` (only ScreenManager itself can set these, not "any class in this
// project"), but achieves the same practical intent -- these two properties exist purely for
// ScreenManager's own bookkeeping.
#pragma once

#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "System/IDisposable.hpp"

namespace CNA::Extended::Screens
{
    using Microsoft::Xna::Framework::GameTime;

    class ScreenManager;

    /**
     * @brief Abstract base for game screens that can be managed by a ScreenManager.
     * @remark When used with a ScreenManager, multiple screens can be active simultaneously in
     * the manager's screen stack based on their UpdateWhenInactive/DrawWhenInactive properties.
     */
    class Screen : public System::IDisposable
    {
    public:
        virtual ~Screen() = default;

        /** @brief Gets the ScreenManager that manages this screen, or nullptr if the screen is not managed. */
        [[nodiscard]] ScreenManager* getScreenManagerProperty() const { return screenManager_; }

        /** @brief Gets whether this screen is currently the active screen. */
        [[nodiscard]] bool getIsActiveProperty() const { return isActive_; }

        /** @brief Gets or sets whether this screen should continue to update when it is not the active screen. */
        [[nodiscard]] bool getUpdateWhenInactiveProperty() const { return updateWhenInactive_; }
        void setUpdateWhenInactiveProperty(bool value) { updateWhenInactive_ = value; }

        /** @brief Gets or sets whether this screen should continue to draw when it is not the active screen. */
        [[nodiscard]] bool getDrawWhenInactiveProperty() const { return drawWhenInactive_; }
        void setDrawWhenInactiveProperty(bool value) { drawWhenInactive_ = value; }

        /** @brief Releases all resources used by the screen. The base implementation does nothing. */
        void Dispose() override {}

        /** @brief Initializes the screen. Called automatically when first shown by a ScreenManager. The base implementation does nothing. */
        virtual void Initialize() {}

        /** @brief Loads content/resources needed by the screen. Called automatically after Initialize(). The base implementation does nothing. */
        virtual void LoadContent() {}

        /** @brief Unloads content/resources used by the screen. Called automatically when closed. The base implementation does nothing. */
        virtual void UnloadContent() {}

        /** @brief Called every time this screen becomes the active (top) screen, including re-activation. The base implementation does nothing. */
        virtual void OnActivated() {}

        /** @brief Called every time this screen is no longer the active (top) screen. The base implementation does nothing. */
        virtual void OnDeactivated() {}

        /** @brief Updates the screen's logic. Called automatically when IsActive or UpdateWhenInactive is true. */
        virtual void Update(GameTime& gameTime) = 0;

        /** @brief Draws the screen's visual content. Called automatically when IsActive or DrawWhenInactive is true. */
        virtual void Draw(const GameTime& gameTime) = 0;

    private:
        friend class ScreenManager;

        ScreenManager* screenManager_ = nullptr;
        bool isActive_ = false;
        bool updateWhenInactive_ = false;
        bool drawWhenInactive_ = false;
    };
}
