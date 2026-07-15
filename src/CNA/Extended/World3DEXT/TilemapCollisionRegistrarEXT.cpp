// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/TilemapCollisionRegistrarEXT.hpp"

#include "CNA/Extended/World3DEXT/CollisionShape3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Tilemap3DEXT.hpp"
#include "CNA/Extended/World3DEXT/TilemapCollisionActor3DEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;
    using Microsoft::Xna::Framework::Vector3;

    std::vector<std::unique_ptr<TilemapCollisionActor3DEXT>> RegisterTilemapCollisionActorsEXT(CollisionWorld3DEXT& world,
                                                                                                 const Tilemap3DEXT& tilemap,
                                                                                                 const std::string& layerName)
    {
        std::vector<std::unique_ptr<TilemapCollisionActor3DEXT>> actors;
        actors.reserve(tilemap.getTilesProperty().size());

        const Vector3 halfSize = tilemap.getTileSizeProperty() * 0.5f;

        int nextId = 0;
        for (const auto& [coordinate, tileId] : tilemap.getTilesProperty())
        {
            const Vector3 center = tilemap.TileToWorldPositionEXT(coordinate.X, coordinate.Y, coordinate.Z);
            const BoundingBox box(center - halfSize, center + halfSize);

            auto actor = std::make_unique<TilemapCollisionActor3DEXT>(nextId, coordinate.X, coordinate.Y, coordinate.Z, tileId,
                                                                        CollisionShape3DEXT(box));
            ++nextId;

            world.Insert(actor.get(), layerName);
            actors.push_back(std::move(actor));
        }

        return actors;
    }
}
