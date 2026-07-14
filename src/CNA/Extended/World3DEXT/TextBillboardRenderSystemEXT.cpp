// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/TextBillboardRenderSystemEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Text3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"

#include <optional>
#include <typeindex>

namespace CNA::Extended::World3DEXT
{
    using ECS::AspectBuilder;
    using ECS::ComponentManager;
    using ECS::Entity;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;

    TextBillboardRenderSystemEXT::TextBillboardRenderSystemEXT(GraphicsDevice& graphicsDevice, Camera3DEXT& camera)
        : EntityDrawSystem(AspectBuilder().All({std::type_index(typeid(Text3DEXT))})), graphicsDevice_(&graphicsDevice),
          camera_(&camera), effect_(graphicsDevice)
    {
        effect_.setTextureEnabledProperty(true);
    }

    void TextBillboardRenderSystemEXT::Initialize(ComponentManager& componentManager)
    {
        (void)componentManager; // no cached ComponentMapper needed: Entity::Get<T>() below is cheap
    }

    void TextBillboardRenderSystemEXT::Draw(const GameTime& gameTime)
    {
        (void)gameTime;

        const Matrix view = camera_->GetViewMatrixEXT();
        const Matrix projection = camera_->GetProjectionMatrixEXT();
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

            Text3DEXT* textComponent = entity->Get<Text3DEXT>();
            if (textComponent == nullptr || textComponent->VertexBufferEXT == nullptr || textComponent->IndexBufferEXT == nullptr
                || textComponent->PrimitiveCountEXT <= 0)
            {
                continue;
            }

            Vector3 objectPosition = Vector3::Zero;
            if (Transform3ComponentEXT* transformComponent = entity->Get<Transform3ComponentEXT>())
            {
                objectPosition = transformComponent->TransformEXT.getWorldPositionProperty();
            }

            const Matrix world = Matrix::CreateScale(textComponent->ScaleEXT)
                * Matrix::CreateBillboard(objectPosition, cameraPosition, cameraUp, std::nullopt);

            effect_.World = world;
            effect_.setTextureProperty(textComponent->TextureEXT);
            effect_.setDiffuseColorProperty(textComponent->TintEXT.ToVector3());
            effect_.setAlphaProperty(static_cast<float>(textComponent->TintEXT.getAProperty()) / 255.0f);
            effect_.Apply();

            graphicsDevice_->SetVertexBuffer(textComponent->VertexBufferEXT);
            graphicsDevice_->SetIndexBuffer(textComponent->IndexBufferEXT);
            graphicsDevice_->DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, textComponent->VertexBufferEXT->getVertexCountProperty(), 0,
                                                    textComponent->PrimitiveCountEXT);
        }
    }
}
