// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CubeMeshRenderSystemEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md and CubeMeshComponentEXT.hpp's own header comment for the design.
#pragma once

#include "CNA/Extended/ECS/Systems/EntityDrawSystem.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"

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

    private:
        Microsoft::Xna::Framework::Graphics::GraphicsDevice* graphicsDevice_;
        Camera3DEXT* camera_;
        Microsoft::Xna::Framework::Graphics::VertexBuffer cubeVertexBuffer_;
        Microsoft::Xna::Framework::Graphics::IndexBuffer cubeIndexBuffer_;
        Microsoft::Xna::Framework::Graphics::BasicEffect effect_;
    };
}
