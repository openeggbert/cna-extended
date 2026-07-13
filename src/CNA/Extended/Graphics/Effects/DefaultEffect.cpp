// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// GLSL shader source below re-authored from DefaultEffect.fx's 4-technique HLSL logic (see
// DefaultEffect.hpp's header comment for the technique-collapsing rationale): each upstream pixel
// shader variant computes `DiffuseColor`, optionally multiplied by vertex color and/or a sampled
// texture -- this single fragment shader reproduces all 4 combinations via two uniform bools.
#include "CNA/Extended/Graphics/Effects/DefaultEffect.hpp"

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CNA::Extended::Graphics::Effects
{
    namespace
    {
        const char* const kVertexSource = R"(#version 300 es
precision highp float;

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

out vec2 TexCoord;
out vec4 VertexColor;

uniform mat4 WorldViewProjection;

void main()
{
    gl_Position = WorldViewProjection * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
    VertexColor = aColor;
}
)";

        const char* const kFragmentSource = R"(#version 300 es
precision mediump float;

in vec2 TexCoord;
in vec4 VertexColor;

out vec4 FragColor;

uniform vec4 DiffuseColor;
uniform sampler2D Texture;
uniform bool TextureEnabled;
uniform bool VertexColorEnabled;

void main()
{
    vec4 color = DiffuseColor;

    if (VertexColorEnabled)
    {
        color *= VertexColor;
    }

    if (TextureEnabled)
    {
        color *= texture(Texture, TexCoord);
    }

    FragColor = color;
}
)";
    }

    DefaultEffect::DefaultEffect(GraphicsDevice& graphicsDevice) : MatrixChainEffect(graphicsDevice, kVertexSource, kFragmentSource)
    {
    }

    void DefaultEffect::setTextureProperty(Texture2D* value)
    {
        texture_ = value;
        dirtyTexture_ = true;
    }

    void DefaultEffect::setAlphaProperty(float value)
    {
        alpha_ = value;
        dirtyMaterialColor_ = true;
    }

    void DefaultEffect::setTextureEnabledProperty(bool value)
    {
        if (textureEnabled_ == value)
        {
            return;
        }
        textureEnabled_ = value;
        dirtyToggles_ = true;
    }

    void DefaultEffect::setVertexColorEnabledProperty(bool value)
    {
        if (vertexColorEnabled_ == value)
        {
            return;
        }
        vertexColorEnabled_ = value;
        dirtyToggles_ = true;
    }

    void DefaultEffect::OnApply()
    {
        MatrixChainEffect::OnApply();

        if (dirtyTexture_)
        {
            if (texture_ != nullptr)
            {
                SetTexture(0, *texture_);
            }
            dirtyTexture_ = false;
        }

        if (dirtyToggles_)
        {
            SetUniformInt("TextureEnabled", textureEnabled_ ? 1 : 0);
            SetUniformInt("VertexColorEnabled", vertexColorEnabled_ ? 1 : 0);
            dirtyToggles_ = false;
        }

        if (dirtyMaterialColor_)
        {
            UpdateMaterialColor();
            dirtyMaterialColor_ = false;
        }
    }

    void DefaultEffect::UpdateMaterialColor()
    {
        const Microsoft::Xna::Framework::Vector3 diffuseColorVector3 = diffuseColor_.ToVector3();
        SetUniformVec4("DiffuseColor", diffuseColorVector3.X * alpha_, diffuseColorVector3.Y * alpha_, diffuseColorVector3.Z * alpha_, alpha_);
    }

    const std::string& DefaultEffect::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.Graphics.Effects.DefaultEffect";
        return typeName;
    }
}
