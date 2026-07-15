// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for DebugDrawComponentEXT/DebugDrawSystemEXT (see
// 3d.md/plan3d.md) -- there is no upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/DebugDrawSystemEXT.hpp"

#include "CNA/Extended/ECS/World.hpp"
#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/DebugDrawComponentEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "System/TimeSpan.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;
    using Microsoft::Xna::Framework::BoundingFrustum;
    using Microsoft::Xna::Framework::BoundingSphere;
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::RasterizerState;
    using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using ECS::World;
    using ECS::WorldBuilder;
    using System::TimeSpan;

    TEST(DebugDrawComponentEXTTests, AddDebugBoxLinesEXT_AppendsTwelveEdges)
    {
        std::vector<DebugLineEXT> lines;
        const BoundingBox box(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));

        AddDebugBoxLinesEXT(lines, box, Color::White);

        EXPECT_EQ(lines.size(), 12u);
        for (const DebugLineEXT& line : lines)
        {
            EXPECT_NE(line.Start, line.End);
        }
    }

    TEST(DebugDrawComponentEXTTests, AddDebugFrustumLinesEXT_AppendsTwelveEdges)
    {
        std::vector<DebugLineEXT> lines;
        const Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 10.0f), Vector3::Zero, Vector3::Up);
        const Matrix projection = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, 800.0f / 480.0f, 0.1f, 100.0f);
        const BoundingFrustum frustum(view * projection);

        AddDebugFrustumLinesEXT(lines, frustum, Color::Yellow);

        EXPECT_EQ(lines.size(), 12u);
    }

    TEST(DebugDrawComponentEXTTests, AddDebugSphereLinesEXT_AppendsThreeSegmentsPerCircle)
    {
        std::vector<DebugLineEXT> lines;
        const BoundingSphere sphere(Vector3::Zero, 2.0f);

        AddDebugSphereLinesEXT(lines, sphere, Color::White, 16);

        // 3 great circles (XY/XZ/YZ), 16 segments each.
        EXPECT_EQ(lines.size(), 3u * 16u);
    }

    TEST(DebugDrawComponentEXTTests, AddDebugSphereLinesEXT_EndpointsLieOnSphereSurface)
    {
        std::vector<DebugLineEXT> lines;
        const BoundingSphere sphere(Vector3(1.0f, 2.0f, 3.0f), 5.0f);

        AddDebugSphereLinesEXT(lines, sphere, Color::White, 24);

        for (const DebugLineEXT& line : lines)
        {
            EXPECT_NEAR((line.Start - sphere.Center).Length(), sphere.Radius, 1e-3f);
            EXPECT_NEAR((line.End - sphere.Center).Length(), sphere.Radius, 1e-3f);
        }
    }

    TEST(DebugDrawComponentEXTTests, AddDebugSphereLinesEXT_ClampsTooFewSegments)
    {
        std::vector<DebugLineEXT> lines;
        const BoundingSphere sphere(Vector3::Zero, 1.0f);

        AddDebugSphereLinesEXT(lines, sphere, Color::White, 1);

        // Clamped to the minimum of 3 segments per circle, not 1.
        EXPECT_EQ(lines.size(), 3u * 3u);
    }

    namespace
    {
        class DebugDrawSystemEXTTest : public ::testing::Test
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

        bool AnyPixelNotBlack(const std::vector<Color>& pixels)
        {
            return std::any_of(pixels.begin(), pixels.end(),
                                [](const Color& p) { return p.getRProperty() > 0 || p.getGProperty() > 0 || p.getBProperty() > 0; });
        }
    }

    TEST_F(DebugDrawSystemEXTTest, LinesFromActiveEntity_AreDrawn)
    {
        WorldBuilder builder;
        builder.AddSystem(std::make_unique<DebugDrawSystemEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        DebugDrawComponentEXT debugComponent;
        AddDebugBoxLinesEXT(debugComponent.LinesEXT, BoundingBox(Vector3(-3.0f, -3.0f, -3.0f), Vector3(3.0f, 3.0f, 3.0f)), Color::White);
        entity.Attach(&debugComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        (void)width;

        EXPECT_TRUE(AnyPixelNotBlack(pixels));
    }

    TEST_F(DebugDrawSystemEXTTest, SphereLinesFromActiveEntity_AreDrawn)
    {
        WorldBuilder builder;
        builder.AddSystem(std::make_unique<DebugDrawSystemEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        DebugDrawComponentEXT debugComponent;
        AddDebugSphereLinesEXT(debugComponent.LinesEXT, BoundingSphere(Vector3::Zero, 3.0f), Color::White);
        entity.Attach(&debugComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        (void)width;

        EXPECT_TRUE(AnyPixelNotBlack(pixels));
    }

    TEST_F(DebugDrawSystemEXTTest, NoActiveEntities_DrawsNothing)
    {
        WorldBuilder builder;
        builder.AddSystem(std::make_unique<DebugDrawSystemEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        (void)width;

        EXPECT_FALSE(AnyPixelNotBlack(pixels));
    }
}
