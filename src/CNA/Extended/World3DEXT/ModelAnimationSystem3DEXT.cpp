// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ModelAnimationSystem3DEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/ModelAnimationComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"

#include <typeindex>

namespace CNA::Extended::World3DEXT
{
    using ECS::AspectBuilder;
    using ECS::ComponentManager;
    using ECS::Entity;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Graphics::AnimationClip;

    ModelAnimationSystem3DEXT::ModelAnimationSystem3DEXT()
        : EntityUpdateSystem(AspectBuilder().All({std::type_index(typeid(ModelAnimationComponentEXT))}))
    {
    }

    void ModelAnimationSystem3DEXT::Initialize(ComponentManager& componentManager)
    {
        (void)componentManager; // no cached ComponentMapper needed: Entity::Get<T>() below is cheap
    }

    void ModelAnimationSystem3DEXT::Update(const GameTime& gameTime)
    {
        for (const int entityId : getActiveEntitiesProperty())
        {
            Entity* entity = GetEntity(entityId);
            if (entity == nullptr)
            {
                continue;
            }

            ModelAnimationComponentEXT* component = entity->Get<ModelAnimationComponentEXT>();
            if (component == nullptr || component->SkinningDataEXT == nullptr || component->ClipNameEXT.empty())
            {
                continue;
            }

            const auto clipIt = component->SkinningDataEXT->AnimationClips.find(component->ClipNameEXT);
            if (clipIt == component->SkinningDataEXT->AnimationClips.end())
            {
                continue; // unknown clip name: hold whatever pose PlayerEXT was already at
            }

            const AnimationClip* playingClip = component->PlayerEXT.getCurrentClipProperty();
            if (playingClip != &clipIt->second)
            {
                component->PlayerEXT.StartClip(clipIt->second);
            }

            component->PlayerEXT.Update(gameTime.getElapsedGameTimeProperty(), true, component->LoopEXT);
        }
    }
}
