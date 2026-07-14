// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT -- billboard quad mesh helpers, new, non-upstream addition.
// See 3d.md/plan3d.md and BillboardComponentEXT.hpp's own header comment for the design.
#pragma once

#include "CNA/Extended/RectangleF.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionTexture.hpp"

#include <array>
#include <memory>

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
    class VertexBuffer;
}

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Converts a pixel-space source rectangle within a texture into a normalized
     * [0,1] UV rectangle.
     * @param pixelRect The source rectangle, in pixels.
     * @param textureWidth The texture's width, in pixels.
     * @param textureHeight The texture's height, in pixels.
     */
    [[nodiscard]] RectangleF ConvertPixelRectToUvRectEXT(const Microsoft::Xna::Framework::Rectangle& pixelRect, int textureWidth, int textureHeight);

    /**
     * @brief Builds the 4 local-space quad vertices (-0.5..0.5 on X/Y, Z=0) for a billboard,
     * with @p uvRect baked in. Vertex order matches the 6-index {0,1,2,0,2,3} triangle fan
     * BillboardRenderSystemEXT's shared IndexBuffer uses.
     */
    [[nodiscard]] std::array<Microsoft::Xna::Framework::Graphics::VertexPositionTexture, 4> BuildBillboardQuadVerticesEXT(const RectangleF& uvRect);

    /** @brief Convenience: builds a new VertexBuffer holding BuildBillboardQuadVerticesEXT(uvRect)'s output. */
    [[nodiscard]] std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer> BuildBillboardQuadVertexBufferEXT(
        Microsoft::Xna::Framework::Graphics::GraphicsDevice& graphicsDevice, const RectangleF& uvRect);
}
