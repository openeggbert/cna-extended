// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/AnimationSystem3DEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/SkinnedModelComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"

#include <typeindex>

namespace CNA::Extended::World3DEXT
{
    using ECS::AspectBuilder;
    using ECS::ComponentManager;
    using ECS::Entity;
    using Microsoft::Xna::Framework::GameTime;

    AnimationSystem3DEXT::AnimationSystem3DEXT()
        : EntityUpdateSystem(AspectBuilder().All({std::type_index(typeid(SkinnedModelComponentEXT))}))
    {
    }

    void AnimationSystem3DEXT::Initialize(ComponentManager& componentManager)
    {
        (void)componentManager; // no cached ComponentMapper needed: Entity::Get<T>() below is cheap
    }

    void AnimationSystem3DEXT::Update(const GameTime& gameTime)
    {
        for (const int entityId : getActiveEntitiesProperty())
        {
            Entity* entity = GetEntity(entityId);
            if (entity == nullptr)
            {
                continue;
            }

            SkinnedModelComponentEXT* component = entity->Get<SkinnedModelComponentEXT>();
            if (component == nullptr || component->ModelEXT == nullptr || component->ClipNameEXT.empty())
            {
                continue;
            }

            component->PositionEXT = component->PositionEXT + gameTime.getElapsedGameTimeProperty();
            component->ModelEXT->ComputeBoneTransformsEXT(component->ClipNameEXT, component->PositionEXT,
                                                            component->LoopEXT, component->BoneTransformsEXT);
        }
    }
}
