// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Triangulation/Triangle.hpp"

namespace CNA::Extended::Triangulation
{
    Triangle::Triangle(const Vertex& a, const Vertex& b, const Vertex& c) : A(a), B(b), C(c)
    {
    }

    bool Triangle::ContainsPoint(const Vertex& point) const
    {
        // Return true if the point to test is one of the vertices.
        if (point.Equals(A) || point.Equals(B) || point.Equals(C))
        {
            return true;
        }

        bool oddNodes = false;

        if (checkPointToSegment(C, A, point))
        {
            oddNodes = !oddNodes;
        }
        if (checkPointToSegment(A, B, point))
        {
            oddNodes = !oddNodes;
        }
        if (checkPointToSegment(B, C, point))
        {
            oddNodes = !oddNodes;
        }

        return oddNodes;
    }

    bool Triangle::ContainsPoint(const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& point)
    {
        return Triangle(a, b, c).ContainsPoint(point);
    }

    bool Triangle::checkPointToSegment(const Vertex& sA, const Vertex& sB, const Vertex& point)
    {
        if ((sA.Position.Y < point.Position.Y && sB.Position.Y >= point.Position.Y) ||
            (sB.Position.Y < point.Position.Y && sA.Position.Y >= point.Position.Y))
        {
            const float x = sA.Position.X + (point.Position.Y - sA.Position.Y) / (sB.Position.Y - sA.Position.Y) * (sB.Position.X - sA.Position.X);

            if (x < point.Position.X)
            {
                return true;
            }
        }

        return false;
    }

    bool Triangle::Equals(const Triangle& other) const
    {
        return other.A.Equals(A) && other.B.Equals(B) && other.C.Equals(C);
    }

    int Triangle::GetHashCode() const
    {
        // Matches upstream's `unchecked { result = A.GetHashCode(); result = (result * 397) ^ B.GetHashCode(); ... }`.
        int result = A.GetHashCode();
        result = (result * 397) ^ B.GetHashCode();
        result = (result * 397) ^ C.GetHashCode();
        return result;
    }
}
