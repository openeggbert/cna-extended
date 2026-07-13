// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/PrimitiveTypeExtensions.cs. C# extension methods have
// no C++ equivalent syntax; ported as free functions in this namespace, matching this project's
// established convention (see Vector2Extensions.hpp). Upstream's `internal static` visibility has
// no C++ equivalent; kept public, matching the precedent set by CollisionShapeKind2D.hpp/
// ActorPairKey.hpp elsewhere in this project.
#pragma once

#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"

#include <stdexcept>

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;

    /** @brief Computes the number of primitives represented by the given vertex count, for the specified primitive type. */
    [[nodiscard]] inline int GetPrimitivesCount(PrimitiveType primitiveType, int verticesCount)
    {
        switch (primitiveType)
        {
            case PrimitiveType::LineStrip:
                return verticesCount - 1;
            case PrimitiveType::LineList:
                return verticesCount / 2;
            case PrimitiveType::TriangleStrip:
                return verticesCount - 2;
            case PrimitiveType::TriangleList:
                return verticesCount / 3;
            default:
                throw std::invalid_argument("Invalid primitive type.");
        }
    }

    /** @brief Computes the number of vertices required to represent the given primitive count, for the specified primitive type. */
    [[nodiscard]] inline int GetVerticesCount(PrimitiveType primitiveType, int primitivesCount)
    {
        switch (primitiveType)
        {
            case PrimitiveType::LineStrip:
                return primitivesCount + 1;
            case PrimitiveType::LineList:
                return primitivesCount * 2;
            case PrimitiveType::TriangleStrip:
                return primitivesCount + 2;
            case PrimitiveType::TriangleList:
                return primitivesCount * 3;
            default:
                throw std::invalid_argument("Invalid primitive type.");
        }
    }
}
