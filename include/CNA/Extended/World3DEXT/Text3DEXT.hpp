// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Text3DEXT -- new, non-upstream addition. See 3d.md/plan3d.md
// at the repository root for the design.
//
// 3D counterpart of BitmapFontExtensions::DrawString: floating, world-space text labels,
// rendered as a single camera-facing billboard mesh instead of a 2D SpriteBatch.DrawString
// call. Reuses this project's already-ported BitmapFonts::BitmapFont glyph layout
// (BitmapFont::GetGlyphs -- real kerning/line-layout logic, not re-implemented here) --
// BuildText3DMeshEXT only converts BitmapFont's already-laid-out 2D pixel-space glyph
// quads into a combined 3D local-space mesh (Y flipped: BitmapFont's layout is Y-down
// screen space, this mesh is Y-up so upright text reads correctly once billboarded).
//
// Known simplification: assumes every glyph in the string comes from the same font page
// texture (true for any single-page BMFont, the overwhelmingly common case); a glyph on a
// different page than the first is skipped rather than starting a second draw batch --
// matches this phase's "start with the simplest correct version" precedent (see
// SpatialHash3DEXT.hpp/CollisionWorld3DEXT.hpp for the same principle applied elsewhere in
// this plan).
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"

#include <memory>
#include <string>

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
    /**
     * @brief ECS component for a floating, world-space text label, drawn by
     * TextBillboardRenderSystemEXT as a single camera-facing billboard mesh.
     * @see BuildText3DMeshEXT, the helper that builds VertexBufferEXT/IndexBufferEXT.
     * @see TextBillboardRenderSystemEXT, which draws this component.
     */
    struct Text3DEXT
    {
        /** @brief The combined multi-glyph quad mesh built by BuildText3DMeshEXT. Not owned by this component. */
        Microsoft::Xna::Framework::Graphics::VertexBuffer* VertexBufferEXT = nullptr;

        /** @brief The index buffer built by BuildText3DMeshEXT alongside VertexBufferEXT. Not owned by this component. */
        Microsoft::Xna::Framework::Graphics::IndexBuffer* IndexBufferEXT = nullptr;

        /** @brief Number of triangles to draw (2 per glyph). */
        int PrimitiveCountEXT = 0;

        /** @brief The font's page texture BuildText3DMeshEXT's UVs were computed against. Not owned by this component. */
        Microsoft::Xna::Framework::Graphics::Texture2D* TextureEXT = nullptr;

        /** @brief The world-space size of the label (scales the local, pixel-sized mesh). */
        float ScaleEXT = 1.0f;

        /** @brief Tint multiplied with the sampled texture color. */
        Microsoft::Xna::Framework::Color TintEXT = Microsoft::Xna::Framework::Color::White;
    };

    /** @brief The result of BuildText3DMeshEXT: newly-allocated GPU buffers plus draw metadata. */
    struct Text3DMeshEXT
    {
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer> VertexBuffer;
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::IndexBuffer> IndexBuffer;
        int PrimitiveCount = 0;
        Microsoft::Xna::Framework::Graphics::Texture2D* Texture = nullptr;
    };

    /**
     * @brief Lays out @p text with @p font (BitmapFont::GetGlyphs, real kerning/line-layout
     * logic) and builds one combined 3D quad mesh, local-space origin at the string's
     * top-left, X right, Y up, Z 0 -- one glyph quad per character, in font-pixel-sized
     * local units (scale via Text3DEXT::ScaleEXT at draw time, not baked in here).
     * @param graphicsDevice The device the mesh's VertexBuffer/IndexBuffer are uploaded to.
     * @param font The font supplying glyph layout and texture regions.
     * @param text The string to lay out. An empty string yields a mesh with PrimitiveCount == 0.
     */
    [[nodiscard]] Text3DMeshEXT BuildText3DMeshEXT(Microsoft::Xna::Framework::Graphics::GraphicsDevice& graphicsDevice,
                                                    const BitmapFonts::BitmapFont& font, const std::string& text);
}
