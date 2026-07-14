// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/CubeMeshRenderSystemEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/CubeMeshComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionTexture.hpp"

#include <array>
#include <cstdint>
#include <typeindex>

namespace CNA::Extended::World3DEXT
{
    using ECS::AspectBuilder;
    using ECS::ComponentManager;
    using ECS::Entity;
    using Microsoft::Xna::Framework::BoundingFrustum;
    using Microsoft::Xna::Framework::BoundingSphere;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;
    using Microsoft::Xna::Framework::Graphics::VertexPositionTexture;

    namespace
    {
        // A unit cube (-0.5..0.5 on every axis), 24 vertices (4 per face, so each face gets
        // its own full [0,1] UV square rather than sharing corner vertices between faces),
        // wound counter-clockwise as seen from outside each face -- matches cna-scene::
        // CubeMesh's own documented winding convention (see CubeMeshComponentEXT.hpp).
        void BuildUnitCubeMeshEXT(std::vector<VertexPositionTexture>& vertices, std::vector<std::uint16_t>& indices)
        {
            struct Face
            {
                std::array<Vector3, 4> Corners;
            };

            const std::array<Face, 6> faces = {{
                // +Z (front)
                {{Vector3(-0.5f, -0.5f, 0.5f), Vector3(0.5f, -0.5f, 0.5f), Vector3(0.5f, 0.5f, 0.5f), Vector3(-0.5f, 0.5f, 0.5f)}},
                // -Z (back)
                {{Vector3(0.5f, -0.5f, -0.5f), Vector3(-0.5f, -0.5f, -0.5f), Vector3(-0.5f, 0.5f, -0.5f), Vector3(0.5f, 0.5f, -0.5f)}},
                // +X (right)
                {{Vector3(0.5f, -0.5f, 0.5f), Vector3(0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f)}},
                // -X (left)
                {{Vector3(-0.5f, -0.5f, -0.5f), Vector3(-0.5f, -0.5f, 0.5f), Vector3(-0.5f, 0.5f, 0.5f), Vector3(-0.5f, 0.5f, -0.5f)}},
                // +Y (top)
                {{Vector3(-0.5f, 0.5f, 0.5f), Vector3(0.5f, 0.5f, 0.5f), Vector3(0.5f, 0.5f, -0.5f), Vector3(-0.5f, 0.5f, -0.5f)}},
                // -Y (bottom)
                {{Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, -0.5f, -0.5f), Vector3(0.5f, -0.5f, 0.5f), Vector3(-0.5f, -0.5f, 0.5f)}},
            }};

            const std::array<Vector2, 4> uvs = {Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(1.0f, 1.0f), Vector2(0.0f, 1.0f)};

            for (const Face& face : faces)
            {
                const auto base = static_cast<std::uint16_t>(vertices.size());
                for (int i = 0; i < 4; ++i)
                {
                    vertices.emplace_back(face.Corners[static_cast<std::size_t>(i)], uvs[static_cast<std::size_t>(i)]);
                }
                indices.push_back(base + 0);
                indices.push_back(base + 1);
                indices.push_back(base + 2);
                indices.push_back(base + 0);
                indices.push_back(base + 2);
                indices.push_back(base + 3);
            }
        }
    }

    CubeMeshRenderSystemEXT::CubeMeshRenderSystemEXT(GraphicsDevice& graphicsDevice, Camera3DEXT& camera)
        : EntityDrawSystem(AspectBuilder().All({std::type_index(typeid(CubeMeshComponentEXT))})),
          graphicsDevice_(&graphicsDevice), camera_(&camera), cubeVertexBuffer_(graphicsDevice, 24),
          cubeIndexBuffer_(graphicsDevice, 36), effect_(graphicsDevice)
    {
        std::vector<VertexPositionTexture> vertices;
        std::vector<std::uint16_t> indices;
        BuildUnitCubeMeshEXT(vertices, indices);

        cubeVertexBuffer_.SetData(vertices.data(), static_cast<int>(vertices.size()));
        cubeIndexBuffer_.SetData(indices.data(), static_cast<int>(indices.size()));

        effect_.setTextureEnabledProperty(true);
    }

    void CubeMeshRenderSystemEXT::Initialize(ComponentManager& componentManager)
    {
        (void)componentManager; // no cached ComponentMapper needed: Entity::Get<T>() below is cheap
    }

    void CubeMeshRenderSystemEXT::Draw(const GameTime& gameTime)
    {
        (void)gameTime;

        const BoundingFrustum frustum = camera_->GetBoundingFrustumEXT();

        for (const int entityId : getActiveEntitiesProperty())
        {
            Entity* entity = GetEntity(entityId);
            if (entity == nullptr)
            {
                continue;
            }

            CubeMeshComponentEXT* cubeComponent = entity->Get<CubeMeshComponentEXT>();
            if (cubeComponent == nullptr)
            {
                continue;
            }

            Matrix worldPosition = Matrix::getIdentityProperty();
            if (Transform3ComponentEXT* transformComponent = entity->Get<Transform3ComponentEXT>())
            {
                worldPosition = transformComponent->TransformEXT.getWorldMatrixProperty();
            }

            const Matrix world = Matrix::CreateScale(cubeComponent->SizeEXT) * worldPosition;

            const float boundingRadius = cubeComponent->SizeEXT.Length() * 0.5f;
            const BoundingSphere worldBounds = BoundingSphere(Vector3::Zero, boundingRadius).Transform(worldPosition);
            if (!frustum.Intersects(worldBounds))
            {
                continue;
            }

            DrawCubeEXT(cubeComponent->TextureEXT, world, cubeComponent->TintEXT);
        }
    }

    void CubeMeshRenderSystemEXT::DrawCubeEXT(Microsoft::Xna::Framework::Graphics::Texture2D* texture, const Matrix& world,
                                               const Microsoft::Xna::Framework::Color& tint)
    {
        effect_.View = camera_->GetViewMatrixEXT();
        effect_.Projection = camera_->GetProjectionMatrixEXT();
        effect_.World = world;
        effect_.setTextureProperty(texture);
        effect_.setDiffuseColorProperty(tint.ToVector3());
        effect_.setAlphaProperty(static_cast<float>(tint.getAProperty()) / 255.0f);
        effect_.Apply();

        graphicsDevice_->SetVertexBuffer(&cubeVertexBuffer_);
        graphicsDevice_->SetIndexBuffer(&cubeIndexBuffer_);
        graphicsDevice_->DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, cubeVertexBuffer_.getVertexCountProperty(), 0, 12);
    }
}
