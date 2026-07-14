// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for RenderSystem3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
//
// Real headless render tests, following the exact idiom already established in
// TilemapIntegrationTests.cpp (see that file's own header comment for the full rationale):
// GraphicsDevice::SetRenderTarget(&rt) -> draw -> GraphicsDevice::GetBackBufferData (while
// the render target is STILL bound) -> SetRenderTarget(nullptr).
//
// The test model is a single hand-built, unlit, vertex-colored triangle (a plain Model with
// one ModelBone/ModelMesh/ModelMeshPart, a real VertexBuffer/IndexBuffer, and a BasicEffect
// with VertexColorEnabled=true) -- there is no content pipeline in scope for this project
// (see the root CLAUDE.md's exclusion list), so tests build Model instances directly the
// same way cna's own SkinnedModelEXTTests.cpp does for GPU-backed ModelMeshPart coverage.
#include "CNA/Extended/World3DEXT/RenderSystem3DEXT.hpp"

#include "CNA/Extended/ECS/World.hpp"
#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ModelComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "System/TimeSpan.hpp"

#include <algorithm>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::BasicEffect;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::IndexBuffer;
    using Microsoft::Xna::Framework::Graphics::Model;
    using Microsoft::Xna::Framework::Graphics::ModelBone;
    using Microsoft::Xna::Framework::Graphics::ModelMesh;
    using Microsoft::Xna::Framework::Graphics::ModelMeshPart;
    using Microsoft::Xna::Framework::Graphics::RasterizerState;
    using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
    using Microsoft::Xna::Framework::Graphics::VertexBuffer;
    using Microsoft::Xna::Framework::Graphics::VertexPositionColor;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using ECS::World;
    using ECS::WorldBuilder;
    using System::TimeSpan;

    namespace
    {
        // A large, camera-facing triangle in the Z=0 plane, big enough to fill most of the
        // viewport when viewed by a camera looking down -Z from a few units away. Solid red,
        // via VertexColorEnabled (no texture needed for this test).
        struct TestTriangleModel
        {
            explicit TestTriangleModel(GraphicsDevice& graphicsDevice)
                : effect(graphicsDevice), vertexBuffer(graphicsDevice, 3), indexBuffer(graphicsDevice, 3),
                  bone(0, "root")
            {
                effect.VertexColorEnabled = true;

                const VertexPositionColor vertices[3] = {
                    VertexPositionColor(Vector3(0.0f, 3.0f, 0.0f), Color(255, 0, 0, 255)),
                    VertexPositionColor(Vector3(3.0f, -3.0f, 0.0f), Color(255, 0, 0, 255)),
                    VertexPositionColor(Vector3(-3.0f, -3.0f, 0.0f), Color(255, 0, 0, 255)),
                };
                vertexBuffer.SetData(vertices, 3);

                const std::uint16_t indices[3] = {0, 1, 2};
                indexBuffer.SetData(indices, 3);

                part = ModelMeshPart(&vertexBuffer, &indexBuffer, 3, 1, 0, 0);
                part.setEffectProperty(&effect);

                mesh = std::make_unique<ModelMesh>(&graphicsDevice, std::vector<ModelMeshPart*>{&part});
                mesh->getEffectsPropertyMutable().Add(&effect);

                model = std::make_unique<Model>(&graphicsDevice, std::vector<ModelBone*>{&bone},
                                                 std::vector<ModelMesh*>{mesh.get()});
            }

            BasicEffect effect;
            VertexBuffer vertexBuffer;
            IndexBuffer indexBuffer;
            ModelBone bone;
            ModelMeshPart part;
            std::unique_ptr<ModelMesh> mesh;
            std::unique_ptr<Model> model;
        };

        class RenderSystem3DEXTTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                // 800x480 matches the GraphicsDevice's own default window/backbuffer size --
                // see TilemapIntegrationTests.cpp's identical choice and its header comment:
                // GetBackBufferData's rect==nullptr path reads the *window's* logical size
                // (EasyGLGraphicsBackend::GetViewportSize), not the currently-bound render
                // target's size, so the two must match here too.
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

    TEST_F(RenderSystem3DEXTTest, VisibleEntity_IsDrawn)
    {
        TestTriangleModel triangle(graphicsDevice);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<RenderSystem3DEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        ModelComponentEXT modelComponent;
        modelComponent.ModelEXT = triangle.model.get();
        modelComponent.BoundsEXT = Microsoft::Xna::Framework::BoundingSphere(Vector3::Zero, 5.0f);
        entity.Attach(&modelComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        const int height = graphicsDevice.getViewportProperty().getHeightProperty();

        EXPECT_TRUE(AnyPixelNotBlack(pixels));
        const Color center = GetPixel(pixels, width, width / 2, height / 2);
        EXPECT_GT(center.getRProperty(), 0);
        EXPECT_EQ(center.getGProperty(), 0);
        EXPECT_EQ(center.getBProperty(), 0);
    }

    TEST_F(RenderSystem3DEXTTest, EntityOutsideFrustum_IsCulledAndNotDrawn)
    {
        TestTriangleModel triangle(graphicsDevice);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<RenderSystem3DEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        Transform3ComponentEXT transformComponent;
        // Far outside the camera's far plane (100 units) and view cone.
        transformComponent.TransformEXT.setPositionProperty(Vector3(0.0f, 0.0f, -10000.0f));
        entity.Attach(&transformComponent);

        ModelComponentEXT modelComponent;
        modelComponent.ModelEXT = triangle.model.get();
        modelComponent.BoundsEXT = Microsoft::Xna::Framework::BoundingSphere(Vector3::Zero, 5.0f);
        entity.Attach(&modelComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        (void)width;

        EXPECT_FALSE(AnyPixelNotBlack(pixels));
    }
}
