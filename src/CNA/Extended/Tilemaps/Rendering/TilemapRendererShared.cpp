// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/TilemapRendererShared.cs. See
// TilemapRendererShared.hpp for design notes.
#include "CNA/Extended/Tilemaps/Rendering/TilemapRendererShared.hpp"

#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <cstdint>
#include <limits>

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::IndexBuffer;
    using Microsoft::Xna::Framework::Graphics::IndexElementSize;
    using Microsoft::Xna::Framework::Graphics::VertexBuffer;

    namespace
    {
        bool CanUseSixteenBitIndices(const std::vector<VertexPositionColorTexture>& vertices, const std::vector<int>& indices)
        {
            if (vertices.size() > std::numeric_limits<std::uint16_t>::max())
            {
                return false;
            }

            for (const int index : indices)
            {
                if (static_cast<std::uint32_t>(index) > std::numeric_limits<std::uint16_t>::max())
                {
                    return false;
                }
            }

            return true;
        }
    }

    LayerModel CreateLayerModel(GraphicsDevice& graphicsDevice,
                                 const std::vector<VertexPositionColorTexture>& vertices,
                                 const std::vector<int>& indices,
                                 Texture2D* texture)
    {
        VertexBuffer vertexBuffer(graphicsDevice, VertexPositionColorTexture::getVertexDeclarationStatic(),
                                   static_cast<int>(vertices.size()), BufferUsage::WriteOnly);
        vertexBuffer.SetData(vertices.data(), static_cast<int>(vertices.size()));

        if (CanUseSixteenBitIndices(vertices, indices))
        {
            std::vector<std::uint16_t> shortIndices(indices.size());
            for (std::size_t i = 0; i < indices.size(); ++i)
            {
                shortIndices[i] = static_cast<std::uint16_t>(indices[i]);
            }

            IndexBuffer indexBuffer(graphicsDevice, IndexElementSize::SixteenBits, static_cast<int>(shortIndices.size()), BufferUsage::WriteOnly);
            indexBuffer.SetData(shortIndices.data(), static_cast<int>(shortIndices.size()));

            return LayerModel(std::move(vertexBuffer), std::move(indexBuffer), texture, static_cast<int>(indices.size()) / 3);
        }

        // 32-bit indices support groups larger than 16,383 tiles (the 16-bit limit).
        std::vector<std::uint32_t> longIndices(indices.size());
        for (std::size_t i = 0; i < indices.size(); ++i)
        {
            longIndices[i] = static_cast<std::uint32_t>(indices[i]);
        }

        IndexBuffer indexBuffer(graphicsDevice, IndexElementSize::ThirtyTwoBits, static_cast<int>(longIndices.size()), BufferUsage::WriteOnly);
        indexBuffer.SetData(longIndices.data(), static_cast<int>(longIndices.size()));

        return LayerModel(std::move(vertexBuffer), std::move(indexBuffer), texture, static_cast<int>(indices.size()) / 3);
    }

    void AddTileQuad(std::vector<VertexPositionColorTexture>& vertices,
                      std::vector<int>& indices,
                      const Vector2& position,
                      int width,
                      int height,
                      const Rectangle& sourceRect,
                      TilemapTileFlipFlags flipFlags,
                      const Texture2D& texture,
                      const Color& color)
    {
        const Vector3 topLeft(position.X, position.Y, 0.0f);
        const Vector3 topRight(position.X + static_cast<float>(width), position.Y, 0.0f);
        const Vector3 bottomLeft(position.X, position.Y + static_cast<float>(height), 0.0f);
        const Vector3 bottomRight(position.X + static_cast<float>(width), position.Y + static_cast<float>(height), 0.0f);

        const std::array<Vector2, 4> uvs = CalculateTextureCoordinates(sourceRect, flipFlags, texture);

        const auto vertexOffset = static_cast<int>(vertices.size());
        vertices.emplace_back(topLeft, color, uvs[0]);
        vertices.emplace_back(topRight, color, uvs[1]);
        vertices.emplace_back(bottomLeft, color, uvs[2]);
        vertices.emplace_back(bottomRight, color, uvs[3]);

        // Counter-clockwise winding matches MonoGame's default CullCounterClockwiseFace rasterizer state.
        indices.push_back(vertexOffset);
        indices.push_back(vertexOffset + 1);
        indices.push_back(vertexOffset + 2);
        indices.push_back(vertexOffset + 1);
        indices.push_back(vertexOffset + 3);
        indices.push_back(vertexOffset + 2);
    }

    std::array<Vector2, 4> CalculateTextureCoordinates(const Rectangle& sourceRect, TilemapTileFlipFlags flipFlags, const Texture2D& texture)
    {
        // Normalize source rectangle to 0-1 UV range.
        // Direct edge-to-edge mapping is correct for PointClamp: pixel centers are at
        // half-integer positions and never coincide with a UV boundary, so no texel inset
        // is needed. An inset would compress n texels into an n-1 texel UV span, causing
        // some screen pixels to sample the wrong texel at non-1:1 display scales.
        const float left = static_cast<float>(sourceRect.getLeftProperty()) / static_cast<float>(texture.getWidthProperty());
        const float right = static_cast<float>(sourceRect.getRightProperty()) / static_cast<float>(texture.getWidthProperty());
        const float top = static_cast<float>(sourceRect.getTopProperty()) / static_cast<float>(texture.getHeightProperty());
        const float bottom = static_cast<float>(sourceRect.getBottomProperty()) / static_cast<float>(texture.getHeightProperty());
        std::array<Vector2, 4> destinationUvs{};

        for (int sourceIndex = 0; sourceIndex < 4; ++sourceIndex)
        {
            int x = sourceIndex % 2;
            int y = sourceIndex / 2;
            const float u = x == 0 ? left : right;
            const float v = y == 0 ? top : bottom;

            // Tiled applies the diagonal flip first, then horizontal and vertical flips.
            if ((flipFlags & TilemapTileFlipFlags::FlipDiagonally) != TilemapTileFlipFlags::None)
            {
                std::swap(x, y);
            }

            if ((flipFlags & TilemapTileFlipFlags::FlipHorizontally) != TilemapTileFlipFlags::None)
            {
                x = 1 - x;
            }

            if ((flipFlags & TilemapTileFlipFlags::FlipVertically) != TilemapTileFlipFlags::None)
            {
                y = 1 - y;
            }

            destinationUvs[static_cast<std::size_t>(y * 2 + x)] = Vector2(u, v);
        }

        return destinationUvs;
    }

    const SamplerState* GetWrapSamplerState(const SamplerState* samplerState)
    {
        if (samplerState == &SamplerState::PointClamp)
        {
            return &SamplerState::PointWrap;
        }

        if (samplerState == &SamplerState::LinearClamp)
        {
            return &SamplerState::LinearWrap;
        }

        // If the caller already configured a wrap state (or any other custom state), use it as-is.
        return samplerState;
    }
}
