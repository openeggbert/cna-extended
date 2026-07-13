// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Triangulation/Triangulator.hpp"

#include <optional>

namespace CNA::Extended::Triangulation
{
    using SharpRuntime::intcs;

    void Triangulator::Triangulate(const std::vector<Vector2>& inputVertices, const WindingOrder desiredWindingOrder,
        std::vector<Vector2>& outputVertices, std::vector<int>& indices)
    {
        std::vector<Triangle>& triangles = trianglesBuffer_;
        triangles.clear();

        // Make sure we have our vertices wound properly.
        if (DetermineWindingOrder(inputVertices) == WindingOrder::Clockwise)
        {
            outputVertices = ReverseWindingOrder(inputVertices);
        }
        else
        {
            outputVertices = inputVertices;
        }

        // Clear all of the lists.
        polygonVertices_.Clear();
        earVertices_.Clear();
        convexVertices_.Clear();
        reflexVertices_.Clear();

        // Generate the cyclical list of vertices in the polygon.
        for (std::size_t i = 0; i < outputVertices.size(); i++)
        {
            polygonVertices_.AddLast(Vertex(outputVertices[i], static_cast<int>(i)));
        }

        // Categorize all of the vertices as convex, reflex, and ear.
        FindConvexAndReflexVertices();
        FindEarVertices();

        // Clip all the ear vertices.
        while (polygonVertices_.getCountProperty() > 3 && earVertices_.getCountProperty() > 0)
        {
            ClipNextEar(triangles);
        }

        // If there are still three points, use that for the last triangle.
        if (polygonVertices_.getCountProperty() == 3)
        {
            triangles.push_back(
                Triangle(polygonVertices_[0].getValueProperty(), polygonVertices_[1].getValueProperty(), polygonVertices_[2].getValueProperty()));
        }

        // Add all of the triangle indices to the output array.
        indices.assign(triangles.size() * 3, 0);

        // Move the if statement out of the loop to prevent all the redundant comparisons.
        if (desiredWindingOrder == WindingOrder::CounterClockwise)
        {
            for (std::size_t i = 0; i < triangles.size(); i++)
            {
                indices[i * 3] = triangles[i].A.Index;
                indices[i * 3 + 1] = triangles[i].B.Index;
                indices[i * 3 + 2] = triangles[i].C.Index;
            }
        }
        else
        {
            for (std::size_t i = 0; i < triangles.size(); i++)
            {
                indices[i * 3] = triangles[i].C.Index;
                indices[i * 3 + 1] = triangles[i].B.Index;
                indices[i * 3 + 2] = triangles[i].A.Index;
            }
        }
    }

