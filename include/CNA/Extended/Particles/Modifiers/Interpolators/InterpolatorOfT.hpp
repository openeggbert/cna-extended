// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/Interpolators/InterpolatorOfT.cs
// (upstream's own file for the generic `Interpolator<T>` type). C++ disallows a class template and
// a non-template class sharing a name -- `Interpolator<T>` is renamed `InterpolatorOfT<T>` here,
// matching this project's established precedent (see `ComponentMapper<T>` -> `ComponentMapperOf<T>`
// in the ECS module).
#pragma once

#include "CNA/Extended/Particles/Modifiers/Interpolators/Interpolator.hpp"

#include <utility>

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    /** @brief Generic interpolator base holding the start/end values to interpolate between. */
    template <typename T>
    class InterpolatorOfT : public Interpolator
    {
    public:
        T StartValue{};
        T EndValue{};

    protected:
        explicit InterpolatorOfT(std::string name) : Interpolator(std::move(name)) {}
    };
}
