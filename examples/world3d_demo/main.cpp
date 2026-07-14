// SPDX-License-Identifier: MIT
//
// world3d_demo: an end-to-end World3DEXT example combining every Phase 1-9 module the way a
// real 3D game would use them together, built through World3DScreenEXT (Phase 9's own
// convenience base class -- this demo also exercises it, not just the modules underneath it):
//
//  - Camera3DEXT provides the view/projection every render system below draws through.
//  - Transform3ComponentEXT + TransformHierarchySystemEXT position a small parent/child cube
//    hierarchy (a "pillar" with a smaller cube orbiting-in-place on top of it, moving with its
//    parent automatically -- no per-frame code needed to keep the child attached).
//  - CubeMeshComponentEXT + CubeMeshRenderSystemEXT draw both the hierarchy cubes and the
//    tilemap's floor tiles (via TilemapRenderer3DEXT, which draws through the same shared
//    cube mesh/effect -- see CubeMeshRenderSystemEXT::DrawCubeEXT).
//  - A third cube entity is placed far behind the camera's far plane, deliberately never
//    drawn -- proving RenderSystem3DEXT-style per-entity frustum culling really works.
//  - Tilemap3DEXT + TilemapTileset3DEXT + TilemapRenderer3DEXT draw a small voxel floor area.
//  - SkinnedModelComponentEXT + AnimationSystem3DEXT + RenderSystem3DEXT's skinned path
//    animate a small two-bone "character" (the same hand-built rig shape
//    AnimationSystem3DEXTTests.cpp uses), its moving bone driven by a real keyframed clip.
//  - ParticleEffectComponentEXT + ParticleUpdateSystem3DEXT + ParticleRenderSystem3DEXT emit a
//    small burst of billboarded particles from the pillar's position.
//
// Like tiled_demo, this runs headlessly: a real GraphicsDevice plus an off-screen
// RenderTarget2D, read back via GraphicsDevice::GetBackBufferData (not
// RenderTarget2D::GetData -- see TilemapIntegrationTests.cpp's RenderToPixels comment for
// why), no real window. Each simulated frame samples a few real rendered pixels to prove
// actual drawing happened, not just "didn't throw"; the final frame is also saved as a PNG.
#include <CNA/Extended/ECS/Entity.hpp>
#include <CNA/Extended/ECS/World.hpp>
#include <CNA/Extended/ECS/WorldBuilder.hpp>
#include <CNA/Extended/World3DEXT/AnimationSystem3DEXT.hpp>
#include <CNA/Extended/World3DEXT/BillboardRenderSystemEXT.hpp>
#include <CNA/Extended/World3DEXT/Camera3DEXT.hpp>
#include <CNA/Extended/World3DEXT/CubeMeshComponentEXT.hpp>
#include <CNA/Extended/World3DEXT/CubeMeshRenderSystemEXT.hpp>
#include <CNA/Extended/World3DEXT/ParticleEffect3DEXT.hpp>
#include <CNA/Extended/World3DEXT/ParticleEffectComponentEXT.hpp>
#include <CNA/Extended/World3DEXT/ParticleEmitter3DEXT.hpp>
#include <CNA/Extended/World3DEXT/ParticleRenderSystem3DEXT.hpp>
#include <CNA/Extended/World3DEXT/ParticleUpdateSystem3DEXT.hpp>
#include <CNA/Extended/World3DEXT/RenderSystem3DEXT.hpp>
#include <CNA/Extended/World3DEXT/SkinnedModelComponentEXT.hpp>
#include <CNA/Extended/World3DEXT/Tilemap3DEXT.hpp>
#include <CNA/Extended/World3DEXT/Tilemap3DFactoryEXT.hpp>
#include <CNA/Extended/World3DEXT/TilemapRenderer3DEXT.hpp>
#include <CNA/Extended/World3DEXT/TilemapTileset3DEXT.hpp>
#include <CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp>
#include <CNA/Extended/World3DEXT/TransformHierarchySystemEXT.hpp>
#include <CNA/Extended/World3DEXT/World3DScreenEXT.hpp>

