// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapObjects/TilemapPolygonObject.cs.
#pragma once

#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/Tilemaps/TilemapObject.hpp"

#include <vector>

namespace CNA::Extended::Tilemaps
{
    /** @brief A closed polygonal object in a tilemap. */
    class TilemapPolygonObject : public TilemapObject
    {
    public:
        /** @brief Creates a polygon object with @p id, origin @p position, and @p points relative to that position. */
        TilemapPolygonObject(int id, const Vector2& position, std::vector<Vector2> points);

        /** @brief Gets/sets the vertices of the polygon relative to the object's position. */
        [[nodiscard]] const std::vector<Vector2>& getPointsProperty() const { return points_; }
        void setPointsProperty(std::vector<Vector2> value) { points_ = std::move(value); }

        /** @brief Gets the vertices of the polygon in world coordinates. */
        [[nodiscard]] std::vector<Vector2> getWorldPointsProperty() const;

        /**
         * @brief Gets the convex polygon bounding shape for this object in world coordinates, wound
         * counter-clockwise (BoundingPolygon2D requires it). If this object's polygon is concave,
         * collision results using this shape will be incorrect -- use getWorldPointsProperty() to
         * access the exact geometry for concave polygons.
         */
        [[nodiscard]] BoundingPolygon2D getShapeProperty() const;

        [[nodiscard]] BoundingBox2D getBoundsProperty() const override;

    private:
        std::vector<Vector2> points_;
    };
}
