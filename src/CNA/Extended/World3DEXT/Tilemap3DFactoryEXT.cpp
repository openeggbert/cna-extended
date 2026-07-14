// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/Tilemap3DFactoryEXT.hpp"

#include "CNA/Extended/World3DEXT/Tilemap3DEXT.hpp"
#include "System/ArgumentException.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    Tilemap3DEXT Tilemap3DFactoryEXT::BuildFromArrayEXT(const std::vector<int>& tileIds, int width, int height, int depth,
                                                          const Vector3& tileSize)
    {
        if (static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * static_cast<std::size_t>(depth) != tileIds.size())
        {
            throw System::ArgumentException("tileIds size must equal width * height * depth.");
        }

        Tilemap3DEXT tilemap(tileSize);
        for (int z = 0; z < depth; ++z)
        {
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    const std::size_t index =
                        (static_cast<std::size_t>(z) * static_cast<std::size_t>(height) + static_cast<std::size_t>(y))
                            * static_cast<std::size_t>(width)
                        + static_cast<std::size_t>(x);
                    const int tileId = tileIds[index];
                    if (tileId != 0)
                    {
                        tilemap.SetTileEXT(x, y, z, tileId);
                    }
                }
            }
        }

        return tilemap;
    }
}
