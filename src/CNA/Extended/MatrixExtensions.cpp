// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/MatrixExtensions.hpp"

#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <cmath>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Quaternion;
    using Microsoft::Xna::Framework::Vector3;

    bool Decompose(const Matrix& matrix, Vector2& position, float& rotation, Vector2& scale)
    {
        Vector3 position3 = Vector3::Zero;
        Vector3 scale3 = Vector3::One;
        Quaternion rotationQuaternion = Quaternion::Identity;

        if (matrix.Decompose(scale3, rotationQuaternion, position3))
        {
            const Vector2 direction = Vector2::Transform(Vector2::UnitX, rotationQuaternion);
            rotation = std::atan2(direction.Y, direction.X);
            position = Vector2(position3.X, position3.Y);
            scale = Vector2(scale3.X, scale3.Y);
            return true;
        }

        position = Vector2::Zero;
        rotation = 0.0f;
        scale = Vector2::One;
        return false;
    }
}
