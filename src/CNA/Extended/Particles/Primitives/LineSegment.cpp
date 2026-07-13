// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Primitives/LineSegment.hpp"

#include "System/HashCode.hpp"

namespace CNA::Extended::Particles::Primitives
{
    LineSegment::LineSegment(const Vector2& point1, const Vector2& point2) : point1_(point1), point2_(point2)
    {
    }

    LineSegment LineSegment::Translate(const Vector2& vector) const
    {
        return LineSegment(point1_ + vector, point2_ + vector);
    }

    LineSegment LineSegment::FromPoints(const Vector2& point1, const Vector2& point2)
    {
        return LineSegment(point1, point2);
    }

    LineSegment LineSegment::FromOrigin(const Vector2& origin, const Vector2& vector)
    {
        return LineSegment(origin, origin + vector);
    }

    bool LineSegment::Equals(const LineSegment& other) const
    {
        return point1_.Equals(other.point1_) && point2_.Equals(other.point2_);
    }

    int LineSegment::GetHashCode() const
    {
        return System::HashCode::Combine(point1_.GetHashCode(), point2_.GetHashCode());
    }

    std::string LineSegment::ToString() const
    {
        return "(" + std::to_string(point1_.X) + ":" + std::to_string(point1_.Y) + "," + std::to_string(point2_.X) + ":"
            + std::to_string(point2_.Y) + ")";
    }
}
