// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for AnimationSystem3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
//
// The real-render test's SkinnedEffect/lighting setup (ambient white + a single enabled
// directional light facing the same way as the quad's normal, RasterizerState::CullNone)
// is copied from cna's own proven-working real-rendering recipe
// (cna/examples/avatar_tint_routing_integration_test.cpp, AvatarRenderer::DrawRealEXT's
// exact sequence) rather than re-derived from scratch.
#include "CNA/Extended/World3DEXT/AnimationSystem3DEXT.hpp"

#include "CNA/Extended/ECS/World.hpp"
#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/RenderSystem3DEXT.hpp"
#include "CNA/Extended/World3DEXT/SkinnedModelComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTextureSkinned.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "System/TimeSpan.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Vector4;
    using Microsoft::Xna::Framework::Graphics::AnimationClipEXT;
    using Microsoft::Xna::Framework::Graphics::BoneTrackEXT;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::IndexBuffer;
    using Microsoft::Xna::Framework::Graphics::KeyframeEXT;
    using Microsoft::Xna::Framework::Graphics::ModelMeshPart;
    using Microsoft::Xna::Framework::Graphics::RasterizerState;
    using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
    using Microsoft::Xna::Framework::Graphics::SkinnedEffect;
    using Microsoft::Xna::Framework::Graphics::SkinnedModelEXT;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::VertexBuffer;
    using Microsoft::Xna::Framework::Graphics::VertexPositionNormalTextureSkinned;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using ECS::World;
    using ECS::WorldBuilder;
    using System::TimeSpan;

    namespace
    {
        // 2-bone rig: bone 0 = static root, bone 1 = child of bone 0, moving from (0,0,0)
        // at t=0s to (5,0,0) at t=1s. All geometry is 100% weighted to bone 1, so the whole
        // triangle visibly translates between the two clip endpoints.
        std::unique_ptr<SkinnedModelEXT> MakeMovingTriangleModel(GraphicsDevice& graphicsDevice, Texture2D texture)
        {
            auto model = std::make_unique<SkinnedModelEXT>();
            model->BoneCount = 2;
            model->ParentBoneIndices = {-1, 0};
            model->BindPoseLocal = {Matrix::getIdentityProperty(), Matrix::getIdentityProperty()};
            model->InverseBindPoseGlobal = {Matrix::getIdentityProperty(), Matrix::getIdentityProperty()};

            BoneTrackEXT track;
            track.BoneIndex = 1;
            track.Keys.push_back(KeyframeEXT{TimeSpan::Zero, Vector3(0.0f, 0.0f, 0.0f)});
            track.Keys.push_back(KeyframeEXT{TimeSpan::FromSeconds(1.0), Vector3(5.0f, 0.0f, 0.0f)});

            AnimationClipEXT clip;
            clip.Duration = TimeSpan::FromSeconds(1.0);
            clip.Tracks.push_back(track);
            model->Clips["Move"] = clip;

            const std::array<std::uint8_t, 4> boneOne{1, 0, 0, 0};
            const Vector4 fullWeightToOne(1.0f, 0.0f, 0.0f, 0.0f);
            const VertexPositionNormalTextureSkinned verts[3] = {
                {Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.5f, 0.0f), fullWeightToOne, boneOne},
                {Vector3(1.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 1.0f), fullWeightToOne, boneOne},
                {Vector3(-1.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 1.0f), fullWeightToOne, boneOne},
            };
            const std::uint16_t indices[3] = {0, 1, 2};

            auto vb = std::make_unique<VertexBuffer>(graphicsDevice, 3);
            vb->SetData(verts, 3);
            auto ib = std::make_unique<IndexBuffer>(graphicsDevice, 3);
            ib->SetData(indices, 3);
            auto part = std::make_unique<ModelMeshPart>(vb.get(), ib.get(), 3, 1, 0, 0);
            model->AddPartEXT("Triangle", std::move(vb), std::move(ib), std::move(part), texture);

            return model;
        }

        class AnimationSystem3DEXTTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                graphicsDevice.setViewportProperty(Viewport(0, 0, 800, 480));
                graphicsDevice.setRasterizerStateProperty(RasterizerState::CullNone);

                camera.setPositionProperty(Vector3(0.0f, 0.0f, 10.0f));
                camera.setTargetProperty(Vector3::Zero);
                camera.setFieldOfViewProperty(MathHelper::PiOver4);
                camera.setAspectRatioProperty(800.0f / 480.0f);
                camera.setNearPlaneProperty(0.1f);
                camera.setFarPlaneProperty(100.0f);
            }

            std::pair<std::vector<Color>, int> RenderToPixels(World& world, const GameTime& gameTime)
            {
                const int width = graphicsDevice.getViewportProperty().getWidthProperty();
                const int height = graphicsDevice.getViewportProperty().getHeightProperty();

                RenderTarget2D rt(graphicsDevice, width, height);
                graphicsDevice.SetRenderTarget(&rt);
                graphicsDevice.Clear(Color::Black);

                world.Draw(gameTime);

                std::vector<Color> pixels(
                    static_cast<std::size_t>(width) * static_cast<std::size_t>(height), Color::Transparent);
                graphicsDevice.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));

                graphicsDevice.SetRenderTarget(nullptr);

                return {std::move(pixels), width};
            }

            GraphicsDevice graphicsDevice;
            Camera3DEXT camera;
        };

        Color GetPixel(const std::vector<Color>& pixels, int width, int x, int y)
        {
            return pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)];
        }

        bool AnyPixelNotBlack(const std::vector<Color>& pixels)
        {
            return std::any_of(pixels.begin(), pixels.end(),
                                [](const Color& p) { return p.getRProperty() > 0 || p.getGProperty() > 0 || p.getBProperty() > 0; });
        }
    }

    TEST_F(AnimationSystem3DEXTTest, Update_AdvancesPositionAndRecomputesBoneTransforms)
    {
        Texture2D whiteTex = Texture2D::CreateFromPixels(graphicsDevice, 1, 1, std::vector<std::uint8_t>{255, 255, 255, 255});
        std::unique_ptr<SkinnedModelEXT> model = MakeMovingTriangleModel(graphicsDevice, whiteTex);
        SkinnedEffect effect(graphicsDevice);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<AnimationSystem3DEXT>());
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        SkinnedModelComponentEXT component;
        component.ModelEXT = model.get();
        component.EffectEXT = &effect;
        component.ClipNameEXT = "Move";
        component.LoopEXT = false;
        entity.Attach(&component);

        EXPECT_TRUE(component.BoneTransformsEXT.empty());

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromSeconds(0.5));
        world->Update(gameTime);

        ASSERT_EQ(component.BoneTransformsEXT.size(), 2u);
        // Bone 1's world transform should now translate by roughly (2.5, 0, 0) -- halfway
        // through the 0->5 unit move over the clip's 1-second duration.
        EXPECT_NEAR(component.BoneTransformsEXT[1].M41, 2.5f, 0.01f);
        EXPECT_EQ(component.PositionEXT, TimeSpan::FromSeconds(0.5));
    }

    TEST_F(AnimationSystem3DEXTTest, DifferentAnimationTimes_ProduceDifferentPixelOutput)
    {
        Texture2D whiteTex = Texture2D::CreateFromPixels(graphicsDevice, 1, 1, std::vector<std::uint8_t>{255, 255, 255, 255});
        std::unique_ptr<SkinnedModelEXT> model = MakeMovingTriangleModel(graphicsDevice, whiteTex);
        SkinnedEffect effect(graphicsDevice);
        effect.setDiffuseColorProperty(Vector3(1.0f, 0.0f, 0.0f));
        effect.setAmbientLightColorProperty(Vector3::One);
        effect.EnableDefaultLighting();
        effect.getDirectionalLight0Property().setEnabledProperty(true);
        effect.getDirectionalLight0Property().setDirectionProperty(Vector3(0.0f, 0.0f, -1.0f));
        effect.getDirectionalLight0Property().setDiffuseColorProperty(Vector3::One);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<AnimationSystem3DEXT>());
        builder.AddSystem(std::make_unique<RenderSystem3DEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        SkinnedModelComponentEXT component;
        component.ModelEXT = model.get();
        component.EffectEXT = &effect;
        component.ClipNameEXT = "Move";
        component.LoopEXT = false;
        component.BoundsEXT = Microsoft::Xna::Framework::BoundingSphere(Vector3::Zero, 10.0f);
        entity.Attach(&component);

        // Frame 1: position 0 -> triangle centered at the origin, visible at screen center.
        GameTime gameTime(TimeSpan::Zero, TimeSpan::Zero);
        world->Update(gameTime);
        const auto [framePixels0, width0] = RenderToPixels(*world, gameTime);
        const int height0 = graphicsDevice.getViewportProperty().getHeightProperty();
        const Color center0 = GetPixel(framePixels0, width0, width0 / 2, height0 / 2);

        // Frame 2: position 1s -> triangle translated 5 units on X, no longer covering the
        // screen center.
        gameTime = GameTime(TimeSpan::FromSeconds(1.0), TimeSpan::FromSeconds(1.0));
        world->Update(gameTime);
        const auto [framePixels1, width1] = RenderToPixels(*world, gameTime);
        const int height1 = graphicsDevice.getViewportProperty().getHeightProperty();
        const Color center1 = GetPixel(framePixels1, width1, width1 / 2, height1 / 2);

        EXPECT_TRUE(AnyPixelNotBlack(framePixels0));
        EXPECT_GT(center0.getRProperty(), 0);
        EXPECT_NE(center0, center1);
    }
}
