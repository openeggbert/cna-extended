// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::RenderSystem3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Builds on: CNA::Extended::ECS::Systems::EntityDrawSystem (this project's already-ported
// ECS draw-system base), Camera3DEXT (view/projection/frustum), Transform3ComponentEXT
// (per-entity world matrix), ModelComponentEXT (what to draw), and
// Microsoft::Xna::Framework::Graphics::Model::Draw (real CNA model rendering, which
// already forwards World/View/Projection to any Effect implementing IEffectMatrices --
// see ModelComponentEXT.hpp's header comment for why no separate Effect* is needed here).
//
// Each frame: for every entity with a ModelComponentEXT, the world matrix is read from
// its Transform3ComponentEXT if it has one (identity otherwise), the model's local-space
// BoundsEXT is transformed into world space, and a BoundingFrustum::Intersects test
// against the camera's current frustum decides whether to draw -- entities fully outside
// the frustum are skipped, matching the frustum-culling scope decision recorded in
// plan3d.md section 2. No GraphicsDevice reference is needed here: Model::Draw()'s
// underlying ModelMesh::Draw() already issues its SetVertexBuffer/DrawIndexedPrimitives
// calls through the GraphicsDevice* each mesh was constructed with (see ModelMesh.cpp).
#pragma once

#include "CNA/Extended/ECS/Systems/EntityDrawSystem.hpp"

namespace CNA::Extended::World3DEXT
{
    class Camera3DEXT;

    /**
     * @brief Draws every entity with a ModelComponentEXT via Camera3DEXT's current
     * view/projection, frustum-culling entities whose transformed BoundsEXT does not
     * intersect the camera's BoundingFrustum.
     * @see ModelComponentEXT, the component this system draws.
     * @see Transform3ComponentEXT, the optional component supplying the world matrix.
     */
    class RenderSystem3DEXT final : public ECS::Systems::EntityDrawSystem
    {
    public:
        /** @param camera The camera supplying view/projection/frustum each frame. Not owned; must outlive this system. */
        explicit RenderSystem3DEXT(Camera3DEXT& camera);

        using ECS::Systems::EntityDrawSystem::Initialize;
        void Initialize(ECS::ComponentManager& componentManager) override;
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        Camera3DEXT* camera_;
    };
}
