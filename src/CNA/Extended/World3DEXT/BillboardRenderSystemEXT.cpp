// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/BillboardRenderSystemEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/BillboardComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"

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

        const Matrix view = camera_->GetViewMatrixEXT();
        const Matrix projection = camera_->GetProjectionMatrixEXT();
        const BoundingFrustum frustum = camera_->GetBoundingFrustumEXT();
        const Vector3 cameraPosition = camera_->getPositionProperty();
        const Vector3 cameraUp = camera_->getUpProperty();

        effect_.View = view;
        effect_.Projection = projection;

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

            const Matrix world = Matrix::CreateScale(billboardComponent->SizeEXT.X, billboardComponent->SizeEXT.Y, 1.0f)
                * Matrix::CreateBillboard(objectPosition, cameraPosition, cameraUp, std::nullopt);

            effect_.World = world;
            effect_.setTextureProperty(billboardComponent->TextureEXT);
            effect_.setDiffuseColorProperty(billboardComponent->TintEXT.ToVector3());
            effect_.setAlphaProperty(static_cast<float>(billboardComponent->TintEXT.getAProperty()) / 255.0f);
            effect_.Apply();

            graphicsDevice_->SetVertexBuffer(billboardComponent->VertexBufferEXT);
            graphicsDevice_->SetIndexBuffer(&quadIndexBuffer_);
            graphicsDevice_->DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, billboardComponent->VertexBufferEXT->getVertexCountProperty(), 0, 2);
        }
    }
}
