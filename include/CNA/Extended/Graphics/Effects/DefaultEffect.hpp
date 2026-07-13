// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// *** GENUINE C++ RE-AUTHORING, NOT A LITERAL PORT -- see MatrixChainEffect.hpp for why ***
// Ported from MonoGame.Extended's Graphics/Effects/DefaultEffect.cs, but the underlying mechanism
// is entirely re-authored: upstream constructs itself from a precompiled `.mgfxo` bytecode blob
// (via EffectResource, itself NOT ported -- see NEXT.md for why) containing 4 separate compiled
// techniques (Position / PositionTexture / PositionColor / PositionColorTexture), switched via
// `CurrentTechnique = Techniques[shaderIndex]`. CNA's ShaderEffect (this port's base, via
// MatrixChainEffect) compiles exactly ONE vertex+fragment program per instance -- it has no
// technique/multi-program concept at all. Ported as a SINGLE GLSL program (see DefaultEffect.cpp
// for the exact source) that always declares vertex-color and texture-coordinate inputs, toggling
// their CONTRIBUTION to the final fragment color via two runtime uniform bools (`TextureEnabled`,
// `VertexColorEnabled`) instead of upstream's compile-time technique selection -- the same visual
// output (DiffuseColor, optionally multiplied by vertex color and/or a sampled texture) reached by
// a different, GLSL-idiomatic mechanism. `UpdateCurrentTechnique()`/the 4-technique dispatch is
// therefore not ported at all; `TextureEnabled`/`VertexColorEnabled` setters just mark the
// relevant uniforms dirty instead.
//
// Vertex attribute layout matches CNA's own confirmed-working SpriteBatch + ShaderEffect
// integration (examples/easygl_shader_effect_test.cpp in the CNA repo): `vec2 aPos` (location 0,
// 2D pixel-space position, NOT the 3D VertexPositionColorTexture.Position CNA's DrawUserPrimitives
// path uses), `vec2 aTexCoord` (location 1), `vec4 aColor` (location 2). This effect is therefore
// verified compatible with SpriteBatch specifically; upstream's only actual consumer of
// DefaultEffect (TilemapRenderer/TilemapWorldRenderer, both Phase 7 -- checked via grep before
// assuming, not guessed) renders 2D tile quads, which is expected to go through SpriteBatch too,
// but this has not yet been confirmed against the real Phase 7 renderer code (not yet ported).
// Revisit this assumption when Phase 7 actually wires DefaultEffect into TilemapRenderer.
//
// Notable upstream quirk, preserved exactly: upstream's `_diffuseColor` field is never exposed
// through any public property in DefaultEffect.cs -- only `Alpha` is publicly settable. Since
// `_diffuseColor` is hardcoded to Color.White and never reassigned, `UpdateMaterialColor()`'s
// output is always exactly (Alpha, Alpha, Alpha, Alpha) in practice. Not a bug to "fix" by adding
// a DiffuseColor setter upstream itself doesn't have -- ported as-is, so this class's public API
// surface has no DiffuseColor accessor either.
#pragma once

#include "CNA/Extended/Graphics/Effects/ITextureEffect.hpp"
#include "CNA/Extended/Graphics/Effects/MatrixChainEffect.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"

#include <string>

namespace CNA::Extended::Graphics::Effects
{
    using Microsoft::Xna::Framework::Color;

    /**
     * @brief An Effect that allows 2D sprite-style objects (position + optional vertex color +
     * optional texture) to be rendered with a uniform diffuse tint, via SpriteBatch.
     */
    class DefaultEffect : public MatrixChainEffect, public ITextureEffect
    {
    public:
        /**
         * @brief Initializes a new DefaultEffect for the specified graphics device.
         * @param graphicsDevice The graphics device that will own this effect.
         */
        explicit DefaultEffect(GraphicsDevice& graphicsDevice);

        [[nodiscard]] Texture2D* getTextureProperty() const override { return texture_; }
        void setTextureProperty(Texture2D* value) override;

        /** @brief Gets or sets the material color alpha (premultiplied into DiffuseColor). */
        [[nodiscard]] float getAlphaProperty() const { return alpha_; }
        void setAlphaProperty(float value);

        /** @brief Gets or sets whether texturing is enabled. */
        [[nodiscard]] bool getTextureEnabledProperty() const { return textureEnabled_; }
        void setTextureEnabledProperty(bool value);

        /** @brief Gets or sets whether vertex color is enabled. */
        [[nodiscard]] bool getVertexColorEnabledProperty() const { return vertexColorEnabled_; }
        void setVertexColorEnabledProperty(bool value);

        [[nodiscard]] const std::string& GetTypeName() const override;

    protected:
        /** @brief Computes derived parameter values (DiffuseColor*Alpha, Texture, TextureEnabled, VertexColorEnabled) immediately before applying. */
        void OnApply() override;

    private:
        void UpdateMaterialColor();

        Texture2D* texture_ = nullptr;
        float alpha_ = 1.0f;
        Color diffuseColor_ = Color::White;
        bool textureEnabled_ = false;
        bool vertexColorEnabled_ = false;
        bool dirtyTexture_ = false;
        bool dirtyMaterialColor_ = true;
        bool dirtyToggles_ = true;
    };
}
