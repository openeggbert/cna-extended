// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for AnimatedBillboardSystemEXT (see 3d.md/plan3d.md) -- there is
// no upstream MonoGame.Extended test suite to port here. Reuses Graphics::Texture2DAtlas/
// SpriteSheet/SpriteSheetAnimationBuilder exactly as AnimatedSprite's own doc example does
// (see AnimatedSprite.hpp), matching this component's design as a thin driver over already-
// tested infrastructure, not a new animation system.
#include "CNA/Extended/World3DEXT/AnimatedBillboardSystemEXT.hpp"

#include "CNA/Extended/Animations/AnimationController.hpp"
#include "CNA/Extended/ECS/World.hpp"
#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/Graphics/SpriteSheet.hpp"
#include "CNA/Extended/Graphics/SpriteSheetAnimationBuilder.hpp"
#include "CNA/Extended/Graphics/Texture2DAtlas.hpp"
#include "CNA/Extended/World3DEXT/AnimatedBillboardComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/BillboardComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/BillboardMeshEXT.hpp"
#include "CNA/Extended/World3DEXT/BillboardRenderSystemEXT.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
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
    using Animations::AnimationController;
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
    using Graphics::SpriteSheet;
    using Graphics::SpriteSheetAnimationBuilder;
    using Graphics::Texture2DAtlas;
    using System::TimeSpan;

    namespace
    {
        // 4x2 texture: left half (frame0, x:0..1) solid red, right half (frame1, x:2..3)
        // solid green -- so playing "anim" and sampling the drawn billboard's pixel color
        // directly shows which frame is active.
        Texture2D BuildTwoFrameTexture(GraphicsDevice& graphicsDevice)
        {
            Texture2D texture(graphicsDevice, 4, 2);
            const std::vector<Color> pixels = {
                Color::Red, Color::Red, Color::Green, Color::Green,
                Color::Red, Color::Red, Color::Green, Color::Green,
            };
            texture.SetData(pixels.data(), static_cast<int>(pixels.size()));
            return texture;
        }

        class AnimatedBillboardSystemEXTTest : public ::testing::Test
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

    TEST_F(AnimatedBillboardSystemEXTTest, FrameAdvance_SwitchesTextureAndUv)
    {
        Texture2D texture = BuildTwoFrameTexture(graphicsDevice);
        Texture2DAtlas atlas("test-atlas", &texture);
        atlas.CreateRegion(0, 0, 2, 2, "frame0");
        atlas.CreateRegion(2, 0, 2, 2, "frame1");
        SpriteSheet sheet("test-sheet", atlas);
        sheet.DefineAnimation("anim", [](SpriteSheetAnimationBuilder& builder) {
            builder.AddFrame("frame0", TimeSpan::FromSeconds(0.1)).AddFrame("frame1", TimeSpan::FromSeconds(0.1)).IsLooping(true);
        });

        AnimationController controller(*sheet.GetAnimation("anim"));
        const std::unique_ptr<VertexBuffer> quad = BuildBillboardQuadVertexBufferEXT(graphicsDevice, RectangleF(0.0f, 0.0f, 0.5f, 1.0f));

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<AnimatedBillboardSystemEXT>());
        builder.AddSystem(std::make_unique<BillboardRenderSystemEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        BillboardComponentEXT billboardComponent;
        billboardComponent.VertexBufferEXT = quad.get();
        billboardComponent.SizeEXT = Vector2(4.0f, 4.0f);
        entity.Attach(&billboardComponent);

        AnimatedBillboardComponentEXT animatedComponent;
        animatedComponent.SpriteSheetEXT = &sheet;
        animatedComponent.ControllerEXT = &controller;
        entity.Attach(&animatedComponent);

        // Frame 0 (red): elapsed 0, still at the animation's first frame.
        GameTime gameTime(TimeSpan::Zero, TimeSpan::Zero);
        world->Update(gameTime);
        const auto [frame0Pixels, frame0Width] = RenderToPixels(*world, gameTime);
        const int height = graphicsDevice.getViewportProperty().getHeightProperty();
        const Color frame0Center = GetPixel(frame0Pixels, frame0Width, frame0Width / 2, height / 2);

        EXPECT_EQ(billboardComponent.TextureEXT, &texture);
        EXPECT_GT(frame0Center.getRProperty(), 0);
        EXPECT_EQ(frame0Center.getGProperty(), 0);

        // Frame 1 (green): elapsed past frame0's 0.1s duration.
        gameTime = GameTime(TimeSpan::FromSeconds(0.15), TimeSpan::FromSeconds(0.15));
        world->Update(gameTime);
        const auto [frame1Pixels, frame1Width] = RenderToPixels(*world, gameTime);
        const Color frame1Center = GetPixel(frame1Pixels, frame1Width, frame1Width / 2, height / 2);

        EXPECT_EQ(frame1Center.getRProperty(), 0);
        EXPECT_GT(frame1Center.getGProperty(), 0);
    }
}
