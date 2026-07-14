// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::TextBillboardRenderSystemEXT -- new, non-upstream addition.
// See 3d.md/plan3d.md and Text3DEXT.hpp's own header comment for the design.
//
// Draws each entity's Text3DEXT mesh as a single camera-facing billboard (Matrix::
// CreateBillboard, the same real CNA/XNA math BillboardRenderSystemEXT uses), scaled by
// Text3DEXT::ScaleEXT. Owns one shared BasicEffect (texture/tint swapped per draw call,
// matching CubeMeshRenderSystemEXT's/BillboardRenderSystemEXT's identical pattern) --
// unlike BillboardRenderSystemEXT, no shared IndexBuffer: each Text3DEXT's own
// VertexBufferEXT/IndexBufferEXT (built by BuildText3DMeshEXT) already encode however many
// glyph quads that string needs.
//
// Known simplification: no frustum culling here (unlike RenderSystem3DEXT/
// CubeMeshRenderSystemEXT/BillboardRenderSystemEXT) -- Text3DEXT has no cached bounds
// (BuildText3DMeshEXT doesn't compute the mesh's local extents), and text labels are
// rarely numerous enough for culling to matter. Can be added later the same way the other
// systems already do it, if a real need appears.
#pragma once

#include "CNA/Extended/ECS/Systems/EntityDrawSystem.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"

namespace CNA::Extended::World3DEXT
{
    class Camera3DEXT;

    /**
     * @brief Draws every entity with a Text3DEXT as a camera-facing billboarded text mesh.
     * @see Text3DEXT, the component this system draws.
     * @see Transform3ComponentEXT, the optional component supplying the world position.
     */
    class TextBillboardRenderSystemEXT final : public ECS::Systems::EntityDrawSystem
    {
    public:
        /**
         * @param graphicsDevice The device text meshes are drawn through. Not owned; must outlive this system.
         * @param camera The camera text labels face and whose view/projection drives their draw. Not owned; must outlive this system.
         */
        TextBillboardRenderSystemEXT(Microsoft::Xna::Framework::Graphics::GraphicsDevice& graphicsDevice, Camera3DEXT& camera);

        using ECS::Systems::EntityDrawSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        Microsoft::Xna::Framework::Graphics::GraphicsDevice* graphicsDevice_;
        Camera3DEXT* camera_;
        Microsoft::Xna::Framework::Graphics::BasicEffect effect_;
    };
}
