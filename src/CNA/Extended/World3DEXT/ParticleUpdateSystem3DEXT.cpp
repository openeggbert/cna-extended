// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ParticleUpdateSystem3DEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/ParticleEffect3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ParticleEffectComponentEXT.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"

#include <typeindex>

namespace CNA::Extended::World3DEXT
{
    using ECS::AspectBuilder;
    using ECS::ComponentManager;
    using ECS::Entity;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Vector3;

    ParticleUpdateSystem3DEXT::ParticleUpdateSystem3DEXT()
        : EntityUpdateSystem(AspectBuilder().All({std::type_index(typeid(ParticleEffectComponentEXT))}))
    {
    }

    void ParticleUpdateSystem3DEXT::Initialize(ComponentManager& componentManager)
    {
        (void)componentManager; // no cached ComponentMapper needed: Entity::Get<T>() below is cheap
    }

    void ParticleUpdateSystem3DEXT::Update(const GameTime& gameTime)
    {
        const float deltaSeconds = static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());

        for (const int entityId : getActiveEntitiesProperty())
        {
            Entity* entity = GetEntity(entityId);
            if (entity == nullptr)
            {
                continue;
            }

            ParticleEffectComponentEXT* particleComponent = entity->Get<ParticleEffectComponentEXT>();
            if (particleComponent == nullptr || particleComponent->EffectEXT == nullptr)
            {
                continue;
            }

            Vector3 origin = Vector3::Zero;
            if (Transform3ComponentEXT* transformComponent = entity->Get<Transform3ComponentEXT>())
            {
                origin = transformComponent->TransformEXT.getWorldPositionProperty();
            }

            particleComponent->EffectEXT->UpdateEXT(deltaSeconds, origin);
        }
    }
}
