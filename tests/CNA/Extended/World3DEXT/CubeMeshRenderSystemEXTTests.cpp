// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for CubeMeshRenderSystemEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here. Real headless render tests, matching
// TilemapIntegrationTests.cpp's/RenderSystem3DEXTTests.cpp's established idiom.
#include "CNA/Extended/World3DEXT/CubeMeshRenderSystemEXT.hpp"

#include "CNA/Extended/ECS/World.hpp"
#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/CubeMeshComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "System/TimeSpan.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::DepthFormat;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::RasterizerState;
    using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
    using Microsoft::Xna::Framework::Graphics::SurfaceFormat;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using ECS::World;
    using ECS::WorldBuilder;
    using System::TimeSpan;

    namespace
    {
        class CubeMeshRenderSystemEXTTest : public ::testing::Test
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
    }

    TEST_F(CubeMeshRenderSystemEXTTest, VisibleCube_IsDrawnWithTintColor)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<CubeMeshRenderSystemEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        CubeMeshComponentEXT cubeComponent;
        cubeComponent.TextureEXT = &whiteTex;
        cubeComponent.SizeEXT = Vector3(4.0f, 4.0f, 4.0f);
        cubeComponent.TintEXT = Color(0, 200, 0, 255);
        entity.Attach(&cubeComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        const int height = graphicsDevice.getViewportProperty().getHeightProperty();
        const Color center = GetPixel(pixels, width, width / 2, height / 2);

        EXPECT_GT(center.getGProperty(), 0);
        EXPECT_EQ(center.getRProperty(), 0);
        EXPECT_EQ(center.getBProperty(), 0);
    }

    TEST_F(CubeMeshRenderSystemEXTTest, CubeBehindFarPlane_IsCulledAndNotDrawn)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<CubeMeshRenderSystemEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        Transform3ComponentEXT transformComponent;
        transformComponent.TransformEXT.setPositionProperty(Vector3(0.0f, 0.0f, -10000.0f));
        entity.Attach(&transformComponent);

        CubeMeshComponentEXT cubeComponent;
        cubeComponent.TextureEXT = &whiteTex;
        cubeComponent.SizeEXT = Vector3(4.0f, 4.0f, 4.0f);
        entity.Attach(&cubeComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        (void)width;

        EXPECT_TRUE(std::none_of(pixels.begin(), pixels.end(),
                                  [](const Color& p) { return p.getRProperty() > 0 || p.getGProperty() > 0 || p.getBProperty() > 0; }));
    }

    // A-07 regression test (audit.md): locks in the missing-depth-buffer fix this session
    // found and fixed in world3d_demo (see NEXT.md) -- RenderTarget2D's 3-arg constructor
    // has no depth buffer at all (its own doc comment says so), which this fixture's
    // RenderToPixels helper deliberately does NOT use (see the 7-arg constructor call
    // below) specifically so this test can tell a real depth test apart from pure
    // draw-order compositing. Two same-screen-silhouette cubes, drawn in both submission
    // orders: the nearer cube's color must win either way.
    TEST_F(CubeMeshRenderSystemEXTTest, TwoOverlappingCubes_NearerCubeWinsRegardlessOfSubmissionOrder)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);

        const auto renderWithOrder = [&](bool nearFirst) {
            WorldBuilder builder;
            builder.AddSystem(std::make_unique<CubeMeshRenderSystemEXT>(graphicsDevice, camera));
            const std::unique_ptr<World> world = builder.Build();
            world->Initialize();

            // Transform3ComponentEXT/CubeMeshComponentEXT are non-owning-pointer targets
            // (Entity::Attach stores a raw pointer) -- declared here, in renderWithOrder's
            // own scope, so they stay alive through world->Update()/Draw() below rather than
            // dangling if they were locals inside a nested per-cube lambda.
            Transform3ComponentEXT transforms[2];
            CubeMeshComponentEXT cubes[2];

            // Near cube (Z=3, closer to the camera at Z=10) is green; far cube (Z=-3) is
            // red. Same X/Y and size on both, so their screen-space silhouettes coincide.
            const float zValues[2] = {nearFirst ? 3.0f : -3.0f, nearFirst ? -3.0f : 3.0f};
            const Color tints[2] = {nearFirst ? Color(0, 200, 0, 255) : Color(200, 0, 0, 255),
                                     nearFirst ? Color(200, 0, 0, 255) : Color(0, 200, 0, 255)};
            for (int i = 0; i < 2; ++i)
            {
                ECS::Entity& entity = world->CreateEntity();
                transforms[i].TransformEXT.setPositionProperty(Vector3(0.0f, 0.0f, zValues[i]));
                entity.Attach(&transforms[i]);

                cubes[i].TextureEXT = &whiteTex;
                cubes[i].SizeEXT = Vector3(4.0f, 4.0f, 4.0f);
                cubes[i].TintEXT = tints[i];
                entity.Attach(&cubes[i]);
            }

            GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
            world->Update(gameTime);

            const int width = graphicsDevice.getViewportProperty().getWidthProperty();
            const int height = graphicsDevice.getViewportProperty().getHeightProperty();

            // Uses the 7-arg RenderTarget2D constructor directly (not this fixture's
            // depth-less RenderToPixels helper) so this test actually exercises a real
            // depth buffer, per this test's own file-header rationale.
            RenderTarget2D rt(graphicsDevice, width, height, false, SurfaceFormat::Color, DepthFormat::Depth24);
            graphicsDevice.SetRenderTarget(&rt);
            graphicsDevice.Clear(Color::Black, 1.0f);

            world->Draw(gameTime);

            std::vector<Color> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), Color::Transparent);
            graphicsDevice.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));
            graphicsDevice.SetRenderTarget(nullptr);

            return GetPixel(pixels, width, width / 2, height / 2);
        };

        const Color nearFirstCenter = renderWithOrder(true);
        EXPECT_GT(nearFirstCenter.getGProperty(), 0);
        EXPECT_EQ(nearFirstCenter.getRProperty(), 0);

        const Color farFirstCenter = renderWithOrder(false);
        EXPECT_GT(farFirstCenter.getGProperty(), 0);
        EXPECT_EQ(farFirstCenter.getRProperty(), 0);
    }
}
