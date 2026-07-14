// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Screens/ScreenManager.cs: manages a stack of Screen instances,
// where the topmost is the active screen while underlying screens can continue updating/drawing
// in the background based on their own settings.
//
// Ownership: Screen* is a NON-owning reference throughout, not std::unique_ptr<Screen>. This is a
// deliberate choice verified against upstream's own test suite behavior, not assumed: upstream's
// ScreenManagerTests.cs constructs a Screen, hands it to ShowScreen, later calls CloseScreen(),
// and THEN asserts on the closed screen's own state (e.g. `Assert.True(screen2.DisposeCalled)`
// after `manager.CloseScreen()`) -- proving the caller retains and inspects the Screen after the
// manager is done with it. In C#, this works because GC keeps the object alive as long as the
// test's own local variable references it; the manager's own "removal" never destroys the object,
// only calls Dispose() and stops tracking it. A C++ owning std::unique_ptr<Screen> that destroys
// the Screen on CloseScreen() would leave the caller's own pointer dangling the moment the test
// inspects it -- so Screen* here is intentionally non-owning, matching the caller-retains-ownership
// semantics upstream's own tests prove, not upstream's GC mechanism.
//
// `Stack<Screen> _screens` + `_screens.Reverse().ToArray()` (bottom-to-top cached array) ->
// `std::vector<Screen*>` used directly as the stack (push_back/pop_back/back()), since forward
// iteration over a std::vector already visits bottom-to-top in insertion order -- no separate
// reverse step needed, a simplification C++'s single all-purpose sequence container affords for
// free where C# needed a dedicated Stack<T> plus a Reverse().ToArray() cache.
//
// ShowScreen(Screen, Transition)/CloseScreen(Transition)/ReplaceScreen(Screen, Transition) take
// std::unique_ptr<Transitions::Transition> (ownership-transferring), unlike Screen* -- Transition
// instances are always constructed inline at the call site and never referenced again by the
// caller afterward (unlike Screen, no test or realistic usage keeps a Transition reference around
// once handed to a ScreenManager), so ScreenManager taking sole ownership is the correct, safe
// translation for that specific parameter.
//
// Real C#/C++ hazard fixed, not a literal translation: upstream's transition Completed handler
// disposes and drops the reference synchronously, safe under GC since the object's memory outlives
// that call. A literal std::unique_ptr::reset() in that same handler would destroy the Transition
// while its own Update() call is still on the stack above it. See ScreenManager.cpp's header
// comment for the fix (deferred destruction via transitionCompletedPending_).
#pragma once

#include "CNA/Extended/Screens/Screen.hpp"
#include "CNA/Extended/Screens/Transitions/Transition.hpp"
#include "CNA/Extended/SimpleDrawableGameComponent.hpp"

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::Screens
{
    /**
     * @brief Manages a stack of Screen instances in a game.
     * @remark Screens are managed using a stack-based approach where the topmost screen is
     * considered the active screen, while underlying screens can continue to update and draw in
     * the background based on their UpdateWhenInactive/DrawWhenInactive settings.
     * @see Screen, the base type managed by this stack. ScreenManager never owns the Screens
     * passed to ShowScreen/ReplaceScreen -- see this header's own top comment for why (ownership
     * remains with the caller, only Transitions::Transition ownership transfers).
     * @see Transitions::Transition, Transitions::FadeTransition, Transitions::ExpandTransition,
     * the optional transition effects accepted by ShowScreen/CloseScreen/ReplaceScreen.
     */
    class ScreenManager : public SimpleDrawableGameComponent
    {
    public:
        ScreenManager() = default;

        [[nodiscard]] const std::string& GetTypeName() const override
        {
            static const std::string typeName = "MonoGame.Extended.Screens.ScreenManager";
            return typeName;
        }

        /** @brief Gets the currently active screen at the top of the screen stack, or nullptr if no screens are loaded. */
        [[nodiscard]] Screen* getActiveScreenProperty() const { return activeScreen_; }

        /**
         * @brief Gets a read-only list of all screens in the stack, ordered from bottom to top.
         * @remark For performance, the list is cached internally and only rebuilt when the screen stack has been modified.
         */
        [[nodiscard]] const std::vector<Screen*>& getScreensProperty() const { return screens_; }

        /**
         * @brief Pushes a new screen onto the stack and makes it the active screen.
         * @param screen The screen to show. Must not be null. Ownership remains with the caller.
         * @remark The previous active screen remains in the screen stack but becomes inactive.
         * @throws std::invalid_argument screen is null.
         */
        void ShowScreen(Screen* screen);

        /**
         * @brief Pushes a new screen onto the stack with a transition effect and makes it the active screen.
         * @param screen The screen to show. Ownership remains with the caller.
         * @param transition The transition effect to use. ScreenManager takes ownership.
         * @remark The previous active screen remains in the screen stack but becomes inactive.
         */
        void ShowScreen(Screen* screen, std::unique_ptr<Transitions::Transition> transition);

        /**
         * @brief Pops the current active screen from the stack, disposing it and making the next screen active.
         * @remark If the screen stack becomes empty, no screen will be active.
         */
        void CloseScreen();

        /**
         * @brief Pops the current active screen from the stack with a transition effect.
         * @param transition The transition effect to use. ScreenManager takes ownership.
         * @remark If the screen stack becomes empty, no screen will be active.
         */
        void CloseScreen(std::unique_ptr<Transitions::Transition> transition);

        /**
         * @brief Replaces the current active screen with a new screen.
         * @remark Equivalent to calling CloseScreen() followed by ShowScreen(Screen*).
         */
        void ReplaceScreen(Screen* screen);

        /**
         * @brief Replaces the current active screen with a new screen using a transition effect.
         * @remark Equivalent to calling CloseScreen(transition) followed by ShowScreen(screen, transition).
         */
        void ReplaceScreen(Screen* screen, std::unique_ptr<Transitions::Transition> transition);

        /** @brief Clears all screens from the stack, disposing them and setting no active screen. */
        void ClearScreens();

        /** @brief Initializes the screen manager and the currently active screen if one exists. */
        void Initialize() override;

        /**
         * @brief Updates all screens in the stack where IsActive or UpdateWhenInactive is true.
         * @remark Update order for screens is done from the bottom of the stack to the top.
         */
        void Update(GameTime& gameTime) override;

        /**
         * @brief Draws all screens in the stack where IsActive or DrawWhenInactive is true.
         * @remark Draw order for screens is done from the bottom of the stack to the top, to allow
         * background screens to draw before foreground ones for proper visual stacking.
         */
        void Draw(const GameTime& gameTime) override;

    protected:
        /** @brief Loads content for the screen manager and the currently active screen if one exists. */
        void LoadContent() override;

        /** @brief Unloads content for the screen manager and the currently active screen if one exists. */
        void UnloadContent() override;

    private:
        std::vector<Screen*> screens_;
        Screen* activeScreen_ = nullptr;
        std::unique_ptr<Transitions::Transition> activeTransition_;
        bool transitionCompletedPending_ = false;
    };
}
