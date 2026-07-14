// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::BillboardComponentEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Graphics::Sprite: a camera-facing textured quad, driven
// by Matrix::CreateBillboard (real CNA/XNA math) each frame by BillboardRenderSystemEXT.
// Adapted from cna-scene::BillboardMesh/BillboardMeshRenderer's always-face-camera concept
// (read as reference material only; see CubeMeshComponentEXT.hpp's header comment for why
// no NOTICE.md entry is needed for cna-scene-informed design) -- re-implemented very
// differently: cna-scene's BillboardMesh/BillboardMeshRenderer is an immediate-mode CPU
// mesh builder + one-shot GPU uploader with no ECS integration, whereas this component
// pairs with a per-entity VertexBufferEXT (baking that entity's own UV rect, since
// different billboards/atlas frames need different UVs -- see
// AnimatedBillboardComponentEXT.hpp for how the animated case updates it) and a shared
// IndexBuffer/BasicEffect owned by BillboardRenderSystemEXT.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
    class VertexBuffer;
}

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief ECS component for a camera-facing textured quad ("billboard"), drawn by
     * BillboardRenderSystemEXT using Matrix::CreateBillboard each frame.
     * @see BillboardRenderSystemEXT, which draws this component.
     * @see BuildBillboardQuadVertexBufferEXT, the helper that builds VertexBufferEXT.
     */
    struct BillboardComponentEXT
    {
        /** @brief The texture sampled by VertexBufferEXT's baked-in UV coordinates. Not owned by this component. */
        Microsoft::Xna::Framework::Graphics::Texture2D* TextureEXT = nullptr;

        /**
         * @brief A 4-vertex, local-space (-0.5..0.5 on X/Y, Z=0) quad with this entity's own UV
         * rect baked in, built via BuildBillboardQuadVertexBufferEXT. Not owned by this component
         * -- caller manages lifetime, matching every other component in this ECS.
         */
        Microsoft::Xna::Framework::Graphics::VertexBuffer* VertexBufferEXT = nullptr;

        /** @brief The billboard's world-space size (scales the local -0.5..0.5 quad). */
        Microsoft::Xna::Framework::Vector2 SizeEXT{1.0f, 1.0f};

        /** @brief Tint multiplied with the sampled texture color. */
        Microsoft::Xna::Framework::Color TintEXT = Microsoft::Xna::Framework::Color::White;
    };
}
