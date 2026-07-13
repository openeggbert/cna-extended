// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/VectorDraw/PrimitiveBatch.hpp"

#include "Microsoft/Xna/Framework/Graphics/Effect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPassCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerStateCollection.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <stdexcept>

namespace CNA::Extended::VectorDraw
{
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::SamplerState;

    PrimitiveBatch::PrimitiveBatch(GraphicsDevice& graphicsDevice, int bufferSize)
        : basicEffect_(graphicsDevice),
          device_(&graphicsDevice),
          lineVertices_(static_cast<std::size_t>(bufferSize - bufferSize % 2)),
          triangleVertices_(static_cast<std::size_t>(bufferSize - bufferSize % 3))
    {
        basicEffect_.VertexColorEnabled = true;
    }

    void PrimitiveBatch::Dispose()
    {
        Dispose(true);
    }

    void PrimitiveBatch::Dispose(bool disposing)
    {
        if (disposing && !isDisposed_)
        {
            basicEffect_.Dispose();
            isDisposed_ = true;
        }
    }

    void PrimitiveBatch::SetProjection(const Matrix& projection)
    {
        basicEffect_.Projection = projection;
    }

    void PrimitiveBatch::Begin(const Matrix& projection, const Matrix& view, const BlendState* blendState)
    {
        if (hasBegun_)
        {
            throw std::logic_error("End must be called before Begin can be called again.");
        }

        previousBlendState_ = device_->getBlendStateProperty();
        device_->setBlendStateProperty(blendState != nullptr ? *blendState : BlendState::NonPremultiplied);

        basicEffect_.Projection = projection;
        basicEffect_.View = view;
        basicEffect_.getCurrentTechniqueProperty()->getPassesProperty()[0].Apply();
        hasBegun_ = true;
    }

    void PrimitiveBatch::AddVertex(const Vector2& vertex, const Color& color, PrimitiveType primitiveType)
    {
        if (!hasBegun_)
        {
            throw std::logic_error("Begin must be called before AddVertex can be called.");
        }

        if (primitiveType == PrimitiveType::LineStrip || primitiveType == PrimitiveType::TriangleStrip)
        {
            throw std::invalid_argument("The specified primitiveType is not supported by PrimitiveBatch.");
        }

        if (primitiveType == PrimitiveType::TriangleList)
        {
            if (triangleVertsCount_ >= static_cast<int>(triangleVertices_.size()))
            {
                FlushTriangles();
            }

            triangleVertices_[static_cast<std::size_t>(triangleVertsCount_)].Position = Vector3(vertex, -0.1f);
            triangleVertices_[static_cast<std::size_t>(triangleVertsCount_)].Color = color;
            triangleVertsCount_++;
        }

        if (primitiveType == PrimitiveType::LineList)
        {
            if (lineVertsCount_ >= static_cast<int>(lineVertices_.size()))
            {
                FlushLines();
            }

            lineVertices_[static_cast<std::size_t>(lineVertsCount_)].Position = Vector3(vertex, 0.0f);
            lineVertices_[static_cast<std::size_t>(lineVertsCount_)].Color = color;
            lineVertsCount_++;
        }
    }

    void PrimitiveBatch::End()
    {
        if (!hasBegun_)
        {
            throw std::logic_error("Begin must be called before End can be called.");
        }

        FlushTriangles();
        FlushLines();
        device_->setBlendStateProperty(previousBlendState_);
        hasBegun_ = false;
    }

    void PrimitiveBatch::FlushTriangles()
    {
        if (!hasBegun_)
        {
            throw std::logic_error("Begin must be called before Flush can be called.");
        }

        if (triangleVertsCount_ >= 3)
        {
            const int primitiveCount = triangleVertsCount_ / 3;
            device_->getSamplerStatesProperty()[0] = SamplerState::AnisotropicClamp;
            device_->DrawUserPrimitives(PrimitiveType::TriangleList, triangleVertices_.data(), 0, primitiveCount);
            triangleVertsCount_ -= primitiveCount * 3;
        }
    }

    void PrimitiveBatch::FlushLines()
    {
        if (!hasBegun_)
        {
            throw std::logic_error("Begin must be called before Flush can be called.");
        }

        if (lineVertsCount_ >= 2)
        {
            const int primitiveCount = lineVertsCount_ / 2;
            device_->getSamplerStatesProperty()[0] = SamplerState::AnisotropicClamp;
            device_->DrawUserPrimitives(PrimitiveType::LineList, lineVertices_.data(), 0, primitiveCount);
            lineVertsCount_ -= primitiveCount * 2;
        }
    }
}
