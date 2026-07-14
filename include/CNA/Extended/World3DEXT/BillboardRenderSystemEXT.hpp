// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::BillboardRenderSystemEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md and BillboardComponentEXT.hpp's own header comment for the design.
//
// Each frame: for every entity with a BillboardComponentEXT, computes
// Matrix::CreateScale(SizeEXT) * Matrix::CreateBillboard(objectWorldPosition, camera
// position, camera up, nullopt) -- real CNA/XNA billboard math, always facing the camera
// regardless of the entity's own Transform3ComponentEXT rotation (matching real billboard
// semantics: only the entity's world *position* is used, not its rotation). Owns one
// shared IndexBuffer (the constant {0,1,2,0,2,3} quad winding every BillboardComponentEXT's
// VertexBufferEXT was built against, see BillboardMeshEXT.hpp) and one shared BasicEffect
// (texture/tint swapped per draw call, matching CubeMeshRenderSystemEXT's identical pattern).
//
// DrawBillboardEXT exposes that same per-billboard draw step publicly, so other systems can
// draw ad-hoc billboards (not backed by a BillboardComponentEXT entity) through this exact
// path instead of duplicating it -- ParticleRenderSystem3DEXT uses this for per-particle
// billboards, matching plan3d.md's Phase 7 requirement ("via Graphics3DEXT::
// BillboardRenderSystemEXT's existing draw path, not a new one").
#pragma once

#include "CNA/Extended/ECS/Systems/EntityDrawSystem.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"

namespace Microsoft::Xna::Framework
{
    struct Vector2;
    struct Vector3;
}

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
    class VertexBuffer;
}

namespace CNA::Extended::World3DEXT
{
    class Camera3DEXT;

    /**
     * @brief Draws every entity with a BillboardComponentEXT as a camera-facing textured
     * quad.
     * @see BillboardComponentEXT, the component this system draws.
     * @see Transform3ComponentEXT, the optional component supplying the world position.
     */
    class BillboardRenderSystemEXT final : public ECS::Systems::EntityDrawSystem
    {
    public:
        /**
         * @param graphicsDevice The device billboards are drawn through. Not owned; must outlive this system.
         * @param camera The camera billboards face and whose view/projection drives their draw. Not owned; must outlive this system.
         */
        BillboardRenderSystemEXT(Microsoft::Xna::Framework::Graphics::GraphicsDevice& graphicsDevice, Camera3DEXT& camera);

        using ECS::Systems::EntityDrawSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

        /**
         * @brief Draws one camera-facing textured quad through this system's shared
         * IndexBuffer/BasicEffect -- the exact same draw step Draw() uses per
         * BillboardComponentEXT entity, exposed for callers that don't have an ECS entity
         * (e.g. ParticleRenderSystem3DEXT's individual particles).
         * @param vertexBuffer A 4-vertex local-space quad with UV baked in (see BillboardMeshEXT.hpp).
         * @param texture The texture to sample. May be null.
         * @param worldPosition The billboard's world-space position (only position, not rotation, matters for a billboard).
         * @param size The billboard's world-space size.
         * @param tint Tint multiplied with the sampled texture color.
         */
        void DrawBillboardEXT(Microsoft::Xna::Framework::Graphics::VertexBuffer& vertexBuffer,
                               Microsoft::Xna::Framework::Graphics::Texture2D* texture,
                               const Microsoft::Xna::Framework::Vector3& worldPosition,
                               const Microsoft::Xna::Framework::Vector2& size, const Microsoft::Xna::Framework::Color& tint);

    private:
        Microsoft::Xna::Framework::Graphics::GraphicsDevice* graphicsDevice_;
        Camera3DEXT* camera_;
        Microsoft::Xna::Framework::Graphics::IndexBuffer quadIndexBuffer_;
        Microsoft::Xna::Framework::Graphics::BasicEffect effect_;
    };
}
