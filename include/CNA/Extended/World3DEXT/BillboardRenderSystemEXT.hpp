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
#pragma once

#include "CNA/Extended/ECS/Systems/EntityDrawSystem.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"

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

    private:
        Microsoft::Xna::Framework::Graphics::GraphicsDevice* graphicsDevice_;
        Camera3DEXT* camera_;
        Microsoft::Xna::Framework::Graphics::IndexBuffer quadIndexBuffer_;
        Microsoft::Xna::Framework::Graphics::BasicEffect effect_;
    };
}
