// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/GraphicsDeviceExtensions.hpp"

#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBufferBinding.hpp"

namespace CNA::Extended::Graphics
{
    void DrawIndexedPrimitives(GraphicsDevice& graphicsDevice, PrimitiveType primitiveType, int baseVertex, int startIndex, int primitiveCount)
    {
        const int minVertexIndex = 0;
        const int numVertices = graphicsDevice.GetVertexBuffers()[0].getVertexBufferProperty()->getVertexCountProperty();

        graphicsDevice.DrawIndexedPrimitives(primitiveType, baseVertex, minVertexIndex, numVertices, startIndex, primitiveCount);
    }
}
