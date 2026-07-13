// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Triangulation/Triangulator.cs. Upstream's own file header
// additionally credits nickgravelyn's Triangulator project
// (https://github.com/nickgravelyn/Triangulator, an ear-clipping triangulation implementation)
// as the origin of this code ("MIT Licensed"). Carried forward here for the same reason
// MonoGame.Extended carries it: this code traces back further than Craftwork Games. See
// NOTICE.md for a provenance note about this specific attribution.
//
// CRITICAL fidelity note -- NOT thread-safe or reentrant, by upstream design (not a bug):
// upstream's `polygonVertices`/`earVertices`/`convexVertices`/`reflexVertices`/`_trianglesBuffer`
// are `static readonly` fields: shared, reused, mutable buffers across every call to
// Triangulate/CutHoleInShape. Upstream's own class doc comment explains why: "Per-call
// allocations have been significantly reduced... It is not something you want to be using each
// and every frame unless you really don't care about garbage." This is ported below as genuine
// static class-level mutable member state (not function-local variables, not thread_local) to
// preserve that exact tradeoff -- do NOT call Triangulate/CutHoleInShape concurrently from
// multiple threads, or recursively/re-entrantly from within each other; doing so will corrupt
// the shared buffers, matching upstream's own limitation exactly.
//
// Upstream's DEBUG-only Log(...) tracing calls ([Conditional("DEBUG")], and in practice a
// permanent no-op even in upstream -- its body is a commented-out System.Console.WriteLine) are
// omitted entirely below rather than ported as dead calls.
//
// IsReflex is dead code upstream (never called anywhere in Triangulator.cs, confirmed by
// grep); ported anyway for completeness as a private static method -- unlike an unused free or
// anonymous-namespace function, an unused private class method triggers no compiler warning.
#pragma once

#include "CNA/Extended/Triangulation/CyclicalList.hpp"
#include "CNA/Extended/Triangulation/IndexableCyclicalLinkedList.hpp"
#include "CNA/Extended/Triangulation/LineSegment.hpp"
#include "CNA/Extended/Triangulation/Triangle.hpp"
#include "CNA/Extended/Triangulation/Vertex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <vector>

namespace CNA::Extended::Triangulation
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief Specifies a desired winding order for the shape vertices. */
    enum class WindingOrder
    {
        Clockwise,
        CounterClockwise
    };

    /**
     * @brief Exposes static methods for triangulating 2D polygons via ear clipping. This is the
     * only type in this module intended for use outside of it -- Vertex, LineSegment, Triangle,
     * CyclicalList, and IndexableCyclicalLinkedList are internal-only implementation details,
     * matching upstream's own "sole public class in the entire library" design.
     */
    class Triangulator
    {
    public:
        Triangulator() = delete;

        /**
         * @brief Triangulates a 2D polygon, producing the vertices and indices required to
         * render the points as a triangle list.
         * @param inputVertices The polygon vertices in counter-clockwise winding order.
         * @param desiredWindingOrder The desired output winding order.
         * @param outputVertices Receives the resulting vertices, including any reversal of
         * winding order.
         * @param indices Receives the resulting indices for rendering the shape as a triangle
         * list.
         */
        static void Triangulate(const std::vector<Vector2>& inputVertices, WindingOrder desiredWindingOrder,
            std::vector<Vector2>& outputVertices, std::vector<int>& indices);

        /**
         * @brief Cuts a hole into a shape.
         * @param shapeVerts The vertices of the primary shape.
         * @param holeVerts The vertices of the hole to be cut. Assumed to lie completely within
         * @p shapeVerts.
         * @return The new set of vertices that can be passed to Triangulate to properly
         * triangulate the shape with the hole.
         */
        [[nodiscard]] static std::vector<Vector2> CutHoleInShape(std::vector<Vector2> shapeVerts, std::vector<Vector2> holeVerts);

        /**
         * @brief Ensures that a set of vertices are wound in a particular order, reversing them
         * if necessary.
         */
        [[nodiscard]] static std::vector<Vector2> EnsureWindingOrder(const std::vector<Vector2>& vertices, WindingOrder windingOrder);

        /** @brief Reverses the winding order for a set of vertices. */
        [[nodiscard]] static std::vector<Vector2> ReverseWindingOrder(const std::vector<Vector2>& vertices);

        /** @brief Determines the winding order of a polygon given a set of vertices. */
        [[nodiscard]] static WindingOrder DetermineWindingOrder(const std::vector<Vector2>& vertices);

    private:
        static void ClipNextEar(std::vector<Triangle>& triangles);
        static void ValidateAdjacentVertex(const Vertex& vertex);
        static void FindConvexAndReflexVertices();
        static void FindEarVertices();
        [[nodiscard]] static bool IsEar(const Vertex& c);
        [[nodiscard]] static bool IsConvex(const Vertex& c);
        [[nodiscard]] static bool IsReflex(const Vertex& c);

        static inline IndexableCyclicalLinkedList<Vertex> polygonVertices_;
        static inline IndexableCyclicalLinkedList<Vertex> earVertices_;
        static inline CyclicalList<Vertex> convexVertices_;
        static inline CyclicalList<Vertex> reflexVertices_;
        static inline std::vector<Triangle> trianglesBuffer_;
    };
}
