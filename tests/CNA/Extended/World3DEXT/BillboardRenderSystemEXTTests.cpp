// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for BillboardRenderSystemEXT (see 3d.md/plan3d.md) -- there is
// no upstream MonoGame.Extended test suite to port here. Real headless render tests,
// matching TilemapIntegrationTests.cpp's/RenderSystem3DEXTTests.cpp's established idiom.
#include "CNA/Extended/World3DEXT/BillboardRenderSystemEXT.hpp"

#include "CNA/Extended/ECS/World.hpp"
#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/World3DEXT/BillboardComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/BillboardMeshEXT.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "System/TimeSpan.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::RasterizerState;
    using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::VertexBuffer;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using ECS::World;
    using ECS::WorldBuilder;
    using System::TimeSpan;

    namespace
    {
        class BillboardRenderSystemEXTTest : public ::testing::Test
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

    TEST_F(BillboardRenderSystemEXTTest, VisibleBillboard_IsDrawnWithTintColor)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);
        const std::unique_ptr<VertexBuffer> quad = BuildBillboardQuadVertexBufferEXT(graphicsDevice, RectangleF(0.0f, 0.0f, 1.0f, 1.0f));

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<BillboardRenderSystemEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        BillboardComponentEXT billboardComponent;
        billboardComponent.TextureEXT = &whiteTex;
        billboardComponent.VertexBufferEXT = quad.get();
        billboardComponent.SizeEXT = Vector2(4.0f, 4.0f);
        billboardComponent.TintEXT = Color(0, 0, 220, 255);
        entity.Attach(&billboardComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        const int height = graphicsDevice.getViewportProperty().getHeightProperty();
        const Color center = GetPixel(pixels, width, width / 2, height / 2);

        EXPECT_GT(center.getBProperty(), 0);
        EXPECT_EQ(center.getRProperty(), 0);
        EXPECT_EQ(center.getGProperty(), 0);
    }

    TEST_F(BillboardRenderSystemEXTTest, CameraOrbitsAroundBillboard_StillShowsFrontFaceEveryFrame)
    {
        // A billboard always faces the camera -- unlike a fixed quad, moving the camera
        // around it (not just toward/away) should not change what's rendered, since
        // Matrix::CreateBillboard recomputes orientation every frame from the current
        // camera position.
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);
        const std::unique_ptr<VertexBuffer> quad = BuildBillboardQuadVertexBufferEXT(graphicsDevice, RectangleF(0.0f, 0.0f, 1.0f, 1.0f));

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<BillboardRenderSystemEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        BillboardComponentEXT billboardComponent;
        billboardComponent.TextureEXT = &whiteTex;
        billboardComponent.VertexBufferEXT = quad.get();
        billboardComponent.SizeEXT = Vector2(4.0f, 4.0f);
        billboardComponent.TintEXT = Color(0, 0, 220, 255);
        entity.Attach(&billboardComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);

        camera.setPositionProperty(Vector3(0.0f, 0.0f, 10.0f));
        const auto [frontPixels, frontWidth] = RenderToPixels(*world, gameTime);

        camera.setPositionProperty(Vector3(10.0f, 0.0f, 0.0f));
        camera.setAspectRatioProperty(800.0f / 480.0f);
        const auto [sidePixels, sideWidth] = RenderToPixels(*world, gameTime);

        const int height = graphicsDevice.getViewportProperty().getHeightProperty();
        const Color frontCenter = GetPixel(frontPixels, frontWidth, frontWidth / 2, height / 2);
        const Color sideCenter = GetPixel(sidePixels, sideWidth, sideWidth / 2, height / 2);

        EXPECT_EQ(frontCenter, sideCenter);
        EXPECT_GT(sideCenter.getBProperty(), 0);
    }
}
