// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ModelAnimationSystem3DEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/ModelAnimationComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"

#include <algorithm>
#include <typeindex>

namespace CNA::Extended::World3DEXT
{
    using ECS::AspectBuilder;
    using ECS::ComponentManager;
    using ECS::Entity;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Matrix;
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
                // Snapshot the outgoing pose *before* switching, so the blend below has something
                // to fade from. Skipped for the very first clip a component ever plays (nothing to
                // blend from) and when BlendDurationEXT is 0 (an explicit hard cut).
                if (playingClip != nullptr && component->BlendDurationEXT > 0.0F)
                {
                    component->BlendFromSkinTransformsEXT = component->PlayerEXT.GetSkinTransforms();
                    component->BlendElapsedEXT = 0.0F;
                }
                else
                {
                    component->BlendFromSkinTransformsEXT.clear();
                }
                component->PlayerEXT.StartClip(clipIt->second);
            }

            component->PlayerEXT.Update(gameTime.getElapsedGameTimeProperty(), true, component->LoopEXT);

            const auto& target = component->PlayerEXT.GetSkinTransforms();
            if (!component->BlendFromSkinTransformsEXT.empty())
            {
                component->BlendElapsedEXT += static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
                const float t = std::clamp(component->BlendElapsedEXT / component->BlendDurationEXT, 0.0F, 1.0F);

                const std::size_t boneCount = std::min(component->BlendFromSkinTransformsEXT.size(), target.size());
                component->BlendedSkinTransformsEXT.resize(boneCount);
                for (std::size_t i = 0; i < boneCount; ++i)
                {
                    component->BlendedSkinTransformsEXT[i] =
                        Matrix::Lerp(component->BlendFromSkinTransformsEXT[i], target[i], t);
                }

                if (t >= 1.0F)
                {
                    component->BlendFromSkinTransformsEXT.clear();
                }
            }
            else
            {
                component->BlendedSkinTransformsEXT = target;
            }
        }
    }
}
