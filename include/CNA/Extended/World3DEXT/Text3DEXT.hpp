// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Text3DEXT -- new, non-upstream addition. See 3d.md/plan3d.md
// at the repository root for the design.
//
// 3D counterpart of BitmapFontExtensions::DrawString: floating, world-space text labels,
// rendered as a small number of camera-facing billboard meshes (one per distinct font page
// texture the string's glyphs use) instead of a 2D SpriteBatch.DrawString call. Reuses this
// project's already-ported BitmapFonts::BitmapFont glyph layout (BitmapFont::GetGlyphs --
// real kerning/line-layout logic, not re-implemented here) -- BuildText3DMeshEXT only
// converts BitmapFont's already-laid-out 2D pixel-space glyph quads into 3D local-space
// mesh parts (Y flipped: BitmapFont's layout is Y-down screen space, this mesh is Y-up so
// upright text reads correctly once billboarded).
//
// Multi-page support (Phase 12 D, 2026-07-15, user-requested "extend later" item): originally
// this assumed every glyph in a string came from the same font page texture (true for any
// single-page BMFont, the overwhelmingly common case), silently skipping glyphs from a
// second page rather than starting a second draw batch. Text3DEXT/Text3DMeshEXT now hold a
// vector of per-page parts instead of one flat mesh -- BuildText3DMeshEXT groups glyphs by
// their page texture (preserving first-seen page order) into one Text3DMeshPartEXT per
// distinct texture, and TextBillboardRenderSystemEXT issues one draw call per part (texture
// swap between parts, same per-part draw-call shape the single-mesh version always had --
// this is "repeat the existing single-texture draw once per page", not a new drawing
// strategy). The 2D BitmapFontExtensions::DrawString equivalent has no comparable multi-page
// grouping logic to mirror -- it draws one glyph at a time and lets SpriteBatch's own
// per-texture batching handle page grouping implicitly, which doesn't apply to this combined
// single/multi-VertexBuffer 3D mesh design.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::BitmapFonts
{
    class BitmapFont;
}

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
    class Texture2D;
    class VertexBuffer;
    class IndexBuffer;
}

namespace CNA::Extended::World3DEXT
{
    /** @brief One font page's worth of a Text3DEXT label's mesh: non-owning GPU buffer pointers plus that page's texture. */
    struct Text3DPartEXT
    {
        /** @brief This page's quad mesh, built by BuildText3DMeshEXT. Not owned by this component. */
        Microsoft::Xna::Framework::Graphics::VertexBuffer* VertexBufferEXT = nullptr;

        /** @brief The index buffer built by BuildText3DMeshEXT alongside VertexBufferEXT. Not owned by this component. */
        Microsoft::Xna::Framework::Graphics::IndexBuffer* IndexBufferEXT = nullptr;

        /** @brief Number of triangles to draw (2 per glyph on this page). */
        int PrimitiveCountEXT = 0;

        /** @brief This part's font page texture, BuildText3DMeshEXT's UVs were computed against. Not owned by this component. */
        Microsoft::Xna::Framework::Graphics::Texture2D* TextureEXT = nullptr;
    };

    /**
     * @brief ECS component for a floating, world-space text label, drawn by
     * TextBillboardRenderSystemEXT as one camera-facing billboard mesh per font page.
     * @see BuildText3DMeshEXT, the helper that builds PartsEXT's GPU buffers.
     * @see TextBillboardRenderSystemEXT, which draws this component.
     */
    struct Text3DEXT
    {
        /** @brief One draw batch per distinct font page texture this label's text uses. */
        std::vector<Text3DPartEXT> PartsEXT;

        /** @brief The world-space size of the label (scales the local, pixel-sized mesh). */
        float ScaleEXT = 1.0f;

        /** @brief Tint multiplied with the sampled texture color. */
        Microsoft::Xna::Framework::Color TintEXT = Microsoft::Xna::Framework::Color::White;
    };

    /** @brief One font page's worth of BuildText3DMeshEXT's result: newly-allocated GPU buffers plus draw metadata. */
    struct Text3DMeshPartEXT
    {
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer> VertexBuffer;
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::IndexBuffer> IndexBuffer;
        int PrimitiveCount = 0;
        Microsoft::Xna::Framework::Graphics::Texture2D* Texture = nullptr;
    };

    /** @brief The result of BuildText3DMeshEXT: one Text3DMeshPartEXT per distinct font page texture the string's glyphs use. */
    struct Text3DMeshEXT
    {
        std::vector<Text3DMeshPartEXT> Parts;
    };

    /**
     * @brief Lays out @p text with @p font (BitmapFont::GetGlyphs, real kerning/line-layout
     * logic) and builds one 3D quad mesh part per distinct font page texture used, local-space
     * origin at the string's top-left, X right, Y up, Z 0 -- one glyph quad per character, in
     * font-pixel-sized local units (scale via Text3DEXT::ScaleEXT at draw time, not baked in
     * here).
     * @param graphicsDevice The device each part's VertexBuffer/IndexBuffer are uploaded to.
     * @param font The font supplying glyph layout and texture regions.
     * @param text The string to lay out. An empty string yields a mesh with no parts.
     */
    [[nodiscard]] Text3DMeshEXT BuildText3DMeshEXT(Microsoft::Xna::Framework::Graphics::GraphicsDevice& graphicsDevice,
                                                    const BitmapFonts::BitmapFont& font, const std::string& text);
}
