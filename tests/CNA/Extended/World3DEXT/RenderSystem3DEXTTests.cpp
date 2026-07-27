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
#include "CNA/Extended/World3DEXT/ModelAnimationComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/ModelAnimationSystem3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ModelComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPartCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "System/TimeSpan.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <string>
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

        // Tests-only scratch content root; mirrors ModelAnimationSystem3DEXTTests.cpp's own copy.
        class ScratchDir
        {
        public:
            ScratchDir()
                : dir_(std::filesystem::temp_directory_path()
                       / ("cna_extended_render_anim_test_" + std::to_string(reinterpret_cast<std::uintptr_t>(this))))
            {
                std::filesystem::create_directories(dir_);
            }
            ~ScratchDir()
            {
                std::error_code ec;
                std::filesystem::remove_all(dir_, ec);
            }
            ScratchDir(const ScratchDir&) = delete;
            ScratchDir& operator=(const ScratchDir&) = delete;

            [[nodiscard]] const std::filesystem::path& path() const { return dir_; }

        private:
            std::filesystem::path dir_;
        };

        void WriteFile(const std::filesystem::path& path, const std::string& text)
        {
            std::ofstream f(path, std::ios::binary);
            f << text;
        }

        // A camera-facing, 2-unit-tall skinned+textured+animated triangle: verbatim copy of
        // ModelAnimationSystem3DEXTTests.cpp's own kSkinnedAnimatedGltf (see that file's header
        // comment for provenance) -- reused here rather than a third copy of the same concept.
        const char* kSkinnedAnimatedGltf = R"GLTF({
  "asset": { "version": "2.0" },
  "scene": 0,
  "scenes": [ { "nodes": [0, 2] } ],
  "nodes": [
    { "name": "ParentBone", "children": [1] },
    { "name": "ChildBone" },
    { "name": "MeshNode", "mesh": 0, "skin": 0 }
  ],
  "meshes": [ { "primitives": [ { "attributes": {
      "POSITION": 0, "NORMAL": 1, "TEXCOORD_0": 2, "JOINTS_0": 3, "WEIGHTS_0": 4
  }, "material": 0 } ] } ],
  "materials": [ { "pbrMetallicRoughness": { "baseColorTexture": { "index": 0 } } } ],
  "textures": [ { "source": 0 } ],
  "images": [ { "bufferView": 8, "mimeType": "image/png" } ],
  "skins": [ { "joints": [1, 0], "inverseBindMatrices": 5 } ],
  "animations": [ {
    "name": "Wave",
    "samplers": [ { "input": 6, "output": 7, "interpolation": "LINEAR" } ],
    "channels": [ { "sampler": 0, "target": { "node": 1, "path": "translation" } } ]
  } ],
  "buffers": [ {
    "byteLength": 397,
    "uri": "data:application/octet-stream;base64,AAAAAAAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAAAAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AQAAAAAAAAABAAAAAAAAAAEAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAAAAAAAAAAAAAAAIA/AAAAAAAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAAAAAAIA/AAAAAAAAAAAAAAAAAAAAAAAAgD8AAAAAAAAAAAAAAAAAAAAAAACAPwAAgD8AAAAAAAAAAAAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAAAAAAIA/AAAAAAAAAAAAAAAAAAAAAAAAgD8AAAAAAACAPwAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAIlQTkcNChoKAAAADUlIRFIAAAABAAAAAQgCAAAAkHdT3gAAAAxJREFUeJxj+M/AAAADAQEAyf6S7wAAAABJRU5ErkJggg=="
  } ],
  "bufferViews": [
    { "buffer": 0, "byteOffset": 0,   "byteLength": 36 },
    { "buffer": 0, "byteOffset": 36,  "byteLength": 36 },
    { "buffer": 0, "byteOffset": 72,  "byteLength": 24 },
    { "buffer": 0, "byteOffset": 96,  "byteLength": 24 },
    { "buffer": 0, "byteOffset": 120, "byteLength": 48 },
    { "buffer": 0, "byteOffset": 168, "byteLength": 128 },
    { "buffer": 0, "byteOffset": 296, "byteLength": 8 },
    { "buffer": 0, "byteOffset": 304, "byteLength": 24 },
    { "buffer": 0, "byteOffset": 328, "byteLength": 69 }
  ],
  "accessors": [
    { "bufferView": 0, "componentType": 5126, "count": 3, "type": "VEC3", "min": [0,0,0], "max": [1,1,0] },
    { "bufferView": 1, "componentType": 5126, "count": 3, "type": "VEC3" },
    { "bufferView": 2, "componentType": 5126, "count": 3, "type": "VEC2" },
    { "bufferView": 3, "componentType": 5123, "count": 3, "type": "VEC4" },
    { "bufferView": 4, "componentType": 5126, "count": 3, "type": "VEC4" },
    { "bufferView": 5, "componentType": 5126, "count": 2, "type": "MAT4" },
    { "bufferView": 6, "componentType": 5126, "count": 2, "type": "SCALAR", "min": [0.0], "max": [1.0] },
    { "bufferView": 7, "componentType": 5126, "count": 2, "type": "VEC3" }
  ]
})GLTF";
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

    TEST_F(RenderSystem3DEXTTest, ModelAnimationComponent_PosesAndDraws)
    {
        // The fixture's triangle sits within [0,1]x[0,1]x{0} -- move the camera in close and
        // center it on the triangle's own midpoint rather than the world origin (unlike
        // TestTriangleModel's 3-unit-wide triangle, which the default SetUp() camera already
        // frames well).
        camera.setPositionProperty(Vector3(0.5f, 0.5f, 5.0f));
        camera.setTargetProperty(Vector3(0.5f, 0.5f, 0.0f));

        ScratchDir contentRoot;
        WriteFile(contentRoot.path() / "skinned.gltf", kSkinnedAnimatedGltf);
        Microsoft::Xna::Framework::Content::ContentManager cm(nullptr, contentRoot.path().string());
        cm.setGraphicsDevice(graphicsDevice);
        Model model = cm.Load<Model>("skinned");
        auto* skinningData =
            static_cast<Microsoft::Xna::Framework::Graphics::SkinningData*>(model.getTagProperty());
        ASSERT_NE(skinningData, nullptr);

        // A fresh SkinnedEffect is unlit by default (real XNA behavior) -- without this, the
        // render below would stay black regardless of whether posing/drawing worked at all.
        // Matches AnimationSystem3DEXTTests.cpp's own identical real-render lighting recipe.
        auto* skinnedFx = dynamic_cast<Microsoft::Xna::Framework::Graphics::SkinnedEffect*>(
            model.getMeshesProperty()[0]->getMeshPartsProperty()[0]->getEffectProperty());
        ASSERT_NE(skinnedFx, nullptr);
        skinnedFx->setAmbientLightColorProperty(Vector3::One);
        skinnedFx->EnableDefaultLighting();
        skinnedFx->getDirectionalLight0Property().setEnabledProperty(true);
        skinnedFx->getDirectionalLight0Property().setDirectionProperty(Vector3(0.0f, 0.0f, -1.0f));
        skinnedFx->getDirectionalLight0Property().setDiffuseColorProperty(Vector3::One);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<ModelAnimationSystem3DEXT>());
        builder.AddSystem(std::make_unique<RenderSystem3DEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        ModelAnimationComponentEXT animComponent(*skinningData);
        animComponent.ModelEXT = &model;
        animComponent.ClipNameEXT = "Wave";
        animComponent.BoundsEXT = Microsoft::Xna::Framework::BoundingSphere(Vector3(0.5f, 0.5f, 0.0f), 2.0f);
        entity.Attach(&animComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromSeconds(0.5));
        world->Update(gameTime); // ModelAnimationSystem3DEXT starts "Wave" and advances it 0.5s

        // Halfway through the clip, the child bone (skin index 1) should have moved: proves
        // RenderSystem3DEXT's draw branch below is exercising a genuinely posed (not bind-pose)
        // skeleton, not just an unrelated static mesh.
        EXPECT_GT(animComponent.PlayerEXT.GetWorldTransforms()[1].getTranslationProperty().X, 0.5f);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        EXPECT_TRUE(AnyPixelNotBlack(pixels));
    }
}
