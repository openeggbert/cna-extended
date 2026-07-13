// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/Effects/MatrixChainEffect.hpp"

#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

namespace CNA::Extended::Graphics::Effects
{
    using Microsoft::Xna::Framework::Graphics::Viewport;

    MatrixChainEffect::MatrixChainEffect(GraphicsDevice& device, const std::string& vertSrc, const std::string& fragSrc)
        : ShaderEffect(device, vertSrc, fragSrc)
    {
    }

    void MatrixChainEffect::SetWorld(const Matrix& world)
    {
        world_ = world;
        dirtyWorldViewProjection_ = true;
    }

    void MatrixChainEffect::SetView(const Matrix& view)
    {
        view_ = view;
        dirtyWorldViewProjection_ = true;
    }

    void MatrixChainEffect::SetProjection(const Matrix& projection)
    {
        projection_ = projection;
        dirtyWorldViewProjection_ = true;
        useDefaultProjection_ = false;
    }

    void MatrixChainEffect::OnApply()
    {
        ShaderEffect::OnApply();

        if (dirtyWorldViewProjection_ || useDefaultProjection_)
        {
            if (useDefaultProjection_)
            {
                const Viewport& viewport = getGraphicsDeviceProperty()->getViewportProperty();
                projection_ = Matrix::CreateOrthographicOffCenter(
                    0.0f, static_cast<float>(viewport.getWidthProperty()), static_cast<float>(viewport.getHeightProperty()), 0.0f, 0.0f, -1.0f);
            }

            const Matrix worldViewProjection = Matrix::Multiply(Matrix::Multiply(world_, view_), projection_);
            float worldViewProjectionColumnMajor[16];
            worldViewProjection.ToColumnMajor(worldViewProjectionColumnMajor);
            SetUniformMat4("WorldViewProjection", worldViewProjectionColumnMajor);

            dirtyWorldViewProjection_ = false;
        }
    }
}
