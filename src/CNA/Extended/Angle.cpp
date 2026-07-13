// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
// Portions based on the SlimMath project (MIT License, Copyright (c) 2007-2010 SlimDX Group)
#include "CNA/Extended/Angle.hpp"

#include <cmath>
#include <numbers>

namespace CNA::Extended
{
    Angle::Angle(const float value, const AngleType angleType)
    {
        switch (angleType)
        {
            default:
                radians_ = 0.0f;
                break;
            case AngleType::Radian:
                radians_ = value;
                break;
            case AngleType::Degree:
                radians_ = value * kDegreeRadian;
                break;
            case AngleType::Revolution:
                radians_ = value * kTau;
                break;
            case AngleType::Gradian:
                radians_ = value * kGradianRadian;
                break;
        }
    }

    float Angle::getDegreesProperty() const
    {
        return radians_ * kRadianDegree;
    }

    void Angle::setDegreesProperty(const float value)
    {
        radians_ = value * kDegreeRadian;
    }

    float Angle::getGradiansProperty() const
    {
        return radians_ * kRadianGradian;
    }

    void Angle::setGradiansProperty(const float value)
    {
        radians_ = value * kGradianRadian;
    }

    float Angle::getRevolutionsProperty() const
    {
        return radians_ * kTauInv;
    }

    void Angle::setRevolutionsProperty(const float value)
    {
        radians_ = value * kTau;
    }

    float Angle::GetValue(const AngleType angleType) const
    {
        switch (angleType)
        {
            default:
                return 0.0f;
            case AngleType::Radian:
                return radians_;
            case AngleType::Degree:
                return getDegreesProperty();
            case AngleType::Revolution:
                return getRevolutionsProperty();
            case AngleType::Gradian:
                return getGradiansProperty();
        }
    }

    void Angle::Wrap()
    {
        float angle = std::fmod(radians_, kTau);
        if (angle <= std::numbers::pi_v<float>)
        {
            angle += kTau;
        }
        if (angle > std::numbers::pi_v<float>)
        {
            angle -= kTau;
        }
        radians_ = angle;
    }

    void Angle::WrapPositive()
    {
        radians_ = std::fmod(radians_, kTau);
        if (radians_ < 0.0f)
        {
            radians_ += kTau;
        }
    }

    Angle Angle::FromVector(const Vector2& vector)
    {
        return Angle(std::atan2(-vector.Y, vector.X));
    }

    Vector2 Angle::ToUnitVector() const
    {
        return ToVector(1.0f);
    }

    Vector2 Angle::ToVector(const float length) const
    {
        return Vector2(length * std::cos(radians_), -length * std::sin(radians_));
    }

    bool Angle::IsBetween(const Angle& value, const Angle& min, const Angle& end)
    {
        return end.radians_ < min.radians_ ? (value.radians_ >= min.radians_ || value.radians_ <= end.radians_)
                                            : (value.radians_ >= min.radians_ && value.radians_ <= end.radians_);
    }

    int Angle::CompareTo(Angle other)
    {
        WrapPositive();
        other.WrapPositive();
        if (radians_ < other.radians_) return -1;
        if (radians_ > other.radians_) return 1;
        return 0;
    }

    bool Angle::Equals(Angle other)
    {
        WrapPositive();
        other.WrapPositive();
        return radians_ == other.radians_;
    }

    int Angle::GetHashCode() const
    {
        // Deliberately does NOT call WrapPositive() first -- matches upstream exactly (see the
        // file-level fidelity note in Angle.hpp).
        return static_cast<int>(std::hash<float>{}(radians_));
    }

    std::string Angle::ToString() const
    {
        return std::to_string(radians_) + " Radians";
    }

    Angle Angle::operator-() const
    {
        return Angle(-radians_);
    }

    Angle operator-(const Angle& left, const Angle& right)
    {
        return Angle(left.radians_ - right.radians_);
    }

    Angle operator*(const Angle& left, const float right)
    {
        return Angle(left.radians_ * right);
    }

    Angle operator*(const float left, const Angle& right)
    {
        return Angle(right.radians_ * left);
    }

    Angle operator+(const Angle& left, const Angle& right)
    {
        return Angle(left.radians_ + right.radians_);
    }
}
