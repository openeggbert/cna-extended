// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ParticleRenderSystem3DEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/BillboardMeshEXT.hpp"
#include "CNA/Extended/World3DEXT/BillboardRenderSystemEXT.hpp"
#include "CNA/Extended/World3DEXT/ParticleEffect3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ParticleEffectComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"

#include <cstdint>
#include <typeindex>

namespace CNA::Extended::World3DEXT
{
    using ECS::AspectBuilder;
    using ECS::ComponentManager;
    using ECS::Entity;
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;

    ParticleRenderSystem3DEXT::ParticleRenderSystem3DEXT(GraphicsDevice& graphicsDevice, BillboardRenderSystemEXT& billboardRenderSystem)
        : EntityDrawSystem(AspectBuilder().All({std::type_index(typeid(ParticleEffectComponentEXT))})),
          billboardRenderSystem_(&billboardRenderSystem),
          particleQuadEXT_(BuildBillboardQuadVertexBufferEXT(graphicsDevice, RectangleF(0.0f, 0.0f, 1.0f, 1.0f)))
    {
    }

    ParticleRenderSystem3DEXT::~ParticleRenderSystem3DEXT() = default;

    void ParticleRenderSystem3DEXT::Initialize(ComponentManager& componentManager)
    {
        (void)componentManager; // no cached ComponentMapper needed: Entity::Get<T>() below is cheap
    }

    void ParticleRenderSystem3DEXT::Draw(const GameTime& gameTime)
    {
        (void)gameTime;

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

            for (const auto& emitter : particleComponent->EffectEXT->getEmittersProperty())
            {
                for (const Particle3DEXT& particle : emitter->ParticlesEXT)
                {
                    billboardRenderSystem_->DrawBillboardEXT(*particleQuadEXT_, particleComponent->TextureEXT, particle.PositionEXT,
                                                              Vector2(particle.ScaleEXT, particle.ScaleEXT),
                                                              Color(particle.ColorEXT.getRProperty(), particle.ColorEXT.getGProperty(),
                                                                    particle.ColorEXT.getBProperty(),
                                                                    static_cast<std::uint8_t>(particle.OpacityEXT * 255.0f)));
                }
            }
        }
    }
}
