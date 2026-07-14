// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/SpriteSheet.cs. `Texture2DAtlas TextureAtlas { get; }`
// is a non-owning `Texture2DAtlas&` (stored internally as a pointer): the constructor takes a
// reference, so upstream's `ArgumentNullException.ThrowIfNull(textureAtlas)` is dropped (null is
// unrepresentable for a C++ reference parameter), matching ActorPairKey.hpp's precedent for
// reference-type parameters elsewhere in this port. `Action<SpriteSheetAnimationBuilder>
// buildAction` -> `std::function<void(SpriteSheetAnimationBuilder&)>`.
#pragma once

#include "CNA/Extended/Graphics/SpriteSheetAnimation.hpp"
#include "System/Collections/Generic/Dictionary.hpp"

#include <functional>
#include <memory>
#include <string>

namespace CNA::Extended::Graphics
{
    class Texture2DAtlas;
    class Sprite;
    class SpriteSheetAnimationBuilder;

    /**
     * @brief A texture atlas plus a set of named frame-index animation definitions built over it
     * -- the usual way to describe a character/effect's frame-by-frame animations (e.g. "walk",
     * "idle", "attack") once and then play any of them back through an AnimatedSprite. A
     * SpriteSheet does not own its Texture2DAtlas; the atlas must outlive it.
     *
     * @see Texture2DAtlas, the region source this sheet's animations are defined against.
     * @see SpriteSheetAnimationBuilder, the fluent builder passed to DefineAnimation's callback
     * for specifying frame order/duration/looping/ping-pong.
     * @see AnimatedSprite, which plays back a named animation defined here.
     */
    class SpriteSheet
    {
    public:
        SpriteSheet(const std::string& name, Texture2DAtlas& textureAtlas);

        /** @brief Gets the number of animations defined in the sprite sheet. */
        [[nodiscard]] int getAnimationCountProperty() const;

        /** @brief Gets the name of the sprite sheet. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }

        /** @brief Gets the texture atlas associated with the sprite sheet. */
        [[nodiscard]] Texture2DAtlas& getTextureAtlasProperty() const { return *textureAtlas_; }

        /**
         * @brief Creates a sprite from the specified region index.
         * @throws std::out_of_range @p regionIndex is out of range.
         */
        [[nodiscard]] Sprite CreateSprite(int regionIndex) const;

        /**
         * @brief Creates a sprite from the specified region name.
         * @throws System::Collections::Generic::KeyNotFoundException no region named @p regionName exists.
         */
        [[nodiscard]] Sprite CreateSprite(const std::string& regionName) const;

        /** @brief Defines a new animation for the sprite sheet, built via @p buildAction. */
        void DefineAnimation(const std::string& name, const std::function<void(SpriteSheetAnimationBuilder&)>& buildAction);

        /**
         * @throws System::Collections::Generic::KeyNotFoundException no animation named @p name exists.
         */
        [[nodiscard]] std::shared_ptr<SpriteSheetAnimation> GetAnimation(const std::string& name) const { return animations_[name]; }

        [[nodiscard]] bool TryGetAnimation(const std::string& name, std::shared_ptr<SpriteSheetAnimation>& animation) const
        {
            return animations_.TryGetValue(name, animation);
        }

        /**
         * @brief Removes the animation definition with the specified name.
         * @return true if the animation was removed; false if no animation with that name existed.
         */
        bool RemoveAnimationDefinition(const std::string& name) { return animations_.Remove(name); }

    private:
        Texture2DAtlas* textureAtlas_;
        std::string name_;
        System::Collections::Generic::Dictionary<std::string, std::shared_ptr<SpriteSheetAnimation>> animations_;
    };
}