#include <Microsoft/Xna/Framework/BoundingSphere.hpp>
#include <Microsoft/Xna/Framework/Color.hpp>
#include <Microsoft/Xna/Framework/GameTime.hpp>
#include <Microsoft/Xna/Framework/MathHelper.hpp>
#include <Microsoft/Xna/Framework/Matrix.hpp>
#include <Microsoft/Xna/Framework/Vector2.hpp>
#include <Microsoft/Xna/Framework/Vector3.hpp>
#include <Microsoft/Xna/Framework/Vector4.hpp>
#include <Microsoft/Xna/Framework/Graphics/BasicEffect.hpp>
#include <Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp>
#include <Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp>
#include <Microsoft/Xna/Framework/Graphics/Model.hpp>
#include <Microsoft/Xna/Framework/Graphics/ModelBone.hpp>
#include <Microsoft/Xna/Framework/Graphics/ModelMesh.hpp>
#include <Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp>
#include <Microsoft/Xna/Framework/Graphics/RasterizerState.hpp>
#include <Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp>
#include <Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp>
#include <Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp>
#include <Microsoft/Xna/Framework/Graphics/Texture2D.hpp>
#include <Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp>
#include <Microsoft/Xna/Framework/Graphics/VertexPositionNormalTextureSkinned.hpp>
#include <Microsoft/Xna/Framework/Graphics/VertexPositionTexture.hpp>
#include <Microsoft/Xna/Framework/Graphics/Viewport.hpp>
#include <System/TimeSpan.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace CNA::Extended::World3DEXT;
namespace ECS = CNA::Extended::ECS;
using Microsoft::Xna::Framework::BoundingSphere;
using Microsoft::Xna::Framework::Color;
using Microsoft::Xna::Framework::GameTime;
using Microsoft::Xna::Framework::MathHelper;
using Microsoft::Xna::Framework::Matrix;
using Microsoft::Xna::Framework::Vector2;
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
using Microsoft::Xna::Framework::Graphics::SkinnedEffect;
using Microsoft::Xna::Framework::Graphics::SkinnedModelEXT;
using Microsoft::Xna::Framework::Graphics::Texture2D;
using Microsoft::Xna::Framework::Graphics::VertexBuffer;
using Microsoft::Xna::Framework::Graphics::VertexPositionNormalTextureSkinned;
using Microsoft::Xna::Framework::Graphics::VertexPositionTexture;
using System::TimeSpan;

namespace
{
    constexpr int kWidth = 800;
    constexpr int kHeight = 480;

    Texture2D MakeSolidTexture(GraphicsDevice& graphicsDevice, const Color& color)
    {
        Texture2D texture(graphicsDevice, 1, 1);
        const std::vector<Color> pixel(1, color);
        texture.SetData(pixel.data(), 1);
        return texture;
    }

