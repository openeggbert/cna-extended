// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// *** GENUINE C++ RE-AUTHORING, NOT A LITERAL PORT -- user-approved after investigation ***
// Upstream's MatrixChainEffect is constructed from a compiled XNA/MonoGame Effect bytecode blob
// (`byte[] byteCode`, ultimately MonoGame's own `.mgfxo` binary format). CNA's own
// `Effect(GraphicsDevice&, const std::vector<bytecs>&)` constructor -- the literal 1:1 target for
// this -- ALWAYS throws System::NotImplementedException: CNA has no MojoShader-equivalent bytecode
// parser/translator yet (tracked as CNA's own future "Phase 74"; see Effect.hpp's doc comment).
// Investigated and confirmed before proceeding, not assumed. Presented to the user as a design
// question with three options (skip Effects/* entirely / port only the dependency-free interfaces
// / manually re-author as a CNA ShaderEffect); the user chose re-authoring.
//
// Re-authored on top of CNA's `ShaderEffect` (GLSL-source-based, a NOXNA CNA extension), not raw
// `Effect`. `Effect`'s Parameters[name] EffectParameter caching model has no ShaderEffect
// equivalent -- ShaderEffect exposes direct `SetUniformXxx(name, ...)` calls instead, so this port
// sets the WorldViewProjection uniform directly in OnApply() rather than caching an EffectParameter
// reference the way upstream's `_matrixParameter` field does; behaviorally identical, just without
// the caching layer ShaderEffect doesn't provide.
//
// Verified against CNA's own working GLSL-ShaderEffect-through-SpriteBatch example
// (examples/easygl_shader_effect_test.cpp) before choosing a uniform name: that example's shader
// uses `uniform mat4 projection`, which SpriteBatch itself sets automatically on every compiled
// program it uses -- to avoid SpriteBatch silently overwriting a name this effect also wants to
// control (upstream's own World/View/Projection chain, not SpriteBatch's simpler auto-projection),
// this port's shader instead uses `uniform mat4 WorldViewProjection`, a name SpriteBatch never
// touches, computed and set here exactly the way upstream's OnApply() does (World * View *
// (custom-or-default-orthographic) Projection).
//
// `Matrix::ToColumnMajor(float[16])` (a NOXNA CNA-provided helper, confirmed via CNA's own
// EasyGL backend usage at BindDrawParams) converts XNA/FNA's row-major Matrix field layout to the
// column-major layout GLSL's `mat4` uniforms expect -- verified via CNA's own backend source, not
// guessed.
#pragma once

#include "CNA/Extended/Graphics/IMatrixChainEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"

namespace CNA::Extended::Graphics::Effects
{
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::ShaderEffect;

    /** @brief An Effect that uses the standard World/View/Projection matrix chain to represent a 3D object on a 2D monitor. */
    class MatrixChainEffect : public ShaderEffect, public CNA::Extended::Graphics::IMatrixChainEffect
    {
    public:
        [[nodiscard]] Matrix getWorldProperty() const override { return world_; }
        void setWorldProperty(const Matrix& value) override { SetWorld(value); }
        void SetWorld(const Matrix& world) override;

        [[nodiscard]] Matrix getViewProperty() const override { return view_; }
        void setViewProperty(const Matrix& value) override { SetView(value); }
        void SetView(const Matrix& view) override;

        [[nodiscard]] Matrix getProjectionProperty() const override { return projection_; }
        void setProjectionProperty(const Matrix& value) override { SetProjection(value); }
        void SetProjection(const Matrix& projection) override;

    protected:
        /**
         * @brief Initializes a new MatrixChainEffect from GLSL vertex/fragment shader source.
         * @param device The graphics device that will own this effect.
         * @param vertSrc GLSL vertex shader source.
         * @param fragSrc GLSL fragment shader source.
         */
        MatrixChainEffect(GraphicsDevice& device, const std::string& vertSrc, const std::string& fragSrc);

        /** @brief Computes and sets the WorldViewProjection uniform if any of World/View/Projection have changed since the last apply. */
        void OnApply() override;

    private:
        Matrix world_ = Matrix::getIdentityProperty();
        Matrix view_ = Matrix::getIdentityProperty();
        Matrix projection_ = Matrix::getIdentityProperty();
        bool useDefaultProjection_ = true;
        bool dirtyWorldViewProjection_ = true;
    };
}
