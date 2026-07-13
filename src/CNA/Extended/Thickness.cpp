// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Thickness.hpp"

#include <stdexcept>

namespace CNA::Extended
{
    Thickness::Thickness(const intcs all) : Thickness(all, all, all, all)
    {
    }

    Thickness::Thickness(const intcs leftRight, const intcs topBottom)
        : Thickness(leftRight, topBottom, leftRight, topBottom)
    {
    }

    Thickness::Thickness(const intcs left, const intcs top, const intcs right, const intcs bottom)
        : left_(left), top_(top), right_(right), bottom_(bottom)
    {
    }

    bool Thickness::Equals(const Thickness& other) const
    {
        return left_ == other.left_ && right_ == other.right_ && top_ == other.top_ && bottom_ == other.bottom_;
    }

    int Thickness::GetHashCode() const
    {
        // unchecked { hashCode = Left; hashCode = (hashCode*397) ^ Top; ... }
        int hashCode = static_cast<int>(left_);
        hashCode = (hashCode * 397) ^ static_cast<int>(top_);
        hashCode = (hashCode * 397) ^ static_cast<int>(right_);
        hashCode = (hashCode * 397) ^ static_cast<int>(bottom_);
        return hashCode;
    }

    Thickness Thickness::FromValues(const std::vector<intcs>& values)
    {
        switch (values.size())
        {
            case 1:
                return Thickness(values[0]);
            case 2:
                return Thickness(values[0], values[1]);
            case 4:
                return Thickness(values[0], values[1], values[2], values[3]);
            default:
                throw std::invalid_argument("Invalid thickness");
        }
    }

    Thickness Thickness::Parse(const std::string& value)
    {
        std::vector<intcs> values;
        std::string token;
        for (const char c : value)
        {
            if (c == ',' || c == ' ')
            {
                if (!token.empty())
                {
                    values.push_back(static_cast<intcs>(std::stoi(token)));
                    token.clear();
                }
            }
            else
            {
                token += c;
            }
        }
        if (!token.empty())
        {
            values.push_back(static_cast<intcs>(std::stoi(token)));
        }

        return FromValues(values);
    }

    void Thickness::Deconstruct(intcs& top, intcs& right, intcs& bottom, intcs& left) const
    {
        top = top_;
        right = right_;
        bottom = bottom_;
        left = left_;
    }

    std::string Thickness::ToString() const
    {
        if (left_ == right_ && top_ == bottom_)
        {
            if (left_ == top_)
            {
                return std::to_string(left_);
            }
            return std::to_string(left_) + " " + std::to_string(top_);
        }

        return std::to_string(left_) + ", " + std::to_string(right_) + ", " + std::to_string(top_) + ", " +
               std::to_string(bottom_);
    }
}
