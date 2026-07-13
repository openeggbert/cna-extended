// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/EllipseF.cs. Fully self-contained -- no dependency on
// any not-yet-ported type, so this file has no deferrals at all.
//
// IEquatable<EllipseF>/IEquatableByRef<EllipseF> are not implemented as C++ interfaces (matches
// the precedent set by the bounding-volume types) -- just a plain Equals(const EllipseF&).
#pragma once

#include "CNA/Extended/RectangleF.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief A two dimensional ellipse defined by a centre Vector2 and X/Y radii. */
    struct EllipseF
    {
        EllipseF() = default;

        /**
         * @brief Initializes a new EllipseF from the specified centre and X/Y radii.
         * @param center The centre point.
         * @param radiusX The radius along the X axis.
         * @param radiusY The radius along the Y axis.
         */
        EllipseF(const Vector2& center, float radiusX, float radiusY);

        /** @brief Gets or sets the centre position of this ellipse. */
        [[nodiscard]] Vector2 getCenterProperty() const { return center_; }
        void setCenterProperty(const Vector2& value) { center_ = value; }

        /** @brief Gets or sets the radius along the X axis of this ellipse. */
        [[nodiscard]] float getRadiusXProperty() const { return radiusX_; }
        void setRadiusXProperty(const float value) { radiusX_ = value; }

        /** @brief Gets or sets the radius along the Y axis of this ellipse. */
        [[nodiscard]] float getRadiusYProperty() const { return radiusY_; }
        void setRadiusYProperty(const float value) { radiusY_ = value; }

        /** @brief Gets or sets the position of this ellipse (an alias for Center). */
        [[nodiscard]] Vector2 getPositionProperty() const { return center_; }
        void setPositionProperty(const Vector2& value) { center_ = value; }

        [[nodiscard]] float getLeftProperty() const { return center_.X - radiusX_; }
        [[nodiscard]] float getTopProperty() const { return center_.Y - radiusY_; }
        [[nodiscard]] float getRightProperty() const { return center_.X + radiusX_; }
        [[nodiscard]] float getBottomProperty() const { return center_.Y + radiusY_; }

        /** @brief Gets the axis-aligned bounding rectangle of this ellipse. */
        [[nodiscard]] RectangleF getBoundingRectangleProperty() const;

        [[nodiscard]] bool Contains(float x, float y) const;
        [[nodiscard]] bool Contains(const Vector2& point) const;

        [[nodiscard]] bool Equals(const EllipseF& ellipse) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const EllipseF& first, const EllipseF& second) { return first.Equals(second); }
        friend bool operator!=(const EllipseF& first, const EllipseF& second) { return !first.Equals(second); }

    private:
        Vector2 center_;
        float radiusX_ = 0.0f;
        float radiusY_ = 0.0f;
    };
}
