// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::InterpolatorOfT3DEXT<T> -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::Interpolators::InterpolatorOfT<T>,
// same shape (2026-07-14, user-requested parity work) -- a class template and a non-template
// class cannot share a name in C++, matching the same InterpolatorOfT naming precedent 2D
// already established (see that file's own header comment for the full rationale, itself
// following ComponentMapper -> ComponentMapperOf in the ECS module).
#pragma once

#include "CNA/Extended/World3DEXT/Interpolator3DEXT.hpp"

#include <utility>

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Generic interpolator base holding the start/end values to interpolate between.
     * @note StartValue/EndValue have no in-class default initializer (unlike 2D's
     * InterpolatorOfT<T>, where `T StartValue;`/`EndValue` rely on T being default-
     * constructible) -- Microsoft::Xna::Framework::Color (used by ColorInterpolator3DEXT) has
     * no zero-argument constructor, so the protected constructor takes explicit initial
     * values instead, and every concrete subclass (including float-valued ones like
     * OpacityInterpolator3DEXT, where default construction would otherwise have been fine)
     * passes them explicitly for consistency.
     */
    template <typename T>
    class InterpolatorOfT3DEXT : public Interpolator3DEXT
    {
    public:
        T StartValue;
        T EndValue;

    protected:
        InterpolatorOfT3DEXT(std::string name, T startValue, T endValue)
            : Interpolator3DEXT(std::move(name)), StartValue(std::move(startValue)), EndValue(std::move(endValue))
        {
        }
    };
}
