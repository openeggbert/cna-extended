// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/RenderSystem3DEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ModelComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"

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

    RenderSystem3DEXT::RenderSystem3DEXT(Camera3DEXT& camera)
        : EntityDrawSystem(AspectBuilder().All({std::type_index(typeid(ModelComponentEXT))})), camera_(&camera)
    {
    }

    void RenderSystem3DEXT::Initialize(ComponentManager& componentManager)
    {
        (void)componentManager; // no cached ComponentMapper needed: Entity::Get<T>() below is cheap
    }

    void RenderSystem3DEXT::Draw(const GameTime& gameTime)
    {
        (void)gameTime;

        const Matrix view = camera_->GetViewMatrixEXT();
        const Matrix projection = camera_->GetProjectionMatrixEXT();
        const BoundingFrustum frustum = camera_->GetBoundingFrustumEXT();

        for (const int entityId : getActiveEntitiesProperty())
        {
            Entity* entity = GetEntity(entityId);
            if (entity == nullptr)
            {
                continue;
            }

            ModelComponentEXT* modelComponent = entity->Get<ModelComponentEXT>();
            if (modelComponent == nullptr || modelComponent->ModelEXT == nullptr)
            {
                continue;
            }

            Matrix world = Matrix::getIdentityProperty();
            if (Transform3ComponentEXT* transformComponent = entity->Get<Transform3ComponentEXT>())
            {
                world = transformComponent->TransformEXT.getWorldMatrixProperty();
            }

            const BoundingSphere worldBounds = modelComponent->BoundsEXT.Transform(world);
            if (!frustum.Intersects(worldBounds))
            {
                continue;
            }

            modelComponent->ModelEXT->Draw(world, view, projection);
        }
    }
}
