// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/TilemapRenderer3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/CubeMeshRenderSystemEXT.hpp"
#include "CNA/Extended/World3DEXT/Tilemap3DEXT.hpp"
#include "CNA/Extended/World3DEXT/TilemapTileset3DEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingFrustum;
    using Microsoft::Xna::Framework::BoundingSphere;
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    TilemapRenderer3DEXT::TilemapRenderer3DEXT(CubeMeshRenderSystemEXT& cubeRenderSystem) : cubeRenderSystemEXT_(&cubeRenderSystem)
    {
    }

    void TilemapRenderer3DEXT::Draw(Camera3DEXT& camera, const Tilemap3DEXT& tilemap, const TilemapTileset3DEXT& tileset)
    {
        const BoundingFrustum frustum = camera.GetBoundingFrustumEXT();
        const Vector3& tileSize = tilemap.getTileSizeProperty();
        const float boundingRadius = tileSize.Length() * 0.5f;

        for (const auto& [coordinate, tileId] : tilemap.getTilesProperty())
        {
            const Vector3 worldPosition = tilemap.TileToWorldPositionEXT(coordinate.X, coordinate.Y, coordinate.Z);
            const BoundingSphere worldBounds(worldPosition, boundingRadius);
            if (!frustum.Intersects(worldBounds))
            {
                continue;
            }

            Texture2D* texture = tileset.GetTileTextureEXT(tileId);
            const Matrix world = Matrix::CreateScale(tileSize) * Matrix::CreateTranslation(worldPosition);
            cubeRenderSystemEXT_->DrawCubeEXT(texture, world, Color::White);
        }
    }
}
