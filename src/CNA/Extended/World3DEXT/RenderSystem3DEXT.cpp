// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/RenderSystem3DEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ModelAnimationComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/ModelComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/SkinnedModelComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedPbrEffect.hpp"

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
    using Microsoft::Xna::Framework::Graphics::Effect;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::ModelMesh;
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;
    using Microsoft::Xna::Framework::Graphics::SkinnedEffect;
    using Microsoft::Xna::Framework::Graphics::SkinnedPbrEffect;

    RenderSystem3DEXT::RenderSystem3DEXT(GraphicsDevice& graphicsDevice, Camera3DEXT& camera)
        : EntityDrawSystem(AspectBuilder().One({std::type_index(typeid(ModelComponentEXT)),
                                                 std::type_index(typeid(SkinnedModelComponentEXT)),
                                                 std::type_index(typeid(ModelAnimationComponentEXT))})),
          graphicsDevice_(&graphicsDevice), camera_(&camera)
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

            Matrix world = Matrix::getIdentityProperty();
            if (Transform3ComponentEXT* transformComponent = entity->Get<Transform3ComponentEXT>())
            {
                world = transformComponent->TransformEXT.getWorldMatrixProperty();
            }

            if (ModelComponentEXT* modelComponent = entity->Get<ModelComponentEXT>())
            {
                if (modelComponent->ModelEXT != nullptr)
                {
                    const BoundingSphere worldBounds = modelComponent->BoundsEXT.Transform(world);
                    if (frustum.Intersects(worldBounds))
                    {
                        modelComponent->ModelEXT->Draw(world, view, projection);
                    }
                }
            }

            if (SkinnedModelComponentEXT* skinnedComponent = entity->Get<SkinnedModelComponentEXT>())
            {
                if (skinnedComponent->ModelEXT != nullptr && skinnedComponent->EffectEXT != nullptr)
                {
                    const BoundingSphere worldBounds = skinnedComponent->BoundsEXT.Transform(world);
                    if (frustum.Intersects(worldBounds))
                    {
                        // Matches AvatarRenderer::DrawRealEXT's real, already-working usage
                        // exactly: one shared SkinnedEffect set once, then Apply() + draw per
                        // part -- see SkinnedModelComponentEXT.hpp's header comment.
                        skinnedComponent->EffectEXT->setWorldProperty(world);
                        skinnedComponent->EffectEXT->setViewProperty(view);
                        skinnedComponent->EffectEXT->setProjectionProperty(projection);
                        skinnedComponent->EffectEXT->SetBoneTransforms(skinnedComponent->BoneTransformsEXT);

                        for (const auto& part : skinnedComponent->ModelEXT->Parts)
                        {
                            skinnedComponent->EffectEXT->setTextureProperty(part.Texture);
                            skinnedComponent->EffectEXT->Apply();

                            graphicsDevice_->SetVertexBuffer(part.Part->getVertexBufferProperty());
                            graphicsDevice_->SetIndexBuffer(part.Part->getIndexBufferProperty());
                            graphicsDevice_->DrawIndexedPrimitives(
                                PrimitiveType::TriangleList,
                                part.Part->getVertexOffsetProperty(),
                                0,
                                part.Part->getNumVerticesProperty(),
                                part.Part->getStartIndexProperty(),
                                part.Part->getPrimitiveCountProperty());
                        }
                    }
                }
            }

            if (ModelAnimationComponentEXT* animComponent = entity->Get<ModelAnimationComponentEXT>())
            {
                if (animComponent->ModelEXT != nullptr)
                {
                    const BoundingSphere worldBounds = animComponent->BoundsEXT.Transform(world);
                    if (frustum.Intersects(worldBounds))
                    {
                        const std::vector<Matrix>& skinTransforms = animComponent->BlendedSkinTransformsEXT;
                        for (ModelMesh* mesh : animComponent->ModelEXT->getMeshesProperty())
                        {
                            for (Effect* effect : mesh->getEffectsPropertyMutable())
                            {
                                if (auto* skinnedEffect = dynamic_cast<SkinnedEffect*>(effect))
                                {
                                    skinnedEffect->SetBoneTransforms(skinTransforms);
                                }
                                else if (auto* skinnedPbrEffect = dynamic_cast<SkinnedPbrEffect*>(effect))
                                {
                                    skinnedPbrEffect->SetBoneTransforms(skinTransforms);
                                }
                            }
                        }
                        animComponent->ModelEXT->Draw(world, view, projection);
                    }
                }
            }
        }
    }
}