    std::vector<Vector2> Triangulator::CutHoleInShape(std::vector<Vector2> shapeVerts, std::vector<Vector2> holeVerts)
    {
        // Make sure the shape vertices are wound counter clockwise and the hole vertices clockwise.
        shapeVerts = EnsureWindingOrder(shapeVerts, WindingOrder::CounterClockwise);
        holeVerts = EnsureWindingOrder(holeVerts, WindingOrder::Clockwise);

        // Clear all of the lists.
        polygonVertices_.Clear();
        earVertices_.Clear();
        convexVertices_.Clear();
        reflexVertices_.Clear();

        // Generate the cyclical list of vertices in the polygon.
        for (std::size_t i = 0; i < shapeVerts.size(); i++)
        {
            polygonVertices_.AddLast(Vertex(shapeVerts[i], static_cast<int>(i)));
        }

        CyclicalList<Vertex> holePolygon;
        for (std::size_t i = 0; i < holeVerts.size(); i++)
        {
            holePolygon.Add(Vertex(holeVerts[i], static_cast<int>(i) + polygonVertices_.getCountProperty()));
        }

        FindConvexAndReflexVertices();
        FindEarVertices();

        // Find the hole vertex with the largest X value.
        Vertex rightMostHoleVertex = holePolygon[0];
        for (const Vertex& v : holePolygon)
        {
            if (v.Position.X > rightMostHoleVertex.Position.X)
            {
                rightMostHoleVertex = v;
            }
        }

        // Construct a list of all line segments where at least one vertex is to the right of the
        // rightmost hole vertex with one vertex above the hole vertex and one below.
        std::vector<LineSegment> segmentsToTest;
        for (intcs i = 0; i < polygonVertices_.getCountProperty(); i++)
        {
            const Vertex a = polygonVertices_[i].getValueProperty();
            const Vertex b = polygonVertices_[i + 1].getValueProperty();

            if ((a.Position.X > rightMostHoleVertex.Position.X || b.Position.X > rightMostHoleVertex.Position.X) &&
                ((a.Position.Y >= rightMostHoleVertex.Position.Y && b.Position.Y <= rightMostHoleVertex.Position.Y) ||
                    (a.Position.Y <= rightMostHoleVertex.Position.Y && b.Position.Y >= rightMostHoleVertex.Position.Y)))
            {
                segmentsToTest.emplace_back(a, b);
            }
        }

        // Now we try to find the closest intersection point heading to the right from our hole vertex.
        std::optional<float> closestPoint;
        LineSegment closestSegment;
        for (const LineSegment& segment : segmentsToTest)
        {
            const std::optional<float> intersection = segment.IntersectsWithRay(rightMostHoleVertex.Position, Vector2::UnitX);
            if (intersection.has_value())
            {
                if (!closestPoint.has_value() || closestPoint.value() > intersection.value())
                {
                    closestPoint = intersection;
                    closestSegment = segment;
                }
            }
        }

        // If closestPoint has no value, there were no collisions (likely from improper input
        // data), but we'll just return without doing anything else.
        if (!closestPoint.has_value())
        {
            return shapeVerts;
        }

        // Otherwise we can find our mutually visible vertex to split the polygon.
        const Vector2 intersectionPoint = rightMostHoleVertex.Position + Vector2::UnitX * closestPoint.value();
        Vertex P = (closestSegment.A.Position.X > closestSegment.B.Position.X) ? closestSegment.A : closestSegment.B;

        // Construct triangle MIP.
        const Triangle mip(rightMostHoleVertex, Vertex(intersectionPoint, 1), P);

        // See if any of the reflex vertices lie inside of the MIP triangle.
        std::vector<Vertex> interiorReflexVertices;
        for (const Vertex& v : reflexVertices_)
        {
            if (mip.ContainsPoint(v))
            {
                interiorReflexVertices.push_back(v);
            }
        }

        // If there are any interior reflex vertices, find the one that, when connected to our
        // rightMostHoleVertex, forms the line closest to Vector2::UnitX.
        if (!interiorReflexVertices.empty())
        {
            float closestDot = -1.0f;
            for (const Vertex& v : interiorReflexVertices)
            {
                const Vector2 d = Vector2::Normalize(v.Position - rightMostHoleVertex.Position);
                const float dot = Vector2::Dot(Vector2::UnitX, d);

                if (dot > closestDot)
                {
                    closestDot = dot;
                    P = v;
                }
            }
        }

        // Now we just form our output array by injecting the hole vertices into place. We know
        // we have to inject the hole into the main array after point P going from
        // rightMostHoleVertex around and then back to P.
        const intcs mIndex = holePolygon.IndexOf(rightMostHoleVertex);
        intcs injectPoint = polygonVertices_.IndexOf(P);

        for (intcs i = mIndex; i <= mIndex + holePolygon.getCountProperty(); i++)
        {
            polygonVertices_.AddAfter(polygonVertices_[injectPoint++], holePolygon[i]);
        }
        polygonVertices_.AddAfter(polygonVertices_[injectPoint], P);

        // Finally we write out the new polygon vertices and return them out.
        std::vector<Vector2> newShapeVerts(static_cast<std::size_t>(polygonVertices_.getCountProperty()));
        for (intcs i = 0; i < polygonVertices_.getCountProperty(); i++)
        {
            newShapeVerts[static_cast<std::size_t>(i)] = polygonVertices_[i].getValueProperty().Position;
        }

        return newShapeVerts;
    }

    std::vector<Vector2> Triangulator::EnsureWindingOrder(const std::vector<Vector2>& vertices, const WindingOrder windingOrder)
    {
        if (DetermineWindingOrder(vertices) != windingOrder)
        {
            return ReverseWindingOrder(vertices);
        }

        return vertices;
    }

