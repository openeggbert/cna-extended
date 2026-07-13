// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's VectorDraw/PrimitiveBatch.cs (itself adapted upstream from
// Velcro/Farseer Physics, used with permission -- see upstream's own file header). A low-level
// primitive batch rendering colored lines and filled triangles directly via BasicEffect with
// vertex colors. This is the underlying renderer used by PrimitiveDrawing.
//
// Confirmed CNA already exposes every graphics primitive this file needs, with matching shapes --
// no re-authoring needed, unlike plan.md's flagged concern for Graphics/Effects/* in a later
// phase: Microsoft::Xna::Framework::Graphics::BasicEffect (constructible from a GraphicsDevice&),
// GraphicsDevice::DrawUserPrimitives(PrimitiveType, const VertexPositionColor*, int, int) (an
// exact-shape overload), GraphicsDevice::getBlendStateProperty()/setBlendStateProperty(),
// GraphicsDevice::getSamplerStatesProperty()[index], BlendState::NonPremultiplied,
// SamplerState::AnisotropicClamp, and Effect::getCurrentTechniqueProperty()->
// getPassesProperty()[0].Apply(). Verified by reading each header directly, not assumed.
//
// `new BasicEffect(graphicsDevice)` (C# heap allocation, released via `_basicEffect.Dispose()`)
// -> a plain by-value BasicEffect member here, since nothing else ever shares this specific
// instance -- C++ value semantics make the heap indirection unnecessary. `IDisposable` ->
// System::IDisposable, matching this project's/CNA's own established Dispose()/Dispose(bool)
// pattern (see GraphicsResource.hpp, which CNA's own Effect/BasicEffect already implement this
// exact way). `VertexPositionColor[]` fixed-size scratch buffers -> std::vector<VertexPositionColor>
// sized once in the constructor, matching upstream's array-allocated-once-then-reused approach
// (no per-frame allocation).
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/IDisposable.hpp"

#include <vector>

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
}

namespace CNA::Extended::VectorDraw
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::BasicEffect;
    using Microsoft::Xna::Framework::Graphics::BlendState;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;
    using Microsoft::Xna::Framework::Graphics::VertexPositionColor;

    /**
     * @brief A low-level primitive batch that renders colored lines and filled triangles directly
     * to the GPU using BasicEffect with vertex colors.
     * @remark Call Begin() before adding vertices and End() to flush all pending geometry to the
     * GPU. This is the underlying renderer used by PrimitiveDrawing.
     */
    class PrimitiveBatch : public System::IDisposable
    {
    public:
        static constexpr int DefaultBufferSize = 500;

        /**
         * @brief Initializes a new PrimitiveBatch.
         * @param graphicsDevice The graphics device used to issue draw calls.
         * @param bufferSize The maximum number of vertices buffered before an automatic flush occurs.
         */
        explicit PrimitiveBatch(GraphicsDevice& graphicsDevice, int bufferSize = DefaultBufferSize);

        void Dispose() override;

        /** @brief Sets the projection matrix applied during rendering. */
        void SetProjection(const Matrix& projection);

        /** @brief Returns true if Begin() has been called and End() has not yet been called. */
        [[nodiscard]] bool IsReady() const { return hasBegun_; }

        /**
         * @brief Begins a new batch, setting the view and projection matrices for this frame.
         * Must be called before any AddVertex() calls.
         * @param projection The projection matrix.
         * @param view The view matrix.
         * @param blendState The BlendState to use while drawing, or nullptr to use
         * BlendState::NonPremultiplied.
         * @throws std::logic_error End() must be called before Begin() can be called again.
         */
        void Begin(const Matrix& projection, const Matrix& view, const BlendState* blendState = nullptr);

        /**
         * @brief Adds a single vertex to the current batch.
         * @param vertex The 2D position of the vertex.
         * @param color The color of the vertex.
         * @param primitiveType The primitive type this vertex belongs to. Only
         * PrimitiveType::TriangleList and PrimitiveType::LineList are supported.
         * @throws std::logic_error Begin() must be called before adding vertices.
         * @throws std::invalid_argument primitiveType is LineStrip or TriangleStrip.
         */
        void AddVertex(const Vector2& vertex, const Color& color, PrimitiveType primitiveType);

        /**
         * @brief Flushes all pending geometry to the GPU and ends the current batch.
         * @throws std::logic_error Begin() must be called before End().
         */
        void End();

    protected:
        /**
         * @brief Releases resources used by this PrimitiveBatch.
         * @param disposing true to release managed resources; false to release only unmanaged resources.
         */
        virtual void Dispose(bool disposing);

    private:
        void FlushTriangles();
        void FlushLines();

        BasicEffect basicEffect_;
        GraphicsDevice* device_;
        std::vector<VertexPositionColor> lineVertices_;
        std::vector<VertexPositionColor> triangleVertices_;
        BlendState previousBlendState_;
        bool hasBegun_ = false;
        bool isDisposed_ = false;
        int lineVertsCount_ = 0;
        int triangleVertsCount_ = 0;
    };
}
