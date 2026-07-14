// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/TilemapRendererShared.cs (the
// `internal static class TilemapRendererShared`; its sibling `LayerModel` class lives in its own
// LayerModel.hpp, see that file's header comment). `internal` -> ordinary public free functions in
// this `CNA::Extended::Tilemaps::Rendering` namespace, matching this project's established
// internal-visibility convention (C++ has no assembly-boundary equivalent).
//
// Upstream's only two callers, `TilemapRenderer`/`TilemapWorldRenderer`, are both blocked on a
// genuine, separately-documented architectural incompatibility between `DefaultEffect`'s GLSL
// vertex attribute layout and `VertexPositionColorTexture`'s vertex declaration -- see this
// module's Rendering-phase notes in NEXT.md/plan.md for the full analysis. This file itself has
// no such dependency (it never touches `Effect`/`DefaultEffect` at all -- only vertex/index buffer
// construction and pure UV/quad math), so it is ported now regardless: correct, self-contained,
// independently testable, and ready for `TilemapRenderer`/`TilemapWorldRenderer` to build on
// directly once that blocker is resolved.
#pragma once

#include "CNA/Extended/Tilemaps/Rendering/LayerModel.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileFlipFlags.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColorTexture.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <array>
#include <vector>

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::BufferUsage;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::SamplerState;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::VertexPositionColorTexture;

    /**
     * @brief Builds a GPU-ready LayerModel from CPU-side vertex/index data, choosing 16-bit vs
     * 32-bit indices automatically. @p texture is stored non-owning; it must outlive the model.
     */
    [[nodiscard]] LayerModel CreateLayerModel(GraphicsDevice& graphicsDevice,
                                               const std::vector<VertexPositionColorTexture>& vertices,
                                               const std::vector<int>& indices,
                                               Texture2D* texture);

    /**
     * @brief Appends one quad's 4 vertices and 6 (CCW-wound) indices to @p vertices/@p indices,
     * for a tile at @p position sized @p width x @p height, sampling @p sourceRect from @p texture
     * with @p flipFlags applied and tinted @p color.
     */
    void AddTileQuad(std::vector<VertexPositionColorTexture>& vertices,
                      std::vector<int>& indices,
                      const Vector2& position,
                      int width,
                      int height,
                      const Rectangle& sourceRect,
                      TilemapTileFlipFlags flipFlags,
                      const Texture2D& texture,
                      const Color& color);

    /**
     * @brief Normalizes @p sourceRect to 0-1 UV range against @p texture's dimensions and applies
     * @p flipFlags (diagonal first, then horizontal, then vertical -- matching Tiled's own flip
     * order), returning per-corner UVs in top-left, top-right, bottom-left, bottom-right order.
     */
    [[nodiscard]] std::array<Vector2, 4> CalculateTextureCoordinates(const Rectangle& sourceRect,
                                                                      TilemapTileFlipFlags flipFlags,
                                                                      const Texture2D& texture);

    /**
     * @brief Maps PointClamp/LinearClamp to their Wrap equivalent for repeating image layers;
     * any other sampler state (including an already-wrapping one) is returned unchanged.
     * Compares by identity against the SamplerState presets, matching upstream's C# reference
     * equality (`samplerState == SamplerState.PointClamp`) on a class-typed parameter.
     */
    [[nodiscard]] const SamplerState* GetWrapSamplerState(const SamplerState* samplerState);
}
