// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/TilemapChunkRenderer3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Tilemap3DEXT.hpp"
#include "CNA/Extended/World3DEXT/TilemapTileset3DEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTexture.hpp"

#include <array>
#include <cstdint>
#include <unordered_map>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;
    using Microsoft::Xna::Framework::BoundingFrustum;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::IndexBuffer;
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::VertexBuffer;
    using Microsoft::Xna::Framework::Graphics::VertexPositionNormalTexture;

    struct TilemapChunkRenderer3DEXT::ChunkBatchEXT
    {
        std::unique_ptr<VertexBuffer> VertexBufferEXT;
        std::unique_ptr<IndexBuffer> IndexBufferEXT;
        int PrimitiveCountEXT = 0;
        Texture2D* TextureEXT = nullptr;
        BoundingBox BoundsEXT;
    };

    namespace
    {
        // Floors toward negative infinity (unlike C++'s truncating-toward-zero `/`), needed
        // since tile coordinates (and therefore chunk coordinates) can be negative.
        int FloorDivEXT(int a, int b)
        {
            const int quotient = a / b;
            const int remainder = a % b;
            return (remainder != 0 && ((remainder < 0) != (b < 0))) ? quotient - 1 : quotient;
        }

        struct ChunkKeyEXT
        {
            int ChunkX = 0;
            int ChunkY = 0;
            int ChunkZ = 0;
            Texture2D* Texture = nullptr;

            friend bool operator==(const ChunkKeyEXT& left, const ChunkKeyEXT& right)
            {
                return left.ChunkX == right.ChunkX && left.ChunkY == right.ChunkY && left.ChunkZ == right.ChunkZ
                    && left.Texture == right.Texture;
            }
        };

        struct ChunkKeyHashEXT
        {
            std::size_t operator()(const ChunkKeyEXT& key) const noexcept
            {
                std::size_t seed = std::hash<int>()(key.ChunkX);
                seed ^= std::hash<int>()(key.ChunkY) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                seed ^= std::hash<int>()(key.ChunkZ) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                seed ^= std::hash<const void*>()(key.Texture) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                return seed;
            }
        };

        struct FaceEXT
        {
            Vector3 Normal;
            int NeighborDX = 0;
            int NeighborDY = 0;
            int NeighborDZ = 0;
            std::array<Vector3, 4> Corners; // in unit-cube [-0.5, 0.5] local space
        };

        // Same 6-face corner/normal layout as CubeMeshRenderSystemEXT::BuildUnitCubeMeshEXT
        // (see that file), plus each face's neighbor-cell offset for visibility culling.
        const std::array<FaceEXT, 6>& GetUnitCubeFacesEXT()
        {
            static const std::array<FaceEXT, 6> faces = {{
                // +Z (front)
                {Vector3(0.0f, 0.0f, 1.0f),
                 0,
                 0,
                 1,
                 {{Vector3(-0.5f, -0.5f, 0.5f), Vector3(0.5f, -0.5f, 0.5f), Vector3(0.5f, 0.5f, 0.5f), Vector3(-0.5f, 0.5f, 0.5f)}}},
                // -Z (back)
                {Vector3(0.0f, 0.0f, -1.0f),
                 0,
                 0,
                 -1,
                 {{Vector3(0.5f, -0.5f, -0.5f), Vector3(-0.5f, -0.5f, -0.5f), Vector3(-0.5f, 0.5f, -0.5f), Vector3(0.5f, 0.5f, -0.5f)}}},
                // +X (right)
                {Vector3(1.0f, 0.0f, 0.0f),
                 1,
                 0,
                 0,
                 {{Vector3(0.5f, -0.5f, 0.5f), Vector3(0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f)}}},
                // -X (left)
                {Vector3(-1.0f, 0.0f, 0.0f),
                 -1,
                 0,
                 0,
                 {{Vector3(-0.5f, -0.5f, -0.5f), Vector3(-0.5f, -0.5f, 0.5f), Vector3(-0.5f, 0.5f, 0.5f), Vector3(-0.5f, 0.5f, -0.5f)}}},
                // +Y (top)
                {Vector3(0.0f, 1.0f, 0.0f),
                 0,
                 1,
                 0,
                 {{Vector3(-0.5f, 0.5f, 0.5f), Vector3(0.5f, 0.5f, 0.5f), Vector3(0.5f, 0.5f, -0.5f), Vector3(-0.5f, 0.5f, -0.5f)}}},
                // -Y (bottom)
                {Vector3(0.0f, -1.0f, 0.0f),
                 0,
                 -1,
                 0,
                 {{Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, -0.5f, -0.5f), Vector3(0.5f, -0.5f, 0.5f), Vector3(-0.5f, -0.5f, 0.5f)}}},
            }};
            return faces;
        }

        struct PendingBatchEXT
        {
            std::vector<VertexPositionNormalTexture> Vertices;
            std::vector<std::uint16_t> Indices;
        };
    }

    TilemapChunkRenderer3DEXT::TilemapChunkRenderer3DEXT(GraphicsDevice& graphicsDevice, Camera3DEXT& camera, int chunkSize)
        : graphicsDeviceEXT_(&graphicsDevice), cameraEXT_(&camera), chunkSizeEXT_(chunkSize > 0 ? chunkSize : DefaultChunkSizeEXT),
          effectEXT_(graphicsDevice)
    {
        effectEXT_.setTextureEnabledProperty(true);

        // Matches CubeMeshRenderSystemEXT's own lighting setup exactly (see that file's
        // header comment for the full reasoning) so chunk-batched tiles look identical to
        // individually-drawn ones.
        effectEXT_.setLightingEnabledProperty(true);
        effectEXT_.EnableDefaultLighting();
        effectEXT_.setAmbientLightColorProperty(Vector3(0.4f, 0.4f, 0.4f));
        effectEXT_.setSpecularColorProperty(Vector3::Zero);
    }

    TilemapChunkRenderer3DEXT::~TilemapChunkRenderer3DEXT() = default;

    void TilemapChunkRenderer3DEXT::RebuildEXT(const Tilemap3DEXT& tilemap, const TilemapTileset3DEXT& tileset)
    {
        chunksEXT_.clear();

        const Vector3& tileSize = tilemap.getTileSizeProperty();
        std::unordered_map<ChunkKeyEXT, PendingBatchEXT, ChunkKeyHashEXT> pending;

        for (const auto& [coordinate, tileId] : tilemap.getTilesProperty())
        {
            Texture2D* texture = tileset.GetTileTextureEXT(tileId);
            const Vector3 worldCenter = tilemap.TileToWorldPositionEXT(coordinate.X, coordinate.Y, coordinate.Z);

            const ChunkKeyEXT key{FloorDivEXT(coordinate.X, chunkSizeEXT_), FloorDivEXT(coordinate.Y, chunkSizeEXT_),
                                   FloorDivEXT(coordinate.Z, chunkSizeEXT_), texture};
            PendingBatchEXT& batch = pending[key];

            for (const FaceEXT& face : GetUnitCubeFacesEXT())
            {
                // Naive neighbor-based face culling: only emit a face if the neighboring
                // cell along its normal is empty (or unpopulated) -- a face between two
                // solid tiles is never visible.
                if (tilemap.HasTileEXT(coordinate.X + face.NeighborDX, coordinate.Y + face.NeighborDY, coordinate.Z + face.NeighborDZ))
                {
                    continue;
                }

                static const std::array<Vector2, 4> kUvs = {Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(1.0f, 1.0f),
                                                              Vector2(0.0f, 1.0f)};

                const auto base = static_cast<std::uint16_t>(batch.Vertices.size());
                for (int i = 0; i < 4; ++i)
                {
                    const Vector3& corner = face.Corners[static_cast<std::size_t>(i)];
                    const Vector3 position = worldCenter + Vector3(corner.X * tileSize.X, corner.Y * tileSize.Y, corner.Z * tileSize.Z);
                    batch.Vertices.emplace_back(position, face.Normal, kUvs[static_cast<std::size_t>(i)]);
                }

                batch.Indices.push_back(base + 0);
                batch.Indices.push_back(base + 1);
                batch.Indices.push_back(base + 2);
                batch.Indices.push_back(base + 0);
                batch.Indices.push_back(base + 2);
                batch.Indices.push_back(base + 3);
            }
        }

        chunksEXT_.reserve(pending.size());
        for (auto& [key, batch] : pending)
        {
            if (batch.Vertices.empty())
            {
                continue;
            }

            auto chunkBatch = std::make_unique<ChunkBatchEXT>();
            chunkBatch->TextureEXT = key.Texture;
            chunkBatch->PrimitiveCountEXT = static_cast<int>(batch.Indices.size() / 3);

            std::vector<Vector3> positions;
            positions.reserve(batch.Vertices.size());
            for (const VertexPositionNormalTexture& vertex : batch.Vertices)
            {
                positions.push_back(vertex.Position);
            }
            chunkBatch->BoundsEXT = BoundingBox::CreateFromPoints(positions);

            chunkBatch->VertexBufferEXT = std::make_unique<VertexBuffer>(*graphicsDeviceEXT_, static_cast<int>(batch.Vertices.size()));
            chunkBatch->VertexBufferEXT->SetData(batch.Vertices.data(), static_cast<int>(batch.Vertices.size()));

            chunkBatch->IndexBufferEXT = std::make_unique<IndexBuffer>(*graphicsDeviceEXT_, static_cast<int>(batch.Indices.size()));
            chunkBatch->IndexBufferEXT->SetData(batch.Indices.data(), static_cast<int>(batch.Indices.size()));

            chunksEXT_.push_back(std::move(chunkBatch));
        }
    }

    int TilemapChunkRenderer3DEXT::GetChunkBatchCountEXT() const { return static_cast<int>(chunksEXT_.size()); }

    int TilemapChunkRenderer3DEXT::GetTotalPrimitiveCountEXT() const
    {
        int total = 0;
        for (const std::unique_ptr<ChunkBatchEXT>& chunk : chunksEXT_)
        {
            total += chunk->PrimitiveCountEXT;
        }
        return total;
    }

    void TilemapChunkRenderer3DEXT::Draw()
    {
        const BoundingFrustum frustum = cameraEXT_->GetBoundingFrustumEXT();

        effectEXT_.View = cameraEXT_->GetViewMatrixEXT();
        effectEXT_.Projection = cameraEXT_->GetProjectionMatrixEXT();
        effectEXT_.World = Matrix::getIdentityProperty(); // vertices are already baked in world space

        for (const std::unique_ptr<ChunkBatchEXT>& chunk : chunksEXT_)
        {
            if (!frustum.Intersects(chunk->BoundsEXT))
            {
                continue;
            }

            effectEXT_.setTextureProperty(chunk->TextureEXT);
            effectEXT_.Apply();

            graphicsDeviceEXT_->SetVertexBuffer(chunk->VertexBufferEXT.get());
            graphicsDeviceEXT_->SetIndexBuffer(chunk->IndexBufferEXT.get());
            graphicsDeviceEXT_->DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, chunk->VertexBufferEXT->getVertexCountProperty(),
                                                       0, chunk->PrimitiveCountEXT);
        }
    }
}
