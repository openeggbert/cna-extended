// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/IMatrixChainEffect.cs: an Effect that uses the standard
// World/View/Projection matrix chain to represent a 3D object on a 2D monitor. Extends CNA's own
// Microsoft::Xna::Framework::Graphics::IEffectMatrices (already provides the World/View/Projection
// getXProperty()/setXProperty() pairs BasicEffect itself implements) with the explicit
// SetWorld/SetView/SetProjection methods upstream additionally exposes -- upstream's `ref Matrix`
// out-of-band setter API alongside the plain property setter, kept as a distinct method pair here
// (not collapsed into the property setters) for 1:1 fidelity, even though both ultimately do the
// same thing in this port.
#pragma once

#include "Microsoft/Xna/Framework/Graphics/IEffectMatrices.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::Matrix;

    /** @brief An Effect that uses the standard World/View/Projection matrix chain to represent a 3D object on a 2D monitor. */
    class IMatrixChainEffect : public Microsoft::Xna::Framework::Graphics::IEffectMatrices
    {
    public:
        /** @brief Sets the model-to-world matrix. */
        virtual void SetWorld(const Matrix& world) = 0;

        /** @brief Sets the world-to-view matrix. */
        virtual void SetView(const Matrix& view) = 0;

        /** @brief Sets the view-to-projection matrix. */
        virtual void SetProjection(const Matrix& projection) = 0;
    };
}
