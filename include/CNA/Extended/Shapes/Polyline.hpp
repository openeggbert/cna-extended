// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Shapes/Polyline.cs.
#pragma once

#include "CNA/Extended/RectangleF.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <vector>

namespace CNA::Extended::Shapes
{
    using Microsoft::Xna::Framework::Vector2;
    using CNA::Extended::RectangleF;

    /** @brief Represents an open sequence of connected line segments defined by a set of points. */
    class Polyline
    {
    public:
        /**
         * @brief Initializes a new Polyline with the specified points.
         * @param points The points of the polyline.
         */
        explicit Polyline(std::vector<Vector2> points);

        /** @brief Gets the points of this polyline. */
        [[nodiscard]] const std::vector<Vector2>& getPointsProperty() const { return points_; }

        /** @brief Gets the minimum X coordinate among this polyline's points. */
        [[nodiscard]] float getLeftProperty() const;

        /** @brief Gets the minimum Y coordinate among this polyline's points. */
        [[nodiscard]] float getTopProperty() const;

        /** @brief Gets the maximum X coordinate among this polyline's points. */
        [[nodiscard]] float getRightProperty() const;

        /** @brief Gets the maximum Y coordinate among this polyline's points. */
        [[nodiscard]] float getBottomProperty() const;

        /** @brief Gets the axis-aligned bounding rectangle of this polyline. */
        [[nodiscard]] RectangleF getBoundingRectangleProperty() const;

        /**
         * @brief Always returns false. Matches upstream's Polyline.Contains(float, float), which
         * is a stub -- a polyline is an open curve with no interior, so containment is not
         * meaningfully defined, but upstream still declares (and always returns false from) this
         * method rather than omitting it. Preserved as-is, not "completed" with a real geometric
         * test.
         */
        [[nodiscard]] bool Contains(float x, float y) const;

        /** @brief Always returns false; see the float,float overload's documentation. */
        [[nodiscard]] bool Contains(const Vector2& point) const;

    private:
        std::vector<Vector2> points_;
    };
}
