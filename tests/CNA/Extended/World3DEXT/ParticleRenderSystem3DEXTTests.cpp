// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for ParticleRenderSystem3DEXT (see 3d.md/plan3d.md) -- there is
// no upstream MonoGame.Extended test suite to port here. Real headless render test,
// matching TilemapIntegrationTests.cpp's/RenderSystem3DEXTTests.cpp's established idiom.
#include "CNA/Extended/World3DEXT/ParticleRenderSystem3DEXT.hpp"

#include "CNA/Extended/ECS/World.hpp"
#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/World3DEXT/BillboardRenderSystemEXT.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ParticleEffect3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ParticleEffectComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/ParticleEmitter3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ParticleUpdateSystem3DEXT.hpp"
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
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::RasterizerState;
    using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using ECS::World;
    using ECS::WorldBuilder;
    using System::TimeSpan;

    namespace
    {
        class ParticleRenderSystem3DEXTTest : public ::testing::Test
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

        bool AnyPixelMatches(const std::vector<Color>& pixels, const Color& color)
        {
            return std::any_of(pixels.begin(), pixels.end(), [&](const Color& p) { return p == color; });
        }
    }

    TEST_F(ParticleRenderSystem3DEXTTest, ActiveParticles_AreDrawnAsBillboards)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);

        ParticleEffect3DEXT effect;
        ParticleEmitter3DEXT& emitter = effect.AddEmitterEXT(std::make_unique<ParticleEmitter3DEXT>());
        emitter.MinScaleEXT = 6.0f;
        emitter.MaxScaleEXT = 6.0f;
        emitter.MinLifetimeEXT = 100.0f;
        emitter.MaxLifetimeEXT = 100.0f;
        emitter.MinSpeedEXT = 0.0f;
        emitter.MaxSpeedEXT = 0.0f;
        emitter.StartColorEXT = Color(255, 0, 255, 255);
        emitter.EndColorEXT = Color(255, 0, 255, 255);
        emitter.StartOpacityEXT = 1.0f;
        emitter.EndOpacityEXT = 1.0f;
        emitter.EmitEXT(1, Vector3::Zero);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<ParticleUpdateSystem3DEXT>());
        auto billboardSystemOwner = std::make_unique<BillboardRenderSystemEXT>(graphicsDevice, camera);
        BillboardRenderSystemEXT& billboardSystem = *billboardSystemOwner;
        builder.AddSystem(std::move(billboardSystemOwner));
        builder.AddSystem(std::make_unique<ParticleRenderSystem3DEXT>(graphicsDevice, billboardSystem));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        ParticleEffectComponentEXT particleComponent;
        particleComponent.EffectEXT = &effect;
        particleComponent.TextureEXT = &whiteTex;
        entity.Attach(&particleComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::Zero);
        world->Update(gameTime);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        (void)width;

        EXPECT_TRUE(AnyPixelMatches(pixels, Color(255, 0, 255, 255)));
    }

    TEST_F(ParticleRenderSystem3DEXTTest, NoActiveParticles_DrawsNothing)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);

        ParticleEffect3DEXT effect;
        effect.AddEmitterEXT(std::make_unique<ParticleEmitter3DEXT>());

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<ParticleUpdateSystem3DEXT>());
        auto billboardSystemOwner = std::make_unique<BillboardRenderSystemEXT>(graphicsDevice, camera);
        BillboardRenderSystemEXT& billboardSystem = *billboardSystemOwner;
        builder.AddSystem(std::move(billboardSystemOwner));
        builder.AddSystem(std::make_unique<ParticleRenderSystem3DEXT>(graphicsDevice, billboardSystem));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        ParticleEffectComponentEXT particleComponent;
        particleComponent.EffectEXT = &effect;
        particleComponent.TextureEXT = &whiteTex;
        entity.Attach(&particleComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::Zero);
        world->Update(gameTime);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        (void)width;

        EXPECT_TRUE(std::none_of(pixels.begin(), pixels.end(),
                                  [](const Color& p) { return p.getRProperty() > 0 || p.getGProperty() > 0 || p.getBProperty() > 0; }));
    }
}
