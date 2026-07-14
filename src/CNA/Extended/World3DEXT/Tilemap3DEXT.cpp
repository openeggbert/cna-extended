// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/Tilemap3DEXT.hpp"

#include "System/HashCode.hpp"

namespace std
{
    std::size_t hash<CNA::Extended::World3DEXT::TileCoordinate3DEXT>::operator()(
        const CNA::Extended::World3DEXT::TileCoordinate3DEXT& value) const noexcept
    {
        return static_cast<std::size_t>(System::HashCode::Combine(value.X, value.Y, value.Z));
    }
}

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    Tilemap3DEXT::Tilemap3DEXT(const Vector3& tileSize) : tileSize_(tileSize)
    {
    }

    int Tilemap3DEXT::GetTileEXT(int x, int y, int z) const
    {
        const auto it = tiles_.find(TileCoordinate3DEXT(x, y, z));
        return it != tiles_.end() ? it->second : 0;
    }

    void Tilemap3DEXT::SetTileEXT(int x, int y, int z, int tileId)
    {
        if (tileId == 0)
        {
            RemoveTileEXT(x, y, z);
            return;
        }

        tiles_[TileCoordinate3DEXT(x, y, z)] = tileId;
    }

    void Tilemap3DEXT::RemoveTileEXT(int x, int y, int z)
    {
        tiles_.erase(TileCoordinate3DEXT(x, y, z));
    }

    bool Tilemap3DEXT::HasTileEXT(int x, int y, int z) const
    {
        return tiles_.contains(TileCoordinate3DEXT(x, y, z));
    }

    Vector3 Tilemap3DEXT::TileToWorldPositionEXT(int x, int y, int z) const
    {
        return Vector3(static_cast<float>(x) * tileSize_.X, static_cast<float>(y) * tileSize_.Y, static_cast<float>(z) * tileSize_.Z);
    }
}
