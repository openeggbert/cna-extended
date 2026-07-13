// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ViewportAdapters/ViewportAdapter.hpp"

#include "CNA/Extended/Vector2Extensions.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended::ViewportAdapters
{
    using Microsoft::Xna::Framework::Vector2;

    Point ViewportAdapter::PointToScreen(int x, int y) const
    {
        const Matrix scaleMatrix = GetScaleMatrix();
        const Matrix invertedMatrix = Matrix::Invert(scaleMatrix);
        return ToPoint(Vector2::Transform(Vector2(static_cast<float>(x), static_cast<float>(y)), invertedMatrix));
    }
}
