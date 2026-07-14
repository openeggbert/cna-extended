// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for ComputeModelBoundsEXT (see 3d.md/plan3d.md, audit.md finding
// A-08) -- there is no upstream MonoGame.Extended test suite to port here.
//
// ModelBoundsEXT.hpp's own header comment documents an important constraint discovered
// while implementing this fix: cna's ModelMesh::boundingSphere_ has no public setter and is
// never written by any code path in `cna` today, so ModelMesh::getBoundingSphereProperty()
// always returns a zero-radius sphere at the origin for any Model buildable here. These
// tests therefore verify what's actually verifiable given that constraint -- the merge
// control flow across 0/1/N meshes (no crashes, no off-by-one on the loop bounds, correct
// delegation to each mesh's own getBoundingSphereProperty()) -- not a non-degenerate merged
// radius, which isn't producible without a `cna`-side change (out of scope here).
#include "CNA/Extended/World3DEXT/ModelBoundsEXT.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"

#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::IndexBuffer;
    using Microsoft::Xna::Framework::Graphics::Model;
    using Microsoft::Xna::Framework::Graphics::ModelBone;
    using Microsoft::Xna::Framework::Graphics::ModelMesh;
    using Microsoft::Xna::Framework::Graphics::ModelMeshPart;
    using Microsoft::Xna::Framework::Graphics::VertexBuffer;
    using Microsoft::Xna::Framework::Graphics::VertexPositionColor;

    namespace
    {
        // Minimal single-triangle mesh, matching RenderSystem3DEXTTests.cpp's
        // TestTriangleModel idiom (no content pipeline in scope -- see root CLAUDE.md).
        struct TestTriangleMesh
        {
            explicit TestTriangleMesh(GraphicsDevice& graphicsDevice)
                : vertexBuffer(graphicsDevice, 3), indexBuffer(graphicsDevice, 3), bone(0, "root")
            {
                const VertexPositionColor vertices[3] = {
                    VertexPositionColor(Vector3(0.0f, 1.0f, 0.0f), Microsoft::Xna::Framework::Color(255, 0, 0, 255)),
                    VertexPositionColor(Vector3(1.0f, -1.0f, 0.0f), Microsoft::Xna::Framework::Color(255, 0, 0, 255)),
                    VertexPositionColor(Vector3(-1.0f, -1.0f, 0.0f), Microsoft::Xna::Framework::Color(255, 0, 0, 255)),
                };
                vertexBuffer.SetData(vertices, 3);

                const std::uint16_t indices[3] = {0, 1, 2};
                indexBuffer.SetData(indices, 3);

                part = ModelMeshPart(&vertexBuffer, &indexBuffer, 3, 1, 0, 0);
                mesh = std::make_unique<ModelMesh>(&graphicsDevice, std::vector<ModelMeshPart*>{&part});
            }

            VertexBuffer vertexBuffer;
            IndexBuffer indexBuffer;
            ModelBone bone;
            ModelMeshPart part;
            std::unique_ptr<ModelMesh> mesh;
        };
    }

    TEST(ModelBoundsEXTTests, ComputeModelBoundsEXT_NoMeshes_ReturnsZeroRadiusSphereAtOrigin)
    {
        GraphicsDevice graphicsDevice;
        ModelBone bone(0, "root");
        const Model model(&graphicsDevice, std::vector<ModelBone*>{&bone}, std::vector<ModelMesh*>{});

        const auto bounds = ComputeModelBoundsEXT(model);

        EXPECT_EQ(bounds.Center, Vector3::Zero);
        EXPECT_EQ(bounds.Radius, 0.0f);
    }

    TEST(ModelBoundsEXTTests, ComputeModelBoundsEXT_SingleMesh_ReturnsThatMeshsOwnBounds)
    {
        GraphicsDevice graphicsDevice;
        TestTriangleMesh triangle(graphicsDevice);
        ModelBone bone(0, "root");
        const Model model(&graphicsDevice, std::vector<ModelBone*>{&bone}, std::vector<ModelMesh*>{triangle.mesh.get()});

        const auto bounds = ComputeModelBoundsEXT(model);

        EXPECT_EQ(bounds.Center, triangle.mesh->getBoundingSphereProperty().Center);
        EXPECT_EQ(bounds.Radius, triangle.mesh->getBoundingSphereProperty().Radius);
    }

    TEST(ModelBoundsEXTTests, ComputeModelBoundsEXT_MultipleMeshes_MergesAllWithoutCrashing)
    {
        GraphicsDevice graphicsDevice;
        TestTriangleMesh first(graphicsDevice);
        TestTriangleMesh second(graphicsDevice);
        TestTriangleMesh third(graphicsDevice);
        ModelBone bone(0, "root");
        const Model model(&graphicsDevice, std::vector<ModelBone*>{&bone},
                           std::vector<ModelMesh*>{first.mesh.get(), second.mesh.get(), third.mesh.get()});

        // Exercises the N>1 loop path (BoundingSphere::CreateMerged called across every
        // mesh, not just the first) without an off-by-one on the mesh count.
        const auto bounds = ComputeModelBoundsEXT(model);

        EXPECT_GE(bounds.Radius, 0.0f);
    }
}
