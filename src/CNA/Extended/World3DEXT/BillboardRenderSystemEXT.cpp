// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/BillboardRenderSystemEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/BillboardComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
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
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;

    BillboardRenderSystemEXT::BillboardRenderSystemEXT(GraphicsDevice& graphicsDevice, Camera3DEXT& camera)
        : EntityDrawSystem(AspectBuilder().All({std::type_index(typeid(BillboardComponentEXT))})), graphicsDevice_(&graphicsDevice),
          camera_(&camera), quadIndexBuffer_(graphicsDevice, 6), effect_(graphicsDevice)
    {
        const std::uint16_t indices[6] = {0, 1, 2, 0, 2, 3};
        quadIndexBuffer_.SetData(indices, 6);

        effect_.setTextureEnabledProperty(true);
    }

    void BillboardRenderSystemEXT::Initialize(ComponentManager& componentManager)
    {
        (void)componentManager; // no cached ComponentMapper needed: Entity::Get<T>() below is cheap
    }

    void BillboardRenderSystemEXT::Draw(const GameTime& gameTime)
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

            BillboardComponentEXT* billboardComponent = entity->Get<BillboardComponentEXT>();
            if (billboardComponent == nullptr || billboardComponent->VertexBufferEXT == nullptr)
            {
                continue;
            }

            Vector3 objectPosition = Vector3::Zero;
            if (Transform3ComponentEXT* transformComponent = entity->Get<Transform3ComponentEXT>())
            {
                objectPosition = transformComponent->TransformEXT.getWorldPositionProperty();
            }

            const float boundingRadius = std::max(billboardComponent->SizeEXT.X, billboardComponent->SizeEXT.Y) * 0.5f;
            const BoundingSphere worldBounds(objectPosition, boundingRadius);
            if (!frustum.Intersects(worldBounds))
            {
                continue;
            }

            DrawBillboardEXT(*billboardComponent->VertexBufferEXT, billboardComponent->TextureEXT, objectPosition,
                              billboardComponent->SizeEXT, billboardComponent->TintEXT);
        }
    }

    void BillboardRenderSystemEXT::DrawBillboardEXT(Microsoft::Xna::Framework::Graphics::VertexBuffer& vertexBuffer,
                                                      Microsoft::Xna::Framework::Graphics::Texture2D* texture, const Vector3& worldPosition,
                                                      const Microsoft::Xna::Framework::Vector2& size, const Microsoft::Xna::Framework::Color& tint)
    {
        const Matrix world = Matrix::CreateScale(size.X, size.Y, 1.0f)
            * Matrix::CreateBillboard(worldPosition, camera_->getPositionProperty(), camera_->getUpProperty(), std::nullopt);

        effect_.View = camera_->GetViewMatrixEXT();
        effect_.Projection = camera_->GetProjectionMatrixEXT();
        effect_.World = world;
        effect_.setTextureProperty(texture);
        effect_.setDiffuseColorProperty(tint.ToVector3());
        effect_.setAlphaProperty(static_cast<float>(tint.getAProperty()) / 255.0f);
        effect_.Apply();

        graphicsDevice_->SetVertexBuffer(&vertexBuffer);
        graphicsDevice_->SetIndexBuffer(&quadIndexBuffer_);
        graphicsDevice_->DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, vertexBuffer.getVertexCountProperty(), 0, 2);
    }
}
