// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CubeMeshRenderSystemEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md and CubeMeshComponentEXT.hpp's own header comment for the design.
//
// DrawCubeEXT exposes the per-cube draw step publicly (mirroring
// BillboardRenderSystemEXT::DrawBillboardEXT's identical role, added in Phase 7), so other
// systems can draw ad-hoc cubes through this exact shared mesh/effect instead of
// duplicating it -- TilemapRenderer3DEXT (Phase 8) uses this per visible tile, matching
// plan3d.md's requirement that tile rendering batch into CubeMeshComponentEXT's existing
// draw path, not a new one.
#pragma once

#include "CNA/Extended/ECS/Systems/EntityDrawSystem.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"

namespace Microsoft::Xna::Framework
{
    struct Matrix;
}

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::World3DEXT
{
    class Camera3DEXT;

    /**
     * @brief Draws every entity with a CubeMeshComponentEXT as an axis-aligned textured box,
     * scaled by SizeEXT, using one shared unit-cube mesh and BasicEffect (texture/tint swapped
     * per draw call).
     * @see CubeMeshComponentEXT, the component this system draws.
     * @see Transform3ComponentEXT, the optional component supplying the world matrix.
     */
    class CubeMeshRenderSystemEXT final : public ECS::Systems::EntityDrawSystem
    {
    public:
        /**
         * @param graphicsDevice The device the shared cube mesh is uploaded to and drawn through. Not owned; must outlive this system.
         * @param camera The camera supplying view/projection/frustum each frame. Not owned; must outlive this system.
         */
        CubeMeshRenderSystemEXT(Microsoft::Xna::Framework::Graphics::GraphicsDevice& graphicsDevice, Camera3DEXT& camera);

        using ECS::Systems::EntityDrawSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

        /**
         * @brief Draws one textured box through this system's shared unit-cube mesh/
         * BasicEffect -- the exact same draw step Draw() uses per CubeMeshComponentEXT
         * entity, exposed for callers that don't have an ECS entity (e.g.
         * TilemapRenderer3DEXT's individual tiles). Frustum culling is the caller's
         * responsibility (matching Draw()'s own entity loop, which culls before reaching
         * this same draw step).
         * @param texture The texture applied to every face. May be null.
         * @param world The cube's fully-composed world matrix (typically
         * Matrix::CreateScale(size) * a position/rotation matrix -- this method applies no
         * further scale/rotation/translation of its own).
         * @param tint Tint multiplied with the sampled texture color.
         */
        void DrawCubeEXT(Microsoft::Xna::Framework::Graphics::Texture2D* texture, const Microsoft::Xna::Framework::Matrix& world,
                          const Microsoft::Xna::Framework::Color& tint);

    private:
        Microsoft::Xna::Framework::Graphics::GraphicsDevice* graphicsDevice_;
        Camera3DEXT* camera_;
        Microsoft::Xna::Framework::Graphics::VertexBuffer cubeVertexBuffer_;
        Microsoft::Xna::Framework::Graphics::IndexBuffer cubeIndexBuffer_;
        Microsoft::Xna::Framework::Graphics::BasicEffect effect_;
    };
}
