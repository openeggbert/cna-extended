// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/Effects/ITextureEffect.cs. Upstream lives in the
// MonoGame.Extended.Graphics.Effects namespace; ported into a matching
// CNA::Extended::Graphics::Effects sub-namespace. `Texture2D Texture { get; set; }` (a nullable
// reference-type property) -> a non-owning `Texture2D*` pair, matching this project's established
// convention for nullable reference-type C# properties.
#pragma once

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::Graphics::Effects
{
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    /** @brief Defines an Effect that uses a Texture2D. */
    class ITextureEffect
    {
    public:
        virtual ~ITextureEffect() = default;

        /** @brief Gets the bound texture, or nullptr if none is set. */
        [[nodiscard]] virtual Texture2D* getTextureProperty() const = 0;

        /** @brief Sets the bound texture. */
        virtual void setTextureProperty(Texture2D* value) = 0;
    };
}