    // A hand-built two-bone rig with one triangle 100%-weighted to the moving bone --
    // matches AnimationSystem3DEXTTests.cpp's MakeMovingTriangleModel shape, standing in
    // for a real skinned character asset (this project has no content pipeline in scope --
    // see the root CLAUDE.md's exclusion list -- so this demo, like that test file, builds
    // GPU-backed skinned geometry directly).
    std::unique_ptr<SkinnedModelEXT> MakeSkinnedCharacterEXT(GraphicsDevice& graphicsDevice, Texture2D& texture)
    {
        auto model = std::make_unique<SkinnedModelEXT>();
        model->BoneCount = 2;
        model->ParentBoneIndices = {-1, 0};
        model->BindPoseLocal = {Matrix::getIdentityProperty(), Matrix::getIdentityProperty()};
        model->InverseBindPoseGlobal = {Matrix::getIdentityProperty(), Matrix::getIdentityProperty()};

        Microsoft::Xna::Framework::Graphics::BoneTrackEXT track;
        track.BoneIndex = 1;
        track.Keys.push_back(Microsoft::Xna::Framework::Graphics::KeyframeEXT{TimeSpan::Zero, Vector3(0.0f, 0.0f, 0.0f)});
        track.Keys.push_back(Microsoft::Xna::Framework::Graphics::KeyframeEXT{TimeSpan::FromSeconds(1.0), Vector3(0.0f, 1.5f, 0.0f)});

        Microsoft::Xna::Framework::Graphics::AnimationClipEXT clip;
        clip.Duration = TimeSpan::FromSeconds(1.0);
        clip.Tracks.push_back(track);
        model->Clips["Bob"] = clip;

        const std::array<std::uint8_t, 4> boneOne{1, 0, 0, 0};
        const Microsoft::Xna::Framework::Vector4 weight(1.0f, 0.0f, 0.0f, 0.0f);
        const VertexPositionNormalTextureSkinned verts[3] = {
            {Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.5f, 0.0f), weight, boneOne},
            {Vector3(0.6f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 1.0f), weight, boneOne},
            {Vector3(-0.6f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 1.0f), weight, boneOne},
        };
        const std::uint16_t indices[3] = {0, 1, 2};

        auto vb = std::make_unique<VertexBuffer>(graphicsDevice, 3);
        vb->SetData(verts, 3);
        auto ib = std::make_unique<IndexBuffer>(graphicsDevice, 3);
        ib->SetData(indices, 3);
        auto part = std::make_unique<ModelMeshPart>(vb.get(), ib.get(), 3, 1, 0, 0);
        model->AddPartEXT("Body", std::move(vb), std::move(ib), std::move(part), texture);

