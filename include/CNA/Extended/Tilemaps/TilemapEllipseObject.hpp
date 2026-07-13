// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapObjects/TilemapEllipseObject.cs.
#pragma once

#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/Tilemaps/TilemapObject.hpp"

#include <optional>

namespace CNA::Extended::Tilemaps
{
    /** @brief An elliptical object in a tilemap. */
    class TilemapEllipseObject : public TilemapObject
    {
    public:
        /** @brief Creates an ellipse object with @p id, top-left @p position, and bounding-rectangle @p size. */
        TilemapEllipseObject(int id, const Vector2& position, const Vector2& size);

        /** @brief Gets/sets the size of the ellipse's bounding rectangle. */
        [[nodiscard]] const Vector2& getSizeProperty() const { return size_; }
        void setSizeProperty(const Vector2& value) { size_ = value; }

        /** @brief Gets the center point of the ellipse. */
        [[nodiscard]] Vector2 getCenterProperty() const { return getPositionProperty() + size_ * 0.5f; }

        /** @brief Gets the horizontal radius of the ellipse. */
        [[nodiscard]] float getRadiusXProperty() const { return size_.X * 0.5f; }

        /** @brief Gets the vertical radius of the ellipse. */
        [[nodiscard]] float getRadiusYProperty() const { return size_.Y * 0.5f; }

        /**
         * @brief Gets a bounding circle for this ellipse when it is circular, or std::nullopt when it is not
         * (RadiusX/RadiusY differ by more than floating-point tolerance). For non-uniform ellipses, use
         * getBoundsProperty() as an approximation.
         */
        [[nodiscard]] std::optional<BoundingCircle2D> getCircleProperty() const;

        [[nodiscard]] BoundingBox2D getBoundsProperty() const override;

    private:
        Vector2 size_;
    };
}
