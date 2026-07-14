// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/BillboardMeshEXT.hpp"

#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::VertexBuffer;
    using Microsoft::Xna::Framework::Graphics::VertexPositionTexture;

    RectangleF ConvertPixelRectToUvRectEXT(const Rectangle& pixelRect, int textureWidth, int textureHeight)
    {
        const float invWidth = textureWidth > 0 ? 1.0f / static_cast<float>(textureWidth) : 0.0f;
        const float invHeight = textureHeight > 0 ? 1.0f / static_cast<float>(textureHeight) : 0.0f;
        return {static_cast<float>(pixelRect.X) * invWidth, static_cast<float>(pixelRect.Y) * invHeight,
                static_cast<float>(pixelRect.Width) * invWidth, static_cast<float>(pixelRect.Height) * invHeight};
    }

    std::array<VertexPositionTexture, 4> BuildBillboardQuadVerticesEXT(const RectangleF& uvRect)
    {
        const float u0 = uvRect.X;
        const float v0 = uvRect.Y;
        const float u1 = uvRect.X + uvRect.Width;
        const float v1 = uvRect.Y + uvRect.Height;

        return {
            VertexPositionTexture(Vector3(-0.5f, 0.5f, 0.0f), Vector2(u0, v0)),
            VertexPositionTexture(Vector3(0.5f, 0.5f, 0.0f), Vector2(u1, v0)),
            VertexPositionTexture(Vector3(0.5f, -0.5f, 0.0f), Vector2(u1, v1)),
            VertexPositionTexture(Vector3(-0.5f, -0.5f, 0.0f), Vector2(u0, v1)),
        };
    }

    std::unique_ptr<VertexBuffer> BuildBillboardQuadVertexBufferEXT(GraphicsDevice& graphicsDevice, const RectangleF& uvRect)
    {
        const std::array<VertexPositionTexture, 4> vertices = BuildBillboardQuadVerticesEXT(uvRect);
        auto vertexBuffer = std::make_unique<VertexBuffer>(graphicsDevice, 4);
        vertexBuffer->SetData(vertices.data(), 4);
        return vertexBuffer;
    }
}
