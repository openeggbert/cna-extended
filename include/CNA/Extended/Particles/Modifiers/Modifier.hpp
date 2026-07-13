// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/Modifier.cs. Upstream's base constructor
// sets `Name = GetType().Name` via reflection -- calling a virtual/type-dependent lookup from a
// base constructor is safe in C# (GC-backed, dispatches to the most-derived override even before
// the derived constructor body runs) but UB in C++ (vtable is still the base class's during base
// construction; see this project's established GameTimer precedent for the same hazard). Translated
// by having the base Modifier constructor take a `name` parameter, and each concrete subclass pass
// its own class name up via its base-class initializer -- a direct constructor argument replacing
// a reflective default, not a simplification of behavior.
#pragma once

#include <string>

namespace CNA::Extended::Particles
{
    class ParticleIterator;
}

namespace CNA::Extended::Particles::Modifiers
{
    /** @brief Base class for particle modifiers: alters particle properties over time, at a configurable update frequency. */
    class Modifier
    {
    public:
        virtual ~Modifier() = default;

        /** @brief Gets the display name of this modifier. Defaults to the concrete subclass's name. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        void setNameProperty(const std::string& value) { name_ = value; }

        /** @brief Gets the update frequency (times per second the modifier updates the whole particle buffer, on average). */
        [[nodiscard]] float getFrequencyProperty() const { return frequency_; }
        /** @throws System::ArgumentOutOfRangeException value is not greater than zero. */
        void setFrequencyProperty(float value);

        /** @brief Gets or sets whether this modifier is applied to particles. */
        [[nodiscard]] bool getEnabledProperty() const { return enabled_; }
        void setEnabledProperty(bool value) { enabled_ = value; }

        /** @brief Distributes an Update() call across frames according to Frequency; does nothing if disabled or the buffer is empty. */
        void InternalUpdate(float elapsedSeconds, ParticleIterator& iterator);

    protected:
        explicit Modifier(std::string name);

        /** @brief Applies this modifier's specific behavior to @p particleCount particles starting at @p iterator's current position. */
        virtual void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) = 0;

    private:
        static constexpr float DefaultModifierFrequency = 60.0f;

        std::string name_;
        float frequency_ = 0.0f;
        float cycleTime_ = 0.0f;
        int particlesUpdatedThisCycle_ = 0;
        bool enabled_ = true;
    };
}
