// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// *** REAL C#/C++ SEMANTIC HAZARD, HANDLED DELIBERATELY, NOT A LITERAL TRANSLATION ***
// Upstream's transition Completed handler does `_activeTransition.Dispose(); _activeTransition =
// null;` -- safe in C# because dropping the last reference doesn't immediately reclaim the
// object's memory; the GC collects it at some later, unspecified point, long after the enclosing
// Transition.Update() call (which raised Completed in the first place) has returned. A literal
// C++ translation using std::unique_ptr::reset() inside that same handler would destroy the
// Transition object WHILE ITS OWN Update() method is still executing on the call stack
// (ScreenManager::Update() -> activeTransition_->Update() -> Completed.Raise() -> this lambda ->
// reset() -> ~Transition() -- with Update()'s own stack frame still above it) -- a genuine
// use-after-free once Update() continues executing/returns after being destroyed out from under
// itself. Fixed by deferring the actual destruction: the Completed handler only sets
// transitionCompletedPending_; ScreenManager::Update() checks and safely destroys the transition
// AFTER activeTransition_->Update() has fully returned, back in ScreenManager's own stack frame.
#include "CNA/Extended/Screens/ScreenManager.hpp"

#include <stdexcept>

namespace CNA::Extended::Screens
{
    void ScreenManager::ShowScreen(Screen* screen)
    {
        if (screen == nullptr)
        {
            throw std::invalid_argument("screen must not be null.");
        }

        if (activeScreen_ != nullptr)
        {
            activeScreen_->isActive_ = false;
            activeScreen_->OnDeactivated();
        }

        screen->screenManager_ = this;
        screen->isActive_ = true;
        screen->Initialize();
        screen->LoadContent();
        screen->OnActivated();

        screens_.push_back(screen);
        activeScreen_ = screen;
    }

    void ScreenManager::ShowScreen(Screen* screen, std::unique_ptr<Transitions::Transition> transition)
    {
        if (activeTransition_)
        {
            return;
        }

        activeTransition_ = std::move(transition);
        activeTransition_->StateChanged += [this, screen](System::Object*, const System::EventArgs&) { ShowScreen(screen); };
        activeTransition_->Completed += [this](System::Object*, const System::EventArgs&) { transitionCompletedPending_ = true; };
    }

    void ScreenManager::CloseScreen()
    {
        if (screens_.empty())
        {
            return;
        }

        Screen* screen = screens_.back();
        screens_.pop_back();

        screen->isActive_ = false;
        screen->OnDeactivated();
        screen->UnloadContent();
        screen->Dispose();

        if (!screens_.empty())
        {
            activeScreen_ = screens_.back();
            activeScreen_->isActive_ = true;
            activeScreen_->OnActivated();
        }
        else
        {
            activeScreen_ = nullptr;
        }
    }

    void ScreenManager::CloseScreen(std::unique_ptr<Transitions::Transition> transition)
    {
        if (activeTransition_)
        {
            return;
        }

        activeTransition_ = std::move(transition);
        activeTransition_->StateChanged += [this](System::Object*, const System::EventArgs&) { CloseScreen(); };
        activeTransition_->Completed += [this](System::Object*, const System::EventArgs&) { transitionCompletedPending_ = true; };
    }

    void ScreenManager::ReplaceScreen(Screen* screen)
    {
        CloseScreen();
        ShowScreen(screen);
    }

    void ScreenManager::ReplaceScreen(Screen* screen, std::unique_ptr<Transitions::Transition> transition)
    {
        if (activeTransition_)
        {
            return;
        }

        activeTransition_ = std::move(transition);
        activeTransition_->StateChanged += [this, screen](System::Object*, const System::EventArgs&) { ReplaceScreen(screen); };
        activeTransition_->Completed += [this](System::Object*, const System::EventArgs&) { transitionCompletedPending_ = true; };
    }

    void ScreenManager::ClearScreens()
    {
        while (!screens_.empty())
        {
            Screen* screen = screens_.back();
            screens_.pop_back();

            const bool wasActive = screen->isActive_;
            screen->isActive_ = false;
            if (wasActive)
            {
                screen->OnDeactivated();
            }
            screen->UnloadContent();
            screen->Dispose();
        }

        activeScreen_ = nullptr;
    }

    void ScreenManager::Initialize()
    {
        SimpleDrawableGameComponent::Initialize();

        if (activeScreen_ != nullptr)
        {
            activeScreen_->Initialize();
        }
    }

    void ScreenManager::LoadContent()
    {
        SimpleDrawableGameComponent::LoadContent();

        if (activeScreen_ != nullptr)
        {
            activeScreen_->LoadContent();
        }
    }

    void ScreenManager::UnloadContent()
    {
        SimpleDrawableGameComponent::UnloadContent();

        if (activeScreen_ != nullptr)
        {
            activeScreen_->UnloadContent();
        }
    }

    void ScreenManager::Update(GameTime& gameTime)
    {
        for (Screen* screen : screens_)
        {
            if (screen->getIsActiveProperty() || screen->getUpdateWhenInactiveProperty())
            {
                screen->Update(gameTime);
            }
        }

        if (activeTransition_)
        {
            activeTransition_->Update(gameTime);

            if (transitionCompletedPending_)
            {
                transitionCompletedPending_ = false;
                activeTransition_->Dispose();
                activeTransition_.reset();
            }
        }
    }

    void ScreenManager::Draw(const GameTime& gameTime)
    {
        for (Screen* screen : screens_)
        {
            if (screen->getIsActiveProperty() || screen->getDrawWhenInactiveProperty())
            {
                screen->Draw(gameTime);
            }
        }

        if (activeTransition_)
        {
            activeTransition_->Draw(gameTime);
        }
    }
}
