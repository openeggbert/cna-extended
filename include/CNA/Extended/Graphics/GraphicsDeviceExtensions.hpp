// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/GraphicsDevice.Extensions.cs. Upstream's single member
// is entirely wrapped in `#if FNA` (a MonoGame-compatibility shim for FNA's simpler
// DrawIndexedPrimitives overload) -- ported unconditionally, matching this project's established
// "CNA mirrors FNA" precedent (see RectangleExtensions/Vector2Extensions.hpp for the same call on
// their own FNA-conditional members). C# extension methods have no C++ equivalent syntax; ported
// as a free function in this namespace. CNA's GraphicsDevice exposes only the 6-argument
// DrawIndexedPrimitives overload (baseVertex, minVertexIndex, numVertices, startIndex,
// primitiveCount) -- this 4-argument convenience overload (deriving minVertexIndex=0 and
// numVertices from the currently bound vertex buffer) is genuinely useful here, not a no-op shim.
#pragma once

#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;

    /**
     * @brief Draws geometry by indexing into the currently bound vertex buffer.
     * @param graphicsDevice The graphics device to draw with.
     * @param primitiveType The type of primitives in the index buffer.
     * @param baseVertex Offset added to each index before reading from the vertex buffer.
     * @param startIndex The index within the index buffer to start drawing from.
     * @param primitiveCount The number of primitives to render from the index buffer.
     */
    void DrawIndexedPrimitives(GraphicsDevice& graphicsDevice, PrimitiveType primitiveType, int baseVertex, int startIndex, int primitiveCount);
}