    std::vector<Vector2> Triangulator::ReverseWindingOrder(const std::vector<Vector2>& vertices)
    {
        std::vector<Vector2> newVerts(vertices.size());

        newVerts[0] = vertices[0];
        for (std::size_t i = 1; i < newVerts.size(); i++)
        {
            newVerts[i] = vertices[vertices.size() - i];
        }

        return newVerts;
    }

    WindingOrder Triangulator::DetermineWindingOrder(const std::vector<Vector2>& vertices)
    {
        float sum = 0.0f;
        Vector2 v1 = vertices[vertices.size() - 1];

        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            const Vector2 v2 = vertices[i];
            sum += (v2.X - v1.X) * (v2.Y + v1.Y);
            v1 = v2;
        }

        return sum > 0.0f ? WindingOrder::Clockwise : WindingOrder::CounterClockwise;
    }

    void Triangulator::ClipNextEar(std::vector<Triangle>& triangles)
    {
        // Find the triangle.
        const Vertex ear = earVertices_[0].getValueProperty();
        const Vertex prev = polygonVertices_[polygonVertices_.IndexOf(ear) - 1].getValueProperty();
        const Vertex next = polygonVertices_[polygonVertices_.IndexOf(ear) + 1].getValueProperty();
        triangles.push_back(Triangle(ear, next, prev));

        // Remove the ear from the shape.
        earVertices_.RemoveAt(0);
        polygonVertices_.RemoveAt(polygonVertices_.IndexOf(ear));

        // Validate the neighboring vertices.
        ValidateAdjacentVertex(prev);
        ValidateAdjacentVertex(next);
    }

    void Triangulator::ValidateAdjacentVertex(const Vertex& vertex)
    {
        if (reflexVertices_.Contains(vertex))
        {
            if (IsConvex(vertex))
            {
                reflexVertices_.Remove(vertex);
                convexVertices_.Add(vertex);
            }
        }

        if (convexVertices_.Contains(vertex))
        {
            const bool wasEar = earVertices_.Contains(vertex);
            const bool isEar = IsEar(vertex);

            if (wasEar && !isEar)
            {
                earVertices_.Remove(vertex);
            }
            else if (!wasEar && isEar)
            {
                earVertices_.AddFirst(vertex);
            }
        }
    }

    void Triangulator::FindConvexAndReflexVertices()
    {
        for (intcs i = 0; i < polygonVertices_.getCountProperty(); i++)
        {
            const Vertex v = polygonVertices_[i].getValueProperty();

            if (IsConvex(v))
            {
                convexVertices_.Add(v);
            }
            else
            {
                reflexVertices_.Add(v);
            }
        }
    }

    void Triangulator::FindEarVertices()
    {
        for (intcs i = 0; i < convexVertices_.getCountProperty(); i++)
        {
            const Vertex c = convexVertices_[i];

            if (IsEar(c))
            {
                earVertices_.AddLast(c);
            }
        }
    }

    bool Triangulator::IsEar(const Vertex& c)
    {
        const Vertex p = polygonVertices_[polygonVertices_.IndexOf(c) - 1].getValueProperty();
        const Vertex n = polygonVertices_[polygonVertices_.IndexOf(c) + 1].getValueProperty();

        for (const Vertex& t : reflexVertices_)
        {
            if (t.Equals(p) || t.Equals(c) || t.Equals(n))
            {
                continue;
            }

            if (Triangle::ContainsPoint(p, c, n, t))
            {
                return false;
            }
        }

        return true;
    }

    bool Triangulator::IsConvex(const Vertex& c)
    {
        const Vertex p = polygonVertices_[polygonVertices_.IndexOf(c) - 1].getValueProperty();
        const Vertex n = polygonVertices_[polygonVertices_.IndexOf(c) + 1].getValueProperty();

        const Vector2 d1 = Vector2::Normalize(c.Position - p.Position);
        const Vector2 d2 = Vector2::Normalize(n.Position - c.Position);
        const Vector2 n2(-d2.Y, d2.X);

        return Vector2::Dot(d1, n2) <= 0.0f;
    }

    bool Triangulator::IsReflex(const Vertex& c)
    {
        return !IsConvex(c);
    }
}
