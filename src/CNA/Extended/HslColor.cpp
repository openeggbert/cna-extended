// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/HslColor.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace CNA::Extended
{
    float HslColor::NormalizeHue(const float h)
    {
        if (h < 0)
        {
            return h + 360.0f * (static_cast<float>(static_cast<int>(h / 360.0f)) + 1.0f);
        }
        return std::fmod(h, 360.0f);
    }

    HslColor::HslColor(const float h, const float s, const float l)
        : h_(std::clamp(h, 0.0f, 360.0f)), s_(std::clamp(s, 0.0f, 1.0f)), l_(std::clamp(l, 0.0f, 1.0f))
    {
    }

    void HslColor::CopyToRef(HslColor& destination) const
    {
        destination.h_ = h_;
        destination.s_ = s_;
        destination.l_ = l_;
    }

    void HslColor::Deconstruct(float& h, float& s, float& l) const
    {
        h = h_;
        s = s_;
        l = l_;
    }

    void HslColor::Match(const std::function<void(float, float, float)>& callback) const
    {
        if (!callback)
        {
            throw std::invalid_argument("callback");
        }
        callback(h_, s_, l_);
    }

    int HslColor::CompareTo(const HslColor& other) const
    {
        if (h_ != other.h_)
        {
            return h_ < other.h_ ? -1 : 1;
        }

        if (l_ != other.l_)
        {
            return l_ < other.l_ ? -1 : 1;
        }

        if (s_ != other.s_)
        {
            return s_ < other.s_ ? -1 : 1;
        }

        return 0;
    }

    bool HslColor::Equals(const HslColor& value) const
    {
        return h_ == value.h_ && l_ == value.l_ && s_ == value.s_;
    }

    int HslColor::GetHashCode() const
    {
        return static_cast<int>(std::hash<float>{}(h_)) ^ static_cast<int>(std::hash<float>{}(s_)) ^
               static_cast<int>(std::hash<float>{}(l_));
    }

    std::string HslColor::ToString() const
    {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "H:%.1f\xC2\xB0 S:%.1f L:%.1f", static_cast<double>(h_),
            static_cast<double>(100.0f * s_), static_cast<double>(100.0f * l_));
        return std::string(buffer);
    }

    HslColor HslColor::Parse(const std::string& s)
    {
        std::vector<std::string> parts;
        std::stringstream stream(s);
        std::string part;
        while (std::getline(stream, part, ','))
        {
            parts.push_back(part);
        }

        auto trimDegree = [](std::string value) {
            while (!value.empty() && (value.back() == '\xC2' || value.back() == '\xB0'))
            {
                // Strip a trailing UTF-8 '°' (0xC2 0xB0) one byte at a time.
                value.pop_back();
            }
            return value;
        };

        const float hue = std::stof(trimDegree(parts.at(0)));
        const float sat = std::stof(parts.at(1));
        const float lig = std::stof(parts.at(2));

        return HslColor(hue, sat, lig);
    }

    HslColor operator+(const HslColor& a, const HslColor& b)
    {
        return HslColor(a.h_ + b.h_, a.s_ + b.s_, a.l_ + b.l_);
    }

    HslColor operator-(const HslColor& a, const HslColor& b)
    {
        return HslColor(a.h_ - b.h_, a.s_ - b.s_, a.l_ - b.l_);
    }

    HslColor HslColor::Lerp(const HslColor& c1, const HslColor& c2, const float t)
    {
        // loop around if c2.H < c1.H
        const float h2 = c2.h_ >= c1.h_ ? c2.h_ : c2.h_ + 360.0f;
        return HslColor(c1.h_ + t * (h2 - c1.h_), c1.s_ + t * (c2.s_ - c1.s_), c1.l_ + t * (c2.l_ - c1.l_));
    }

    float HslColor::RgbFromHue(const float min, const float max, float hue)
    {
        hue = std::fmod(hue + 1.0f, 1.0f);

        if (hue * 6.0f < 1.0f)
        {
            return min + (max - min) * 6.0f * hue;
        }

        if (hue * 2.0f < 1.0f)
        {
            return max;
        }

        if (hue * 3.0f < 2.0f)
        {
            return min + (max - min) * (2.0f / 3.0f - hue) * 6.0f;
        }

        return min;
    }

    Color HslColor::ToRgb(const HslColor& hsl)
    {
        float h = hsl.h_;
        const float s = hsl.s_;
        const float l = hsl.l_;

        // MonoGame.Extended.MathExtended.MachineEpsilon (not yet ported; Phase 1, "MathExtended,
        // FloatHelper, Angle") is exactly the standard IEEE-754 float epsilon.
        const float machineEpsilon = std::numeric_limits<float>::epsilon();

        if (s < machineEpsilon)
        {
            return Color(l, l, l);
        }

        if (l <= machineEpsilon)
        {
            return Color::Black;
        }

        h /= 360.0f;

        const float max = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        const float min = 2.0f * l - max;

        const float r = RgbFromHue(min, max, h + 0.3333333f);
        const float g = RgbFromHue(min, max, h);
        const float b = RgbFromHue(min, max, h - 0.3333333f);

        return Color(r, g, b);
    }

    HslColor HslColor::FromRgb(const Color& color)
    {
        const float r = static_cast<float>(color.getRProperty()) / 255.0f;
        const float g = static_cast<float>(color.getGProperty()) / 255.0f;
        const float b = static_cast<float>(color.getBProperty()) / 255.0f;

        const float max = std::max(r, std::max(g, b));
        const float min = std::min(r, std::min(g, b));
        const float delta = max - min;

        float h = 0.0f;
        float s = 0.0f;
        const float l = (max + min) * 0.5f;

        if (std::abs(delta) < std::numeric_limits<float>::epsilon())
        {
            return HslColor(h, s, l);
        }

        if (std::abs(r - max) < std::numeric_limits<float>::epsilon())
        {
            h = (g - b) / delta;
        }
        else if (std::abs(g - max) < std::numeric_limits<float>::epsilon())
        {
            h = (b - r) / delta + 2.0f;
        }
        else if (std::abs(b - max) < std::numeric_limits<float>::epsilon())
        {
            h = (r - g) / delta + 4.0f;
        }

        h *= 60.0f;
        h = NormalizeHue(h);

        if (l <= 0.5f)
        {
            s = delta / (max + min);
        }
        else
        {
            s = delta / (2.0f - max - min);
        }

        return HslColor(h, s, l);
    }
}
