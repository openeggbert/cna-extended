// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Modifier3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::Modifier, brought to parity
// (2026-07-14, user-requested), with one deliberate behavioral deviation from the 2D 1:1
// mirror, found and resolved during implementation (not assumed):
//
// 2D's Modifier::InternalUpdate paces a full pass over the particle buffer across several
// frames according to Frequency, but -- critically -- the per-particle Update() call it
// dispatches to always receives `cycleTime_` (1/Frequency) as its elapsed-time argument, NOT
// the real elapsed time of the InternalUpdate() call that triggered it. That is a real,
// intentional 2D design choice (every particle receives a full cycle's worth of simulated
// time exactly once per cycle, spread across frames for performance), but it means a single
// large-elapsedSeconds call (e.g. a test calling UpdateEXT(0.5f, ...) as a convenience
// shortcut for "half a second passed", rather than many real small per-frame calls) would
// apply only `cycleTime_` (~0.0167s at the default 60Hz) of simulated change, not the full
// 0.5s -- a real, observable simulation-timing difference from ParticleEmitter3DEXT's
// existing (pre-this-architecture) behavior, where every active particle receives the exact
// real deltaSeconds of each UpdateEXT call, unconditionally, every call.
//
// This port's InternalUpdate keeps Frequency's *particle-count* pacing (how many of the
// total particles are touched this call) but passes the real elapsedSeconds through to
// Update() instead of `cycleTime_` -- whichever particles a given call does touch receive
// physically correct time deltas, rather than a stale fixed-size one. ParticleEmitter3DEXT's
// own built-in default modifiers (AgeModifier3DEXT, LinearGravityModifier3DEXT) additionally
// override Frequency to an effectively-unthrottled value specifically so the emitter's
// pre-existing "every particle, every call" behavior is preserved exactly (verified against
// the full pre-existing ParticleEmitter3DEXTTests.cpp/ParticleRenderSystem3DEXTTests.cpp
// suite, which needed zero behavioral changes for this reason) -- Frequency-based
// count-throttling remains available and correctly implemented for any other modifier that
// deliberately wants it for performance on very large particle counts.
#pragma once

#include <string>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    struct Particle3DEXT;

    /** @brief Base class for particle modifiers: alters particle properties over time, at a configurable update frequency. */
    class Modifier3DEXT
    {
    public:
        virtual ~Modifier3DEXT() = default;

        /** @brief Gets the display name of this modifier. Defaults to the concrete subclass's name. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        void setNameProperty(const std::string& value) { name_ = value; }

        /** @brief Gets the update frequency (times per second the modifier updates the whole particle list, on average). */
        [[nodiscard]] float getFrequencyProperty() const { return frequency_; }
        /** @throws System::ArgumentOutOfRangeException value is not greater than zero. */
        void setFrequencyProperty(float value);

        /** @brief Gets or sets whether this modifier is applied to particles. */
        [[nodiscard]] bool getEnabledProperty() const { return enabled_; }
        void setEnabledProperty(bool value) { enabled_ = value; }

        /** @brief Distributes an Update() call across frames according to Frequency; does nothing if disabled or the list is empty. */
        void InternalUpdate(float elapsedSeconds, std::vector<Particle3DEXT>& particles);

    protected:
        explicit Modifier3DEXT(std::string name);

        /** @brief Applies this modifier's specific behavior to @p particleCount particles starting at @p startIndex in @p particles. */
        virtual void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) = 0;

    private:
        static constexpr float DefaultModifierFrequency = 60.0f;

        std::string name_;
        float frequency_ = 0.0f;
        float cycleTime_ = 0.0f;
        int particlesUpdatedThisCycle_ = 0;
        bool enabled_ = true;
    };
}
