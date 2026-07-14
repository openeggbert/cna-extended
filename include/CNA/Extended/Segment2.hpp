// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Segment2.cs. Upstream cites "Real-Time Collision
// Detection, Christer Ericson, 2005" (Chapter 3.5/5.1.2, pg 53-54/127-130) as the algorithmic
// source -- an academic citation in a comment, not a third-party code/license dependency (this
// file's code is still 100% Craftwork Games MIT).
//
// *** LIKELY UPSTREAM BUG, PRESERVED FOR FIDELITY -- flagged prominently, not silently fixed ***
// SquaredDistanceTo(Vector2) has a branch that computes `endToPoint.Dot(endToPoint)` (the correct
// answer for "point projects beyond the End of the segment") but never returns it -- the result
// is discarded and execution falls through to the "point projects onto the segment interior"
// formula regardless. This looks like a missing `return` typo (the two other branches both
// return immediately; a code comment even says "Handle cases where point projects outside
// segment", implying both boundary cases were meant to return early). The C++ port below
// reproduces this exact behavior byte-for-semantics -- per this project's explicit "port 1:1,
// no simplification" requirement, "fixing" an upstream bug during a mechanical port is not this
// port's call to make. See Segment2.cpp's SquaredDistanceTo for where this is replicated, and
// the ported test file for a case demonstrating the resulting wrong answer for a point beyond
// End. Recommend the user decide whether to file this upstream or diverge from it locally.
//
// All members are now ported. The two Intersects(RectangleF|BoundingRectangle, out Vector2)
// overloads call PrimitivesHelper.IntersectsSlab, landed once PrimitivesHelper was ported
// (task 22 -- verified genuinely unblocked, not assumed).
//
// IEquatable<Segment2>/IEquatableByRef<Segment2> are not implemented as C++ interfaces (matches
// the precedent set by the bounding-volume types) -- just a plain Equals(const Segment2&).
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    struct RectangleF;
    struct BoundingRectangle;

    /** @brief A two dimensional line segment defined by a starting and an ending Vector2. */
    struct Segment2
    {
        /** @brief The starting Vector2 of this Segment2. */
        Vector2 Start;

        /** @brief The ending Vector2 of this Segment2. */
        Vector2 End;

        Segment2() = default;

        /**
         * @brief Initializes a new Segment2 from the specified starting and ending Vector2.
         * @param start The starting point.
         * @param end The ending point.
         */
        Segment2(const Vector2& start, const Vector2& end);

        /**
         * @brief Initializes a new Segment2 from the specified starting and ending coordinates.
         * @param x1 The starting x-coordinate.
         * @param y1 The starting y-coordinate.
         * @param x2 The ending x-coordinate.
         * @param y2 The ending y-coordinate.
         */
        Segment2(float x1, float y1, float x2, float y2);

        /** @brief Computes the closest Vector2 on this Segment2 to a specified Vector2. */
        [[nodiscard]] Vector2 ClosestPointTo(const Vector2& point) const;

        /**
         * @brief Computes the squared distance from this Segment2 to a specified Vector2. See
         * this file's header comment for a known upstream fidelity note about this method.
         */
        [[nodiscard]] float SquaredDistanceTo(const Vector2& point) const;

        /** @brief Computes the distance from this Segment2 to a specified Vector2. */
        [[nodiscard]] float DistanceTo(const Vector2& point) const;

        /**
         * @brief Determines whether this Segment2 intersects with the specified RectangleF.
         * @param rectangle The rectangle to test for intersection.
         * @param intersectionPoint Receives the point of intersection if found, otherwise
         * Vector2(NaN, NaN).
         */
        [[nodiscard]] bool Intersects(const RectangleF& rectangle, Vector2& intersectionPoint) const;

        /**
         * @brief Determines whether this Segment2 intersects with the specified
         * BoundingRectangle.
         * @param boundingRectangle The bounding rectangle to test for intersection.
         * @param intersectionPoint Receives the point of intersection if found, otherwise
         * Vector2(NaN, NaN).
         */
        [[nodiscard]] bool Intersects(const BoundingRectangle& boundingRectangle, Vector2& intersectionPoint) const;

        [[nodiscard]] bool Equals(const Segment2& segment) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const Segment2& first, const Segment2& second) { return first.Equals(second); }
        friend bool operator!=(const Segment2& first, const Segment2& second) { return !first.Equals(second); }
    };
}
