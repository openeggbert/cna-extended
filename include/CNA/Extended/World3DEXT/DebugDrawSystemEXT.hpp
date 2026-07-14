// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::DebugDrawSystemEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md and DebugDrawComponentEXT.hpp's own header comment for the design.
//
// Each frame, gathers every active entity's DebugDrawComponentEXT::LinesEXT into one CPU
// buffer and draws it as a single PrimitiveType::LineList via one shared, growth-only
// VertexBuffer (reallocated only when the line count exceeds current capacity) and one
// shared BasicEffect (VertexColorEnabled=true, unlit).
#pragma once

#include "CNA/Extended/ECS/Systems/EntityDrawSystem.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"

#include <memory>

namespace Microsoft::Xna::Framework::Graphics
{
    class VertexBuffer;
}

namespace CNA::Extended::World3DEXT
{
    class Camera3DEXT;

    /**
     * @brief Draws every active entity's DebugDrawComponentEXT::LinesEXT each frame as a
     * single batched line list.
     * @see DebugDrawComponentEXT, the component this system draws.
     */
    class DebugDrawSystemEXT final : public ECS::Systems::EntityDrawSystem
    {
    public:
        /**
         * @param graphicsDevice The device lines are drawn through. Not owned; must outlive this system.
         * @param camera The camera supplying view/projection each frame. Not owned; must outlive this system.
         */
        DebugDrawSystemEXT(Microsoft::Xna::Framework::Graphics::GraphicsDevice& graphicsDevice, Camera3DEXT& camera);
        ~DebugDrawSystemEXT() override;

        using ECS::Systems::EntityDrawSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        Microsoft::Xna::Framework::Graphics::GraphicsDevice* graphicsDevice_;
        Camera3DEXT* camera_;
        Microsoft::Xna::Framework::Graphics::BasicEffect effect_;
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer> lineVertexBuffer_;
        int lineVertexCapacity_ = 0;
    };
}
