// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::RenderSystem3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Builds on: CNA::Extended::ECS::Systems::EntityDrawSystem (this project's already-ported
// ECS draw-system base), Camera3DEXT (view/projection/frustum), Transform3ComponentEXT
// (per-entity world matrix), ModelComponentEXT/SkinnedModelComponentEXT/ModelAnimationComponentEXT
// (what to draw), and Microsoft::Xna::Framework::Graphics::Model::Draw (real CNA model
// rendering, which already forwards World/View/Projection to any Effect implementing
// IEffectMatrices -- see ModelComponentEXT.hpp's header comment for why no separate Effect*
// is needed for the unskinned path).
//
// Each frame: for every entity with a ModelComponentEXT, SkinnedModelComponentEXT, and/or
// ModelAnimationComponentEXT, the world matrix is read from its Transform3ComponentEXT if it
// has one (identity otherwise), the relevant component's local-space BoundsEXT is transformed
// into world space, and a BoundingFrustum::Intersects test against the camera's current
// frustum decides whether to draw -- entities fully outside the frustum are skipped, matching
// the frustum-culling scope decision recorded in plan3d.md section 2.
//
// The three paths draw differently: ModelComponentEXT draws via Model::Draw(world, view,
// projection), which needs no GraphicsDevice reference here (ModelMesh::Draw() already
// issues its SetVertexBuffer/DrawIndexedPrimitives calls through the GraphicsDevice* each
// mesh was constructed with -- see ModelMesh.cpp). SkinnedModelComponentEXT has no such
// built-in Draw(): it is drawn by hand, following AvatarRenderer::DrawRealEXT's exact,
// already-working pattern (AvatarRenderer.cpp) -- one shared SkinnedEffect set once
// (World/View/Projection/BoneTransforms), then per SkinnedModelEXT::Parts entry: Apply()
// + SetVertexBuffer/SetIndexBuffer/DrawIndexedPrimitives -- which does need a
// GraphicsDevice reference, held here for exactly that path. ModelAnimationComponentEXT
// draws through Model::Draw() too (its Model already carries a real SkinnedEffect/
// SkinnedPbrEffect per skinned part, assigned at content-load time) -- this system only
// needs to push PlayerEXT's freshly computed GetSkinTransforms() onto each of the model's
// existing per-mesh Effects (via SetBoneTransforms(), a SkinnedEffect/SkinnedPbrEffect-only
// method IEffectMatrices/Model::Draw don't know about) immediately before calling Draw(),
// not build a whole new by-hand draw loop the way SkinnedModelComponentEXT needs.
#pragma once

#include "CNA/Extended/ECS/Systems/EntityDrawSystem.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
}

namespace CNA::Extended::World3DEXT
{
    class Camera3DEXT;

    /**
     * @brief Draws every entity with a ModelComponentEXT and/or SkinnedModelComponentEXT
     * via Camera3DEXT's current view/projection, frustum-culling entities whose
     * transformed bounds do not intersect the camera's BoundingFrustum.
     * @see ModelComponentEXT, SkinnedModelComponentEXT, the components this system draws.
     * @see Transform3ComponentEXT, the optional component supplying the world matrix.
     */
    class RenderSystem3DEXT final : public ECS::Systems::EntityDrawSystem
    {
    public:
        /**
         * @param graphicsDevice The device SkinnedModelComponentEXT parts are drawn through. Not owned; must outlive this system.
         * @param camera The camera supplying view/projection/frustum each frame. Not owned; must outlive this system.
         */
        RenderSystem3DEXT(Microsoft::Xna::Framework::Graphics::GraphicsDevice& graphicsDevice, Camera3DEXT& camera);

        using ECS::Systems::EntityDrawSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        Microsoft::Xna::Framework::Graphics::GraphicsDevice* graphicsDevice_;
        Camera3DEXT* camera_;
    };
}
