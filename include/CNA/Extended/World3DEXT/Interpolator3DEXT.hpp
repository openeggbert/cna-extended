// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Interpolator3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::Interpolators::Interpolator, same
// shape (2026-07-14, user-requested parity work). Header-only, matching the 2D original's
// own header-only precedent (no .cpp needed for a pure-virtual base with only inline
// members).
#pragma once

#include <string>

namespace CNA::Extended::World3DEXT
{
    struct Particle3DEXT;

    /** @brief Base class for particle interpolators: computes a target property value from a normalized [0,1] age fraction. */
    class Interpolator3DEXT
    {
    public:
        virtual ~Interpolator3DEXT() = default;

        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        void setNameProperty(const std::string& value) { name_ = value; }

        [[nodiscard]] bool getEnabledProperty() const { return enabled_; }
        void setEnabledProperty(bool value) { enabled_ = value; }

        /** @brief Updates @p particle's property for interpolation @p amount (particle age as a fraction of its lifetime, 0..1 -- see Particle3DEXT::GetAgeRatioEXT()). */
        virtual void Update(float amount, Particle3DEXT* particle) = 0;

    protected:
        explicit Interpolator3DEXT(std::string name) : name_(std::move(name)) {}

    private:
        std::string name_;
        bool enabled_ = true;
    };
}
