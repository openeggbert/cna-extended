// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapObjects/TilemapPolylineObject.cs.
#pragma once

#include "CNA/Extended/LineSegment2D.hpp"
#include "CNA/Extended/Tilemaps/TilemapObject.hpp"

#include <vector>

namespace CNA::Extended::Tilemaps
{
    /** @brief An open polyline (unclosed polygon) object in a tilemap: a series of connected line segments, typically used for paths or boundaries. */
    class TilemapPolylineObject : public TilemapObject
    {
    public:
        /** @brief Creates a polyline object with @p id, origin @p position, and @p points relative to that position. */
        TilemapPolylineObject(int id, const Vector2& position, std::vector<Vector2> points);

        /** @brief Gets/sets the vertices of the polyline relative to the object's position. */
        [[nodiscard]] const std::vector<Vector2>& getPointsProperty() const { return points_; }
        void setPointsProperty(std::vector<Vector2> value) { points_ = std::move(value); }

        /** @brief Gets the vertices of the polyline in world coordinates. */
        [[nodiscard]] std::vector<Vector2> getWorldPointsProperty() const;

        /** @brief Gets the line segments of the polyline in world coordinates. Empty if fewer than 2 points. */
        [[nodiscard]] std::vector<LineSegment2D> getSegmentsProperty() const;

        [[nodiscard]] BoundingBox2D getBoundsProperty() const override;

    private:
        std::vector<Vector2> points_;
    };
}
