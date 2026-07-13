// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Profiles/Profile.cs. Static factory methods (Point(),
// Line(), etc.) each construct a fresh instance in upstream (a C# reference type) -> return
// std::unique_ptr<Profile> here, matching that "new instance per call" ownership. Upstream's
// `ToString() => GetType().Name` (reflection) is translated as a plain virtual `ToString()`
// overridden per concrete subclass with its own literal class-name string -- safe here (unlike
// Modifier's constructor-time reflection) since it's only ever called after construction completes.
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <memory>
#include <string>

namespace CNA::Extended::Particles::Profiles
{
    using Microsoft::Xna::Framework::Vector2;

    enum class CircleRadiation;
    enum class LineRadiation;

    /** @brief Base class for particle emission profiles: computes the initial offset and heading for each newly released particle. */
    class Profile
    {
    public:
        virtual ~Profile() = default;

        /** @brief Computes the offset (from the emitter position) and unit heading for a new particle, writing into @p offset/@p heading. */
        virtual void GetOffsetAndHeading(Vector2* offset, Vector2* heading) = 0;

        [[nodiscard]] virtual std::string ToString() const = 0;

        /** @brief Creates a profile emitting all particles from a single point with random headings. */
        [[nodiscard]] static std::unique_ptr<Profile> Point();

        /** @brief Creates a profile emitting particles uniformly along a line segment with random headings. */
        [[nodiscard]] static std::unique_ptr<Profile> Line(const Vector2& axis, float length);

        /** @brief Creates a profile emitting particles along a line segment with the given radiation mode. */
        [[nodiscard]] static std::unique_ptr<Profile> Line(const Vector2& axis, float length, LineRadiation radiate);

        /** @brief Creates a profile emitting particles along a line segment with the given radiation mode and direction. */
        [[nodiscard]] static std::unique_ptr<Profile> Line(
            const Vector2& axis, float length, LineRadiation radiate, const Vector2& direction);

        /** @brief Creates a profile emitting particles from the perimeter of a circle. */
        [[nodiscard]] static std::unique_ptr<Profile> Ring(float radius, CircleRadiation radiate);

        /** @brief Creates a profile emitting particles from the perimeter of a rectangle (equal probability per side). */
        [[nodiscard]] static std::unique_ptr<Profile> Box(float width, float height);

        /** @brief Creates a profile emitting particles from within a rectangular area. */
        [[nodiscard]] static std::unique_ptr<Profile> BoxFill(float width, float height);

        /** @brief Creates a profile emitting particles from the perimeter of a rectangle with uniform density per unit length. */
        [[nodiscard]] static std::unique_ptr<Profile> BoxUniform(float width, float height);

        /** @brief Creates a profile emitting particles from within a circular area. */
        [[nodiscard]] static std::unique_ptr<Profile> Circle(float radius, CircleRadiation radiate);

        /** @brief Creates a profile emitting particles from a point in a directional cone. */
        [[nodiscard]] static std::unique_ptr<Profile> Spray(const Vector2& direction, float spread);
    };
}
