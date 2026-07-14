// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ParticleRenderSystem3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Draws every active particle across every entity's ParticleEffectComponentEXT as a
// camera-facing billboard, via BillboardRenderSystemEXT::DrawBillboardEXT -- the exact
// same draw path BillboardComponentEXT entities use, not a new one (matching plan3d.md's
// Phase 7 requirement). Owns one shared, full-texture ([0,1] UV, no atlas sub-region --
// see ParticleEffectComponentEXT.hpp's "one texture per effect" scope note) quad
// VertexBuffer, built once in the constructor via BillboardMeshEXT.hpp's
// BuildBillboardQuadVertexBufferEXT, reused for every particle.
#pragma once

#include "CNA/Extended/ECS/Systems/EntityDrawSystem.hpp"

#include <memory>

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
    class VertexBuffer;
}

namespace CNA::Extended::World3DEXT
{
    class BillboardRenderSystemEXT;

    /**
     * @brief Draws every active particle across every entity's ParticleEffectComponentEXT
     * as a camera-facing billboard.
     * @see ParticleEffectComponentEXT, the component this system reads.
     * @see BillboardRenderSystemEXT, whose DrawBillboardEXT this system draws through.
     */
    class ParticleRenderSystem3DEXT final : public ECS::Systems::EntityDrawSystem
    {
    public:
        /**
         * @param graphicsDevice The device the shared particle quad is uploaded to. Not owned; must outlive this system.
         * @param billboardRenderSystem The system this one draws every particle billboard through. Not owned; must outlive this system.
         */
        ParticleRenderSystem3DEXT(Microsoft::Xna::Framework::Graphics::GraphicsDevice& graphicsDevice,
                                   BillboardRenderSystemEXT& billboardRenderSystem);
        ~ParticleRenderSystem3DEXT() override;

        using ECS::Systems::EntityDrawSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        BillboardRenderSystemEXT* billboardRenderSystem_;
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer> particleQuadEXT_;
    };
}
