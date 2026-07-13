// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/SpriteBatchExtensions.hpp"

#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "CNA/Extended/HslColor.hpp"
#include "CNA/Extended/Particles/ParticleEffect.hpp"
#include "CNA/Extended/Particles/ParticleEmitter.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "System/ObjectDisposedException.hpp"

#include <vector>

namespace CNA::Extended::Particles
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::BlendState;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Microsoft::Xna::Framework::Graphics::SpriteEffects;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    namespace
    {
        void RenderParticle(
            SpriteBatch& spriteBatch, Data::Particle* particle, Texture2D* texture, const Rectangle& sourceRect, const Vector2& origin,
            const Vector2& offset)
        {
            const HslColor hsl(particle->Color[0], particle->Color[1], particle->Color[2]);
            Color color = HslColor::ToRgb(hsl);

            // BlendState has no value-equality override in either upstream or this port -- upstream's `==`
            // is therefore C#'s default *reference* equality (is the device's current BlendState literally
            // the same singleton object as BlendState.AlphaBlend), translated here as address identity.
            if (&spriteBatch.getGraphicsDeviceProperty()->getBlendStateProperty() == &BlendState::AlphaBlend)
            {
                color = color * particle->Opacity;
            }
            else
            {
                color.setAProperty(static_cast<SharpRuntime::bytecs>(MathHelper::Clamp(particle->Opacity * 255.0f, 0.0f, 255.0f)));
            }

            const Vector2 position = Vector2(particle->Position[0], particle->Position[1]) + offset;
            const Vector2 scale(particle->Scale[0], particle->Scale[1]);
            const float rotation = particle->Rotation;
            const float layerDepth = particle->LayerDepth;

            spriteBatch.Draw(*texture, position, sourceRect, color, rotation, origin, scale, SpriteEffects::None, layerDepth);
        }

        void UnsafeDraw(SpriteBatch& spriteBatch, ParticleEmitter& emitter)
        {
            if (!emitter.getTextureRegionProperty())
            {
                return;
            }

            if (emitter.getActiveParticlesProperty() == 0)
            {
                return;
            }

            if (!emitter.getVisibleProperty())
            {
                return;
            }

            const std::shared_ptr<Graphics::Texture2DRegion>& region = emitter.getTextureRegionProperty();
            Texture2D* texture = region->getTextureProperty();
            const Rectangle sourceRect = region->getBoundsProperty();
            const Vector2 origin = Vector2(static_cast<float>(region->getWidthProperty()), static_cast<float>(region->getHeightProperty())) * 0.5f;

            if (emitter.getRenderingOrderProperty() == ParticleRenderingOrder::FrontToBack)
            {
                const int count = emitter.getActiveParticlesProperty();

                std::vector<Data::Particle*> particlePtrs(static_cast<std::size_t>(count));

                ParticleIterator& iterator = emitter.getBufferProperty().getIteratorProperty();
                int index = 0;

                while (iterator.getHasNextProperty())
                {
                    particlePtrs[static_cast<std::size_t>(index++)] = iterator.Next();
                }

                for (int i = count - 1; i >= 0; --i)
                {
                    RenderParticle(spriteBatch, particlePtrs[static_cast<std::size_t>(i)], texture, sourceRect, origin, emitter.getOffsetProperty());
                }
            }
            else
            {
                ParticleIterator& iterator = emitter.getBufferProperty().getIteratorProperty();

                while (iterator.getHasNextProperty())
                {
                    Data::Particle* particle = iterator.Next();
                    RenderParticle(spriteBatch, particle, texture, sourceRect, origin, emitter.getOffsetProperty());
                }
            }
        }
    }

    void Draw(SpriteBatch& spriteBatch, ParticleEffect& effect)
    {
        System::ObjectDisposedException::ThrowIf(effect.getIsDisposedProperty(), "effect");

        for (const std::unique_ptr<ParticleEmitter>& emitter : effect.getEmittersProperty())
        {
            UnsafeDraw(spriteBatch, *emitter);
        }
    }

    void Draw(SpriteBatch& spriteBatch, ParticleEmitter& emitter)
    {
        System::ObjectDisposedException::ThrowIf(emitter.getIsDisposedProperty(), "emitter");
        UnsafeDraw(spriteBatch, emitter);
    }
}