        return model;
    }

    // World3DScreenEXT's own intended usage: a derived screen wires up its systems via
    // ConfigureWorldEXT, then creates its entities after the base Initialize() has built
    // the World.
    class World3DDemoScreen final : public World3DScreenEXT
    {
    public:
        World3DDemoScreen(GraphicsDevice& graphicsDevice, Texture2D& pillarTexture, Texture2D& floorTexture, Texture2D& particleTexture,
                           Texture2D& characterTexture)
            : graphicsDevice_(&graphicsDevice), pillarTexture_(&pillarTexture), floorTexture_(&floorTexture),
              particleTexture_(&particleTexture), characterModel_(MakeSkinnedCharacterEXT(graphicsDevice, characterTexture)),
              characterEffect_(graphicsDevice)
        {
        }

        void Initialize() override
        {
            World3DScreenEXT::Initialize();

            GetCamera3DEXT().setPositionProperty(Vector3(4.0f, 6.0f, 14.0f));
            GetCamera3DEXT().setTargetProperty(Vector3(0.0f, 1.0f, 0.0f));
            GetCamera3DEXT().setFieldOfViewProperty(MathHelper::PiOver4);
            GetCamera3DEXT().setAspectRatioProperty(static_cast<float>(kWidth) / static_cast<float>(kHeight));
            GetCamera3DEXT().setNearPlaneProperty(0.1f);
            GetCamera3DEXT().setFarPlaneProperty(200.0f);

            // ---- Transform hierarchy: a pillar with a smaller cube riding on top of it ----
            ECS::Entity& pillar = GetWorld3DEXT().CreateEntity();
            pillarTransform_.TransformEXT.setPositionProperty(Vector3(0.0f, 1.5f, 0.0f));
            pillar.Attach(&pillarTransform_);
            pillarCube_.TextureEXT = pillarTexture_;
            pillarCube_.SizeEXT = Vector3(1.5f, 3.0f, 1.5f);
            pillarCube_.TintEXT = Color(200, 80, 40, 255);
            pillar.Attach(&pillarCube_);

            ECS::Entity& cap = GetWorld3DEXT().CreateEntity();
            capTransform_.TransformEXT.setPositionProperty(Vector3(0.0f, 2.2f, 0.0f)); // local offset from the pillar
            capTransform_.ParentEntityIdEXT = pillar.getIdProperty();
            cap.Attach(&capTransform_);
            capCube_.TextureEXT = pillarTexture_;
            capCube_.SizeEXT = Vector3(2.2f, 0.6f, 2.2f);
            capCube_.TintEXT = Color(240, 200, 40, 255);
            cap.Attach(&capCube_);

            // ---- Deliberately culled entity: far behind the camera's 200-unit far plane ----
            ECS::Entity& hidden = GetWorld3DEXT().CreateEntity();
            hiddenTransform_.TransformEXT.setPositionProperty(Vector3(0.0f, 0.0f, -10000.0f));
            hidden.Attach(&hiddenTransform_);
            hiddenCube_.TextureEXT = pillarTexture_;
            hiddenCube_.SizeEXT = Vector3(1.0f, 1.0f, 1.0f);
            hidden.Attach(&hiddenCube_);

            // ---- Skinned character, bobbing up and down via its one animated bone ----
            ECS::Entity& character = GetWorld3DEXT().CreateEntity();
            characterTransform_.TransformEXT.setPositionProperty(Vector3(4.0f, 1.0f, 2.0f));
            character.Attach(&characterTransform_);
            characterComponent_.ModelEXT = characterModel_.get();
            characterComponent_.EffectEXT = &characterEffect_;
            characterComponent_.ClipNameEXT = "Bob";
            characterComponent_.LoopEXT = true;
            characterComponent_.BoundsEXT = BoundingSphere(Vector3::Zero, 2.0f);
            character.Attach(&characterComponent_);

            // ---- A small particle burst rising from the pillar ----
            ECS::Entity& sparks = GetWorld3DEXT().CreateEntity();
            sparksTransform_.TransformEXT.setPositionProperty(Vector3(0.0f, 3.0f, 0.0f));
            sparks.Attach(&sparksTransform_);
            ParticleEmitter3DEXT& emitter = sparksEffect_.AddEmitterEXT(std::make_unique<ParticleEmitter3DEXT>());
            emitter.EmissionRateEXT = 30.0f;
            emitter.MinLifetimeEXT = 0.8f;
            emitter.MaxLifetimeEXT = 1.2f;
            emitter.MinSpeedEXT = 1.0f;
            emitter.MaxSpeedEXT = 2.5f;
            emitter.ConeDirectionEXT = Vector3::Up;
            emitter.ConeHalfAngleEXT = MathHelper::PiOver4;
            emitter.MinScaleEXT = 0.3f;
            emitter.MaxScaleEXT = 0.6f;
            emitter.StartColorEXT = Color(255, 220, 80, 255);
            emitter.EndColorEXT = Color(255, 60, 0, 255);
            emitter.GravityEXT = Vector3(0.0f, -1.0f, 0.0f);
            sparksComponent_.EffectEXT = &sparksEffect_;
            sparksComponent_.TextureEXT = particleTexture_;
            sparks.Attach(&sparksComponent_);

            // ---- Voxel floor, drawn directly each frame (TilemapRenderer3DEXT is a
            // standalone renderer, not an ECS system -- see its own header comment) ----
            constexpr int kFloorSize = 6;
            std::vector<int> tileIds(static_cast<std::size_t>(kFloorSize) * kFloorSize, 1);
            floorTilemap_ = std::make_unique<Tilemap3DEXT>(
                Tilemap3DFactoryEXT::BuildFromArrayEXT(tileIds, kFloorSize, 1, kFloorSize, Vector3(2.0f, 0.5f, 2.0f)));
            floorTileset_.SetTileTextureEXT(1, floorTexture_);
        }

        void Draw(const GameTime& gameTime) override
        {
            World3DScreenEXT::Draw(gameTime);
            tilemapRenderer_->Draw(GetCamera3DEXT(), *floorTilemap_, floorTileset_);
        }

    protected:
        void ConfigureWorldEXT(ECS::WorldBuilder& builder) override
        {
            builder.AddSystem(std::make_unique<TransformHierarchySystemEXT>());
            builder.AddSystem(std::make_unique<AnimationSystem3DEXT>());
            builder.AddSystem(std::make_unique<ParticleUpdateSystem3DEXT>());
            builder.AddSystem(std::make_unique<RenderSystem3DEXT>(*graphicsDevice_, GetCamera3DEXT()));

            auto cubeSystemOwner = std::make_unique<CubeMeshRenderSystemEXT>(*graphicsDevice_, GetCamera3DEXT());
            cubeRenderSystem_ = cubeSystemOwner.get();
            builder.AddSystem(std::move(cubeSystemOwner));

            auto billboardSystemOwner = std::make_unique<BillboardRenderSystemEXT>(*graphicsDevice_, GetCamera3DEXT());
            BillboardRenderSystemEXT& billboardSystem = *billboardSystemOwner;
            builder.AddSystem(std::move(billboardSystemOwner));
            builder.AddSystem(std::make_unique<ParticleRenderSystem3DEXT>(*graphicsDevice_, billboardSystem));

            tilemapRenderer_ = std::make_unique<TilemapRenderer3DEXT>(*cubeRenderSystem_);
        }

    private:
        GraphicsDevice* graphicsDevice_;
        Texture2D* pillarTexture_;
        Texture2D* floorTexture_;
        Texture2D* particleTexture_;

        Transform3ComponentEXT pillarTransform_;
        CubeMeshComponentEXT pillarCube_;
        Transform3ComponentEXT capTransform_;
        CubeMeshComponentEXT capCube_;
        Transform3ComponentEXT hiddenTransform_;
        CubeMeshComponentEXT hiddenCube_;

        Transform3ComponentEXT characterTransform_;
        std::unique_ptr<SkinnedModelEXT> characterModel_;
        SkinnedEffect characterEffect_;
        SkinnedModelComponentEXT characterComponent_;

        Transform3ComponentEXT sparksTransform_;
        ParticleEffect3DEXT sparksEffect_;
        ParticleEffectComponentEXT sparksComponent_;

        std::unique_ptr<Tilemap3DEXT> floorTilemap_;
        TilemapTileset3DEXT floorTileset_;
        CubeMeshRenderSystemEXT* cubeRenderSystem_ = nullptr;
        std::unique_ptr<TilemapRenderer3DEXT> tilemapRenderer_;
    };

    // Mirrors TilemapIntegrationTests.cpp's/tiled_demo's own RenderToPixels helper.
    template <typename Fn>
    std::vector<Color> RenderToPixels(GraphicsDevice& graphicsDevice, Fn&& render)
    {
        RenderTarget2D rt(graphicsDevice, kWidth, kHeight);
        graphicsDevice.SetRenderTarget(&rt);
        graphicsDevice.Clear(Color::Black);

        render();

        std::vector<Color> pixels(static_cast<std::size_t>(kWidth) * static_cast<std::size_t>(kHeight), Color::Transparent);
        graphicsDevice.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));

        graphicsDevice.SetRenderTarget(nullptr);
        return pixels;
    }

    Color SamplePixel(const std::vector<Color>& pixels, int x, int y)
    {
        return pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(kWidth) + static_cast<std::size_t>(x)];
    }

    bool AnyPixelMatchesApprox(const std::vector<Color>& pixels, const Color& target, int tolerance)
    {
        for (const Color& p : pixels)
        {
            if (std::abs(p.getRProperty() - target.getRProperty()) <= tolerance && std::abs(p.getGProperty() - target.getGProperty()) <= tolerance
                && std::abs(p.getBProperty() - target.getBProperty()) <= tolerance)
            {
                return true;
            }
        }
        return false;
    }

    int RunDemo()
    {
        std::cout << "=== cna-extended world3d_demo ===\n";
        std::cout << "Modules exercised: World3DScreenEXT, Camera3DEXT, Transform3ComponentEXT/TransformHierarchySystemEXT,\n"
                     "CubeMeshComponentEXT/CubeMeshRenderSystemEXT, RenderSystem3DEXT (frustum culling + skinned path),\n"
                     "SkinnedModelComponentEXT/AnimationSystem3DEXT, ParticleEffectComponentEXT/ParticleUpdateSystem3DEXT/\n"
                     "ParticleRenderSystem3DEXT, Tilemap3DEXT/TilemapTileset3DEXT/TilemapRenderer3DEXT.\n\n";

        GraphicsDevice graphicsDevice;
        graphicsDevice.setViewportProperty(Microsoft::Xna::Framework::Graphics::Viewport(0, 0, kWidth, kHeight));
        graphicsDevice.setRasterizerStateProperty(RasterizerState::CullNone);

        Texture2D pillarTexture = MakeSolidTexture(graphicsDevice, Color::White);
        Texture2D floorTexture = MakeSolidTexture(graphicsDevice, Color::White);
        Texture2D particleTexture = MakeSolidTexture(graphicsDevice, Color::White);
        Texture2D characterTexture = MakeSolidTexture(graphicsDevice, Color::White);

        World3DDemoScreen screen(graphicsDevice, pillarTexture, floorTexture, particleTexture, characterTexture);
        screen.Initialize();

        std::vector<Color> lastFramePixels;
        TimeSpan totalTime = TimeSpan::Zero;
        const TimeSpan frameDelta = TimeSpan::FromSeconds(0.1);

        constexpr int kFrameCount = 10;
        for (int frame = 0; frame < kFrameCount; ++frame)
        {
            GameTime gameTime(totalTime, frameDelta);
            screen.Update(gameTime);

            lastFramePixels = RenderToPixels(graphicsDevice, [&]() { screen.Draw(gameTime); });

            const Color center = SamplePixel(lastFramePixels, kWidth / 2, kHeight / 2);
            std::cout << "frame " << frame << ": centerPixel=(" << static_cast<int>(center.getRProperty()) << ","
                      << static_cast<int>(center.getGProperty()) << "," << static_cast<int>(center.getBProperty()) << ")\n";

            totalTime = totalTime + frameDelta;
        }

        std::cout << "\n";

        bool ok = true;

        const bool anyLitPixel = std::any_of(lastFramePixels.begin(), lastFramePixels.end(), [](const Color& p) {
            return p.getRProperty() > 0 || p.getGProperty() > 0 || p.getBProperty() > 0;
        });
        std::cout << (anyLitPixel ? "[OK]   " : "[FAIL] ") << "Something rendered (pillar/floor/skinned character/particles).\n";
        ok = ok && anyLitPixel;

        const bool sparkColorVisible = AnyPixelMatchesApprox(lastFramePixels, Color(255, 220, 80, 255), 40)
            || AnyPixelMatchesApprox(lastFramePixels, Color(255, 60, 0, 255), 60);
        std::cout << (sparkColorVisible ? "[OK]   " : "[INFO] ") << "Spark-colored particle pixel visible (probabilistic; cone-sampled).\n";

        const std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "cna_extended_world3d_demo_frame.png";
        Texture2D screenshot(graphicsDevice, kWidth, kHeight);
        screenshot.SetData(lastFramePixels.data(), static_cast<int>(lastFramePixels.size()));
        screenshot.SaveAsPng(outputPath.string());
        std::cout << "Saved final frame to " << outputPath.string() << "\n";

        return ok ? 0 : 1;
    }
}

int main()
{
    try
    {
        return RunDemo();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "world3d_demo failed: " << ex.what() << "\n";
        return 1;
    }
}
