// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/TilemapRendererShared.cs's file-scoped
// `internal sealed class LayerModel : IDisposable`. Split into its own file to match this
// project's one-type-per-file convention (see CLAUDE.md), unlike upstream's single-file layout.
//
// `IDisposable`/manual `Dispose()` -> plain C++ RAII: CNA's `VertexBuffer`/`IndexBuffer` are
// themselves move-only RAII types whose destructors already release their GPU resources (see
// their own header comments), so there is nothing left for `LayerModel` to manually dispose --
// unlike upstream, where `VertexBuffer`/`IndexBuffer` are GC-managed XNA `IDisposable` resources
// that must be disposed explicitly. `LayerModel` is therefore move-only (implicitly, by owning
// two move-only members) with no declared destructor/Dispose method at all.
//
// `Texture` is a non-owning `Texture2D*`, matching this project's established convention for
// GPU texture references borrowed from an externally-owned, longer-lived source (e.g.
// `TilemapTileset::GetRenderSource`, `TilemapImageLayer::getTextureProperty`) -- upstream's
// `Texture2D Texture` field is likewise just a borrowed GC reference, never an owner.
#pragma once

#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::IndexBuffer;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::VertexBuffer;

    /** @brief A GPU-ready vertex/index buffer pair for one batched draw call, plus the texture and parallax factor it draws with. */
    class LayerModel
    {
    public:
        LayerModel(VertexBuffer vertexBuffer, IndexBuffer indexBuffer, Texture2D* texture, int primitiveCount)
            : vertexBuffer_(std::move(vertexBuffer)), indexBuffer_(std::move(indexBuffer)), texture_(texture), primitiveCount_(primitiveCount)
        {
        }

        [[nodiscard]] VertexBuffer& getVertexBufferProperty() { return vertexBuffer_; }
        [[nodiscard]] const VertexBuffer& getVertexBufferProperty() const { return vertexBuffer_; }

        [[nodiscard]] IndexBuffer& getIndexBufferProperty() { return indexBuffer_; }
        [[nodiscard]] const IndexBuffer& getIndexBufferProperty() const { return indexBuffer_; }

        [[nodiscard]] Texture2D* getTextureProperty() const { return texture_; }

        [[nodiscard]] int getPrimitiveCountProperty() const { return primitiveCount_; }

        [[nodiscard]] const Vector2& getParallaxFactorProperty() const { return parallaxFactor_; }
        void setParallaxFactorProperty(const Vector2& value) { parallaxFactor_ = value; }

    private:
        VertexBuffer vertexBuffer_;
        IndexBuffer indexBuffer_;
        Texture2D* texture_;
        int primitiveCount_;
        Vector2 parallaxFactor_ = Vector2::One;
    };
}
