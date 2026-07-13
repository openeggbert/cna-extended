// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/MatrixExtensions.cs. C# extension methods have no C++
// equivalent syntax; ported as a free function in this namespace, matching the convention used
// throughout this project for extension methods (e.g. ColorExtensions::ToHex).
#pragma once

#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector2;

    /**
     * @brief Decomposes a 3D Matrix into a 2D position, rotation, and scale, by decomposing the
     * matrix's 3D scale/rotation/translation and projecting the rotation onto the XY plane.
     * @param matrix The matrix to decompose.
     * @param position Receives the 2D position.
     * @param rotation Receives the rotation angle, in radians.
     * @param scale Receives the 2D scale.
     * @return true if the matrix was successfully decomposed; otherwise, false.
     */
    [[nodiscard]] bool Decompose(const Matrix& matrix, Vector2& position, float& rotation, Vector2& scale);
}
