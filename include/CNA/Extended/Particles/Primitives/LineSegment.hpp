// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Primatives/LineSegment.cs. Upstream's own folder is
// misspelled "Primatives" on disk, but the file's actual declared namespace is the correctly-spelled
// `MonoGame.Extended.Particles.Primitives` -- ported into the matching, correctly-spelled
// `CNA::Extended::Particles::Primitives` (and file path), per this project's "namespace follows the
// real C# declaration, not the file's on-disk location" convention. This is a distinct, standalone
// type from this project's existing `CNA::Extended::LineSegment2D` (Phase 1, general
// collision/geometry primitive) and `CNA::Extended::Triangulation::LineSegment` (Phase 1,
// triangulation-specific) -- checked both before porting; neither has the same API surface
// (Origin/Direction/Translate/ToVector2/FromPoints/FromOrigin) or purpose (this one exists purely
// to let `ParticleEmitter::Trigger` distribute particles along a line), so it is ported as its own
// type here, matching upstream's own decision to keep it separate rather than reusing a shared type.
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended::Particles::Primitives
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief A line segment defined by two points, used to distribute triggered particles along a line. */
    struct LineSegment
    {
        LineSegment() = default;

        /** @brief Creates a line segment from @p point1 to @p point2. */
        LineSegment(const Vector2& point1, const Vector2& point2);

        /** @brief Gets the origin (first point) of the line segment. */
        [[nodiscard]] const Vector2& getOriginProperty() const { return point1_; }

        /** @brief Gets the direction vector from the origin to the second point. */
        [[nodiscard]] Vector2 getDirectionProperty() const { return point2_ - point1_; }

        /** @brief Returns a copy of this line segment translated by @p vector. */
        [[nodiscard]] LineSegment Translate(const Vector2& vector) const;

        /** @brief Converts this line segment to a vector: the second point minus the first. */
        [[nodiscard]] Vector2 ToVector2() const { return point2_ - point1_; }

        /** @brief Creates a line segment from two points. */
        [[nodiscard]] static LineSegment FromPoints(const Vector2& point1, const Vector2& point2);

        /** @brief Creates a line segment starting at @p origin and extending by @p vector. */
        [[nodiscard]] static LineSegment FromOrigin(const Vector2& origin, const Vector2& vector);

        [[nodiscard]] bool Equals(const LineSegment& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const LineSegment& left, const LineSegment& right) { return left.Equals(right); }
        friend bool operator!=(const LineSegment& left, const LineSegment& right) { return !left.Equals(right); }

    private:
        Vector2 point1_;
        Vector2 point2_;
    };
}
