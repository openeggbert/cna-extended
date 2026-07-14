// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::HueInterpolator3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Not a literal 1:1 port of CNA::Extended::Particles::Modifiers::Interpolators::
// HueInterpolator, which writes directly to Data::Particle::Color[0] (hue) since 2D's
// Particle stores color natively as HSL -- no conversion needed. Particle3DEXT::ColorEXT is
// plain RGB (see ColorInterpolator3DEXT.hpp's header comment for why this architecture
// doesn't store HSL), so this class converts the particle's *current* color to HSL via
// CNA::Extended::HslColor::FromRgb (reusing that existing utility rather than re-deriving
// RGB<->HSL conversion), overwrites only the H channel with the interpolated value, and
// converts back with HslColor::ToRgb -- preserving the current Saturation/Lightness exactly
// as 2D's own "hue-only" semantic intends, just via a round-trip conversion instead of a
// native HSL field. A small CNA::Extended::World3DEXT -> CNA::Extended (top-level, not
// Particles-specific) dependency, matching Phase 11 A's UndefinedLayerException-reuse
// precedent for utility types with no 2D-specific logic worth duplicating.
#pragma once

#include "CNA/Extended/World3DEXT/InterpolatorOfT3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Interpolates only a particle's hue between StartValue and EndValue (0..1), preserving its current saturation/lightness. */
    class HueInterpolator3DEXT final : public InterpolatorOfT3DEXT<float>
    {
    public:
        HueInterpolator3DEXT() : InterpolatorOfT3DEXT("HueInterpolator3DEXT", 0.0f, 0.0f) {}

        void Update(float amount, Particle3DEXT* particle) override;
    };
}
