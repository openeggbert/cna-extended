// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/Screens/TestScreens.cs.
// Test-only fixtures, not part of the library -- lives under tests/, header-only.
#pragma once

#include "CNA/Extended/Screens/GameScreen.hpp"
#include "CNA/Extended/Screens/Screen.hpp"

#include <functional>
#include <optional>
#include <string>

namespace CNA::Extended::Screens::Tests
{
    class TestScreen : public Screen
    {
    public:
        explicit TestScreen(std::string name = "TestScreen") : name_(std::move(name)) {}

        void Dispose() override { disposeCalled_ = true; }

        void OnActivated() override { ++activatedCallCount_; }
        void OnDeactivated() override { ++deactivatedCallCount_; }

        void Update(GameTime& gameTime) override
        {
            if (onUpdate)
            {
                onUpdate(name_);
            }
            ++updateCallCount_;
            lastUpdateGameTime_ = gameTime;
        }

        void Draw(const GameTime& gameTime) override
        {
            if (onDraw)
            {
                onDraw(name_);
            }
            ++drawCallCount_;
            lastDrawGameTime_ = gameTime;
        }

        void Reset()
        {
            disposeCalled_ = false;
            updateCallCount_ = 0;
            drawCallCount_ = 0;
            activatedCallCount_ = 0;
            deactivatedCallCount_ = 0;
            lastUpdateGameTime_.reset();
            lastDrawGameTime_.reset();
            onUpdate = nullptr;
            onDraw = nullptr;
        }

        [[nodiscard]] bool getDisposeCalledProperty() const { return disposeCalled_; }
        [[nodiscard]] int getUpdateCallCountProperty() const { return updateCallCount_; }
        [[nodiscard]] int getDrawCallCountProperty() const { return drawCallCount_; }
        [[nodiscard]] int getActivatedCallCountProperty() const { return activatedCallCount_; }
        [[nodiscard]] int getDeactivatedCallCountProperty() const { return deactivatedCallCount_; }
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }

        std::function<void(const std::string&)> onUpdate;
        std::function<void(const std::string&)> onDraw;

    private:
        std::string name_;
        bool disposeCalled_ = false;
        int updateCallCount_ = 0;
        int drawCallCount_ = 0;
        int activatedCallCount_ = 0;
        int deactivatedCallCount_ = 0;
        std::optional<GameTime> lastUpdateGameTime_;
        std::optional<GameTime> lastDrawGameTime_;
    };

    class TestGameScreen : public GameScreen
    {
    public:
        explicit TestGameScreen(Microsoft::Xna::Framework::Game* game) : GameScreen(game) {}

        void Update(GameTime&) override {}
        void Draw(const GameTime&) override {}
    };
}
