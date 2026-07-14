// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CubeMeshComponentEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Graphics::Sprite for axis-aligned textured cubes/tiles
// (the "cube/tile terrain" use case cna-scene::CubeMesh/CubeBatch were designed for --
// read as reference material, not linked; see this file's own copyright note below).
// CubeMeshRenderSystemEXT owns one shared unit-cube VertexBuffer/IndexBuffer/BasicEffect
// (matching TilemapRenderer's/AvatarRenderer's established "one shared effect, texture
// swapped per draw call" pattern) -- this component only carries per-instance state
// (texture, world-space size, tint), not its own GPU geometry.
//
// cna-scene and cna-extended share the same copyright holder (Robert Vokáč) -- unlike
// the genuinely third-party MIT code this project's NOTICE.md documents (SlimMath,
// nickgravelyn's Triangulator, artemis_CSharp), reusing cna-scene's own design here is
// the project owner's own code informing his own other project, not third-party
// attribution; no NOTICE.md entry is needed (resolves 3d.md section 9.2's deferred
// question). This is also a re-implementation, not a copy: cna-scene's CubeMesh/CubeBatch
// are an immediate-mode batching API with no GPU-buffer-per-instance concept; this file's
// shape (an ECS component + a system owning one shared unit-cube mesh) is CubeMeshComponentEXT's own design.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief ECS component for an axis-aligned textured cube, drawn by CubeMeshRenderSystemEXT
     * using its own shared unit-cube geometry scaled by SizeEXT.
     * @see CubeMeshRenderSystemEXT, which draws this component.
     */
    struct CubeMeshComponentEXT
    {
        /** @brief The texture applied to every face of the cube. Not owned by this component. */
        Microsoft::Xna::Framework::Graphics::Texture2D* TextureEXT = nullptr;

        /** @brief The cube's world-space size along each axis. */
        Microsoft::Xna::Framework::Vector3 SizeEXT{1.0f, 1.0f, 1.0f};

        /** @brief Tint multiplied with the sampled texture color. */
        Microsoft::Xna::Framework::Color TintEXT = Microsoft::Xna::Framework::Color::White;
    };
}
