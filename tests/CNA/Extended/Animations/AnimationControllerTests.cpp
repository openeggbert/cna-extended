// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Animations/AnimationTests.cs
// (15 [Fact] tests, xUnit -> GoogleTest, per-test fresh fixture instance -> TEST_F with SetUp()).
// TestAnimationFrame/TestAnimation mirror upstream's private test-only IAnimationFrame/IAnimation
// implementations; upstream constructs them via object-initializer syntax after a constructor
// taking only the frame array, so this port folds the initializer-set fields (FrameCount always
// set equal to frames.Length, IsLooping/IsReversed/IsPingPong) directly into TestAnimation's
// constructor instead.
#include "CNA/Extended/Animations/AnimationController.hpp"

#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::Animations
{
    using Microsoft::Xna::Framework::GameTime;

    namespace
    {
        class TestAnimationFrame : public IAnimationFrame
        {
        public:
            TestAnimationFrame(int frameIndex, TimeSpan duration) : frameIndex_(frameIndex), duration_(duration)
            {
            }

            [[nodiscard]] int getFrameIndexProperty() const override { return frameIndex_; }
            [[nodiscard]] TimeSpan getDurationProperty() const override { return duration_; }

        private:
            int frameIndex_;
            TimeSpan duration_;
        };

        class TestAnimation : public IAnimation
        {
        public:
            explicit TestAnimation(std::vector<IAnimationFrame*> frames, bool isLooping = false, bool isReversed = false, bool isPingPong = false)
                : name_("TestAnimation"), frames_(std::move(frames)), isLooping_(isLooping), isReversed_(isReversed), isPingPong_(isPingPong)
            {
            }

            [[nodiscard]] const std::string& getNameProperty() const override { return name_; }
            [[nodiscard]] const std::vector<IAnimationFrame*>& getFramesProperty() const override { return frames_; }
            [[nodiscard]] int getFrameCountProperty() const override { return static_cast<int>(frames_.size()); }
            [[nodiscard]] bool getIsLoopingProperty() const override { return isLooping_; }
            [[nodiscard]] bool getIsReversedProperty() const override { return isReversed_; }
            [[nodiscard]] bool getIsPingPongProperty() const override { return isPingPong_; }

        private:
            std::string name_;
            std::vector<IAnimationFrame*> frames_;
            bool isLooping_;
            bool isReversed_;
            bool isPingPong_;
        };
    }

    class AnimationControllerTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            frame0_ = std::make_unique<TestAnimationFrame>(0, TimeSpan::FromSeconds(1));
            frame1_ = std::make_unique<TestAnimationFrame>(1, TimeSpan::FromSeconds(1));
            animation_ = std::make_unique<TestAnimation>(std::vector<IAnimationFrame*>{frame0_.get(), frame1_.get()});
            controller_ = std::make_unique<AnimationController>(*animation_);
        }

        std::unique_ptr<TestAnimationFrame> frame0_;
        std::unique_ptr<TestAnimationFrame> frame1_;
        std::unique_ptr<TestAnimation> animation_;
        std::unique_ptr<AnimationController> controller_;
    };

    TEST_F(AnimationControllerTest, PlayThrowsExceptionForInvalidFrame)
    {
        EXPECT_THROW(controller_->Play(-1), System::ArgumentOutOfRangeException);
        EXPECT_THROW(controller_->Play(10), System::ArgumentOutOfRangeException);
    }

    TEST_F(AnimationControllerTest, PausePausesAnimation)
    {
        controller_->Play();
        const bool result = controller_->Pause();

        EXPECT_TRUE(result);
        EXPECT_TRUE(controller_->getIsPausedProperty());
    }

    TEST_F(AnimationControllerTest, PauseResetsFrameDurationWhenSpecified)
    {
        controller_->Play();
        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromSeconds(0.5));
        controller_->Update(gameTime);

        const bool result = controller_->Pause(true);

        EXPECT_TRUE(result);
        EXPECT_EQ(animation_->getFramesProperty()[0]->getDurationProperty(), controller_->getCurrentFrameTimeRemainingProperty());
    }

    TEST_F(AnimationControllerTest, UnpauseResumesAnimation)
    {
        controller_->Play();
        controller_->Pause();
        const bool result = controller_->Unpause();

        EXPECT_TRUE(result);
        EXPECT_FALSE(controller_->getIsPausedProperty());
    }

    TEST_F(AnimationControllerTest, UnpauseAdvancesFrameWhenSpecified)
    {
        controller_->Play();
        controller_->Pause();
        const bool result = controller_->Unpause(true);

        EXPECT_TRUE(result);
        EXPECT_EQ(1, controller_->getCurrentFrameProperty());
    }

    TEST_F(AnimationControllerTest, StopStopsAnimation)
    {
        controller_->Play();
        const bool result = controller_->Stop();

        EXPECT_TRUE(result);
        EXPECT_FALSE(controller_->getIsAnimatingProperty());
    }

    TEST_F(AnimationControllerTest, UpdateAdvancesFrame)
    {
        controller_->Play();
        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromSeconds(1.1));
        controller_->Update(gameTime);

        EXPECT_EQ(1, controller_->getCurrentFrameProperty());
    }

    TEST_F(AnimationControllerTest, UpdateTriggersAnimationEvents)
    {
        bool frameBeginTriggered = false;
        bool frameEndTriggered = false;
        bool animationLoopTriggered = false;
        bool animationCompletedTriggered = false;

        controller_->getOnAnimationEventEvent() += [&](IAnimationController*, AnimationEventTrigger trigger)
        {
            switch (trigger)
            {
                case AnimationEventTrigger::FrameBegin:
                    frameBeginTriggered = true;
                    break;
                case AnimationEventTrigger::FrameEnd:
                    frameEndTriggered = true;
                    break;
                case AnimationEventTrigger::AnimationLoop:
                    animationLoopTriggered = true;
                    break;
                case AnimationEventTrigger::AnimationCompleted:
                    animationCompletedTriggered = true;
                    break;
                default:
                    break;
            }
        };

        controller_->Play();
        controller_->Update(GameTime(TimeSpan::Zero, TimeSpan::FromSeconds(1.1)));

        EXPECT_TRUE(frameBeginTriggered);
        EXPECT_TRUE(frameEndTriggered);

        controller_->setIsLoopingProperty(true);
        controller_->Update(GameTime(TimeSpan::Zero, TimeSpan::FromSeconds(1.1)));
        EXPECT_TRUE(animationLoopTriggered);

        controller_->setIsLoopingProperty(false);
        controller_->Update(GameTime(TimeSpan::Zero, TimeSpan::FromSeconds(2)));
        EXPECT_TRUE(animationCompletedTriggered);
    }

    TEST_F(AnimationControllerTest, UpdateLoopsAnimation)
    {
        controller_->setIsLoopingProperty(true);
        controller_->Play();
        controller_->Update(GameTime(TimeSpan::Zero, TimeSpan::FromSeconds(2.1)));

        EXPECT_EQ(0, controller_->getCurrentFrameProperty());
    }

    TEST_F(AnimationControllerTest, ResetResetsAnimation)
    {
        controller_->Play();
        controller_->Reset();

        EXPECT_FALSE(controller_->getIsAnimatingProperty());
        EXPECT_TRUE(controller_->getIsPausedProperty());
        EXPECT_EQ(0, controller_->getCurrentFrameProperty());
    }

    TEST_F(AnimationControllerTest, SetFrameChangesCurrentFrame)
    {
        controller_->SetFrame(1);

        EXPECT_EQ(1, controller_->getCurrentFrameProperty());
        EXPECT_EQ(animation_->getFramesProperty()[1]->getDurationProperty(), controller_->getCurrentFrameTimeRemainingProperty());
    }

    TEST_F(AnimationControllerTest, SetFrameThrowsExceptionForInvalidFrame)
    {
        EXPECT_THROW(controller_->SetFrame(-1), System::ArgumentOutOfRangeException);
        EXPECT_THROW(controller_->SetFrame(10), System::ArgumentOutOfRangeException);
    }

    TEST_F(AnimationControllerTest, DisposeDisposesAnimation)
    {
        controller_->Dispose();

        EXPECT_TRUE(controller_->getIsDisposedProperty());
    }

    TEST_F(AnimationControllerTest, UpdateWithTimeSpanAdvancesFrame)
    {
        controller_->Play();
        controller_->Update(TimeSpan::FromSeconds(1.1));

        EXPECT_EQ(1, controller_->getCurrentFrameProperty());
    }

    TEST_F(AnimationControllerTest, UpdateWithGameTimeProducesSameResultAsTimeSpan)
    {
        TestAnimationFrame frame0(0, TimeSpan::FromSeconds(1));
        TestAnimationFrame frame1(1, TimeSpan::FromSeconds(1));
        TestAnimation animation(std::vector<IAnimationFrame*>{&frame0, &frame1});

        AnimationController controller1(animation);
        AnimationController controller2(animation);

        controller1.Play();
        controller2.Play();

        const TimeSpan elapsed = TimeSpan::FromSeconds(0.5);
        const GameTime gameTime(TimeSpan::Zero, elapsed);

        controller1.Update(gameTime);
        controller2.Update(elapsed);

        EXPECT_EQ(controller1.getCurrentFrameProperty(), controller2.getCurrentFrameProperty());
        EXPECT_EQ(controller1.getCurrentFrameTimeRemainingProperty(), controller2.getCurrentFrameTimeRemainingProperty());
    }
}
