// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/InputListenerSettings.cs. Upstream's
// `where T : InputListener` generic constraint would normally -> `static_assert(std::is_base_of_v<
// InputListener, T>)`, matching this project's established generic-constraint-to-static_assert
// convention (see ObjectPool<T>) -- but unlike ObjectPool<T>, every concrete settings/listener
// pair here (KeyboardListenerSettings/KeyboardListener, etc.) has a genuine circular reference:
// XListenerSettings.hpp only forward-declares XListener (to avoid a real header cycle), so XListener
// is still an INCOMPLETE type at the point XListenerSettings : InputListenerSettings<XListener> is
// declared -- std::is_base_of_v requires a complete type, so a static_assert here would fail to
// compile at every single derivation site, not just catch a genuine misuse. The constraint is
// therefore not enforced at compile time in this port; T's InputListener-derivedness is
// guaranteed structurally instead (every XListenerSettings in this module is only ever declared
// deriving InputListenerSettings<X> for the matching XListener it was written for).
// Upstream's `T CreateListener()` returns a listener BY VALUE (a reference type in
// C#, so really "returns a new heap object"); ported returning `std::unique_ptr<T>`, since C++
// abstract InputListener-derived types cannot be returned by value and ownership must transfer to
// the caller. Header-only: a template, matching this project's established convention for generic
// C# types.
#pragma once

#include "CNA/Extended/Input/InputListeners/InputListener.hpp"

#include <memory>

namespace CNA::Extended::Input::InputListeners
{
    /**
     * @brief Abstract base for a settings/factory object that creates a configured InputListener.
     * @tparam T The concrete InputListener type this settings object creates.
     */
    template <typename T>
    class InputListenerSettings
    {
    public:
        virtual ~InputListenerSettings() = default;

        /** @brief Creates a new listener configured from this settings object. */
        [[nodiscard]] virtual std::unique_ptr<T> CreateListener() const = 0;

    protected:
        InputListenerSettings() = default;
    };
}
