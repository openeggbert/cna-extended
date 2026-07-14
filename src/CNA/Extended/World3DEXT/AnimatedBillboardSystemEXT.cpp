// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/AnimatedBillboardSystemEXT.hpp"

#include "CNA/Extended/Animations/AnimationController.hpp"
#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/Graphics/SpriteSheet.hpp"
#include "CNA/Extended/Graphics/Texture2DAtlas.hpp"
#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "CNA/Extended/World3DEXT/AnimatedBillboardComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/BillboardComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/BillboardMeshEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"

#include <typeindex>

namespace CNA::Extended::World3DEXT
{
    using ECS::AspectBuilder;
    using ECS::ComponentManager;
    using ECS::Entity;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using CNA::Extended::Graphics::Texture2DRegion;

    AnimatedBillboardSystemEXT::AnimatedBillboardSystemEXT()
        : EntityUpdateSystem(AspectBuilder().All(
              {std::type_index(typeid(BillboardComponentEXT)), std::type_index(typeid(AnimatedBillboardComponentEXT))}))
    {
    }

    void AnimatedBillboardSystemEXT::Initialize(ComponentManager& componentManager)
    {
        (void)componentManager; // no cached ComponentMapper needed: Entity::Get<T>() below is cheap
    }

    void AnimatedBillboardSystemEXT::Update(const GameTime& gameTime)
    {
        for (const int entityId : getActiveEntitiesProperty())
        {
            Entity* entity = GetEntity(entityId);
            if (entity == nullptr)
            {
                continue;
            }

            AnimatedBillboardComponentEXT* animatedComponent = entity->Get<AnimatedBillboardComponentEXT>();
            if (animatedComponent == nullptr || animatedComponent->ControllerEXT == nullptr || animatedComponent->SpriteSheetEXT == nullptr)
            {
                continue;
            }

            const int previousFrame = animatedComponent->ControllerEXT->getCurrentFrameProperty();
            animatedComponent->ControllerEXT->Update(gameTime);

            if (animatedComponent->ControllerEXT->getCurrentFrameProperty() != previousFrame)
            {
                SyncCurrentFrameEXT(entityId);
            }
        }
    }

    void AnimatedBillboardSystemEXT::OnEntityAdded(int entityId)
    {
        EntityUpdateSystem::OnEntityAdded(entityId);
        SyncCurrentFrameEXT(entityId);
    }

    void AnimatedBillboardSystemEXT::SyncCurrentFrameEXT(int entityId)
    {
        Entity* entity = GetEntity(entityId);
        if (entity == nullptr)
        {
            return;
        }

        BillboardComponentEXT* billboardComponent = entity->Get<BillboardComponentEXT>();
        AnimatedBillboardComponentEXT* animatedComponent = entity->Get<AnimatedBillboardComponentEXT>();
        if (billboardComponent == nullptr || animatedComponent == nullptr || billboardComponent->VertexBufferEXT == nullptr
            || animatedComponent->ControllerEXT == nullptr || animatedComponent->SpriteSheetEXT == nullptr)
        {
            return;
        }

        const std::shared_ptr<Texture2DRegion> region =
            animatedComponent->SpriteSheetEXT->getTextureAtlasProperty()[animatedComponent->ControllerEXT->getCurrentFrameProperty()];
        Texture2D* texture = region->getTextureProperty();

        const RectangleF uvRect = ConvertPixelRectToUvRectEXT(region->getBoundsProperty(), texture->getWidthProperty(), texture->getHeightProperty());
        const auto vertices = BuildBillboardQuadVerticesEXT(uvRect);
        billboardComponent->VertexBufferEXT->SetData(vertices.data(), 4);
        billboardComponent->TextureEXT = texture;
    }
}
