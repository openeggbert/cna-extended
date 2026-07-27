// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for ModelAnimationComponentEXT/ModelAnimationSystem3DEXT (see
// 3d.md/plan3d.md) -- there is no upstream MonoGame.Extended equivalent to port here.
//
// Uses cna's own real glTF-runtime-loading path (Microsoft::Xna::Framework::Content::
// ContentManager::Load<Model>) to build a genuine skinned Model + SkinningData, rather than
// hand-constructing one field-by-field: `kSkinnedAnimatedGltf` below is copied verbatim from
// cna's own proven-good RuntimeGltfModelTests.cpp fixture (a 2-bone skinned/textured triangle,
// skin.joints deliberately reversed to prove topological reordering, one "Wave" clip translating
// the child bone from X=0 to X=2 over 1 second) -- reusing an already-tested-elsewhere fixture
// instead of authoring a new one from scratch avoids introducing a second, unverified glTF
// encoding of the same concept.
#include "CNA/Extended/World3DEXT/ModelAnimationSystem3DEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/ECS/World.hpp"
#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/World3DEXT/ModelAnimationComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "System/TimeSpan.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

namespace CNA::Extended::World3DEXT
{
    using ECS::Entity;
    using ECS::World;
    using ECS::WorldBuilder;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Content::ContentManager;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::Model;
    using Microsoft::Xna::Framework::Graphics::SkinningData;
    using System::TimeSpan;

    namespace
    {
        class ScratchDir
        {
        public:
            ScratchDir()
                : dir_(std::filesystem::temp_directory_path()
                       / ("cna_extended_model_anim_test_" + std::to_string(reinterpret_cast<std::uintptr_t>(this))))
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

        // Verbatim copy of RuntimeGltfModelTests.cpp's kSkinnedAnimatedGltf (cna repo) -- see
        // this file's own header comment for why it is reused rather than re-authored.
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

        Model LoadSkinnedTestModel(ScratchDir& contentRoot, GraphicsDevice& graphicsDevice)
        {
            WriteFile(contentRoot.path() / "skinned.gltf", kSkinnedAnimatedGltf);
            ContentManager cm(nullptr, contentRoot.path().string());
            cm.setGraphicsDevice(graphicsDevice);
            return cm.Load<Model>("skinned");
        }
    }

    TEST(ModelAnimationSystem3DEXTTest, SettingClipNameStartsItAndAdvancesBoneTransforms)
    {
        ScratchDir contentRoot;
        GraphicsDevice graphicsDevice;
        Model model = LoadSkinnedTestModel(contentRoot, graphicsDevice);
        auto* skinningData = static_cast<SkinningData*>(model.getTagProperty());
        ASSERT_NE(skinningData, nullptr);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<ModelAnimationSystem3DEXT>());
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        Entity& entity = world->CreateEntity();
        ModelAnimationComponentEXT component(*skinningData);
        component.ModelEXT = &model;
        component.ClipNameEXT = "Wave";
        entity.Attach(&component);

        // Bind pose (no clip started yet): child bone (skin index 1, see the reversed-joints
        // comment on kSkinnedAnimatedGltf) sits at its authored translation, X=0.
        EXPECT_NEAR(component.PlayerEXT.GetWorldTransforms()[1].getTranslationProperty().X, 0.0f, 1e-4f);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromSeconds(0.5));
        world->Update(gameTime); // ClipNameEXT set before this Update -> StartClip("Wave") + advance 0.5s

        ASSERT_NE(component.PlayerEXT.getCurrentClipProperty(), nullptr);
        // Halfway through the 1-second "Wave" clip (X: 0 -> 2, LINEAR): X should be ~1.
        EXPECT_NEAR(component.PlayerEXT.GetWorldTransforms()[1].getTranslationProperty().X, 1.0f, 1e-3f);

        world->Update(gameTime); // another 0.5s: clip finishes at X=2 (LoopEXT defaults true, wraps to 0 exactly at the boundary)
    }

    TEST(ModelAnimationSystem3DEXTTest, EmptyClipNameHoldsBindPoseWithoutStartingPlayback)
    {
        ScratchDir contentRoot;
        GraphicsDevice graphicsDevice;
        Model model = LoadSkinnedTestModel(contentRoot, graphicsDevice);
        auto* skinningData = static_cast<SkinningData*>(model.getTagProperty());
        ASSERT_NE(skinningData, nullptr);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<ModelAnimationSystem3DEXT>());
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        Entity& entity = world->CreateEntity();
        ModelAnimationComponentEXT component(*skinningData); // ClipNameEXT left empty
        component.ModelEXT = &model;
        entity.Attach(&component);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromSeconds(1.0));
        world->Update(gameTime);

        EXPECT_EQ(component.PlayerEXT.getCurrentClipProperty(), nullptr);
    }

    TEST(ModelAnimationSystem3DEXTTest, ChangingClipNameSwitchesClipsWithAHardCut)
    {
        ScratchDir contentRoot;
        GraphicsDevice graphicsDevice;
        Model model = LoadSkinnedTestModel(contentRoot, graphicsDevice);
        auto* skinningData = static_cast<SkinningData*>(model.getTagProperty());
        ASSERT_NE(skinningData, nullptr);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<ModelAnimationSystem3DEXT>());
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        Entity& entity = world->CreateEntity();
        ModelAnimationComponentEXT component(*skinningData);
        component.ModelEXT = &model;
        component.ClipNameEXT = "Wave";
        entity.Attach(&component);

        // Deliberately not 0.5+0.5: the 1-second "Wave" clip loops (LoopEXT defaults true), and
        // AnimationPlayer::Update's modulo-based wraparound maps position == Duration to exactly
        // 0.0 -- landing a second step precisely on the clip boundary would make "did it restart"
        // indistinguishable from "did it loop back to 0 exactly as expected".
        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromSeconds(0.4));
        world->Update(gameTime);
        EXPECT_NEAR(component.PlayerEXT.getCurrentPositionProperty().getTotalSecondsProperty(), 0.4, 1e-6);

        // Re-selecting the SAME clip name must NOT restart it (only a genuine change restarts).
        world->Update(gameTime);
        EXPECT_NEAR(component.PlayerEXT.getCurrentPositionProperty().getTotalSecondsProperty(), 0.8, 1e-6);

        // Switching to an unknown clip name holds the last computed pose rather than crashing.
        component.ClipNameEXT = "NoSuchClip";
        world->Update(gameTime);
        EXPECT_NEAR(component.PlayerEXT.getCurrentPositionProperty().getTotalSecondsProperty(), 0.8, 1e-6);
    }
}
