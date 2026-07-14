// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for World3DScreenEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here. Real headless render test, matching
// TilemapIntegrationTests.cpp's/RenderSystem3DEXTTests.cpp's established idiom -- proves
// the full Screen lifecycle (Initialize -> Update -> Draw) actually reaches a real
// World3DEXT render system and draws something.
#include "CNA/Extended/World3DEXT/World3DScreenEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/ECS/World.hpp"
#include "CNA/Extended/World3DEXT/CubeMeshComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/CubeMeshRenderSystemEXT.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
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
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::RasterizerState;
    using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using System::TimeSpan;

    namespace
    {
        // A minimal real screen: one CubeMeshRenderSystemEXT, one cube entity, added via
        // ConfigureWorldEXT -- proves derived screens can wire up their own World3DEXT
        // systems through the intended extension point.
        class TestScreenEXT final : public World3DScreenEXT
        {
        public:
            TestScreenEXT(GraphicsDevice& graphicsDevice, Texture2D& texture) : graphicsDevice_(&graphicsDevice), texture_(&texture) {}

            void Initialize() override
            {
                World3DScreenEXT::Initialize();

                ECS::Entity& entity = GetWorld3DEXT().CreateEntity();
                cubeComponent_.TextureEXT = texture_;
                cubeComponent_.SizeEXT = Vector3(4.0f, 4.0f, 4.0f);
                cubeComponent_.TintEXT = Color(0, 200, 0, 255);
                entity.Attach(&cubeComponent_);
            }

        protected:
            void ConfigureWorldEXT(ECS::WorldBuilder& builder) override
            {
                builder.AddSystem(std::make_unique<CubeMeshRenderSystemEXT>(*graphicsDevice_, GetCamera3DEXT()));
            }

        private:
            GraphicsDevice* graphicsDevice_;
            Texture2D* texture_;
            CubeMeshComponentEXT cubeComponent_;
        };
    }

    TEST(World3DScreenEXTTests, InitializeUpdateDraw_DrawsThroughConfiguredWorldSystems)
    {
        GraphicsDevice graphicsDevice;
        graphicsDevice.setViewportProperty(Viewport(0, 0, 800, 480));
        graphicsDevice.setRasterizerStateProperty(RasterizerState::CullNone);

        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);

        TestScreenEXT screen(graphicsDevice, whiteTex);
        screen.Initialize();

        screen.GetCamera3DEXT().setPositionProperty(Vector3(0.0f, 0.0f, 10.0f));
        screen.GetCamera3DEXT().setTargetProperty(Vector3::Zero);
        screen.GetCamera3DEXT().setFieldOfViewProperty(MathHelper::PiOver4);
        screen.GetCamera3DEXT().setAspectRatioProperty(800.0f / 480.0f);
        screen.GetCamera3DEXT().setNearPlaneProperty(0.1f);
        screen.GetCamera3DEXT().setFarPlaneProperty(100.0f);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        screen.Update(gameTime);

        const int width = graphicsDevice.getViewportProperty().getWidthProperty();
        const int height = graphicsDevice.getViewportProperty().getHeightProperty();
        RenderTarget2D rt(graphicsDevice, width, height);
        graphicsDevice.SetRenderTarget(&rt);
        graphicsDevice.Clear(Color::Black);

        screen.Draw(gameTime);

        std::vector<Color> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), Color::Transparent);
        graphicsDevice.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));
        graphicsDevice.SetRenderTarget(nullptr);

        const Color center = pixels[static_cast<std::size_t>(height / 2) * static_cast<std::size_t>(width) + static_cast<std::size_t>(width / 2)];
        EXPECT_GT(center.getGProperty(), 0);
        EXPECT_EQ(center.getRProperty(), 0);
        EXPECT_EQ(center.getBProperty(), 0);
    }

    TEST(World3DScreenEXTTests, GetCamera3DEXT_IsUsableBeforeInitialize)
    {
        // A fresh screen's camera should be independently configurable even before
        // Initialize() builds the World (the two are unrelated -- the camera is a plain
        // member, not part of the World).
        GraphicsDevice graphicsDevice;
        Texture2D texture(graphicsDevice, 1, 1);
        TestScreenEXT screen(graphicsDevice, texture);

        screen.GetCamera3DEXT().setPositionProperty(Vector3(1.0f, 2.0f, 3.0f));
        EXPECT_EQ(screen.GetCamera3DEXT().getPositionProperty(), Vector3(1.0f, 2.0f, 3.0f));
    }
}
