// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/Interpolators/Interpolator.cs. Same
// reflection-in-constructor hazard as Modifier.cs (`Name = GetType().Name`) -- same fix: the
// constructor takes a `name` parameter, passed up from each concrete subclass. See Modifier.hpp's
// header comment for the full rationale.
#pragma once

#include <string>

namespace CNA::Extended::Particles::Data
{
    struct Particle;
}

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    /** @brief Base class for particle interpolators: computes a target property value from a normalized [0,1] age fraction. */
    class Interpolator
    {
    public:
        virtual ~Interpolator() = default;

        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        void setNameProperty(const std::string& value) { name_ = value; }

        [[nodiscard]] bool getEnabledProperty() const { return enabled_; }
        void setEnabledProperty(bool value) { enabled_ = value; }

        /** @brief Updates @p particle's property for interpolation @p amount (particle age as a fraction of its lifespan, 0..1). */
        virtual void Update(float amount, Data::Particle* particle) = 0;

    protected:
        explicit Interpolator(std::string name) : name_(std::move(name)) {}

    private:
        std::string name_;
        bool enabled_ = true;
    };
}
