// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Properties/TilemapProperties.cs (root
// `MonoGame.Extended.Tilemaps` namespace -- see TilemapPropertyType.hpp's header comment).
// `IEnumerable<KeyValuePair<string, TilemapPropertyValue>>` -> begin()/end() forwarding to the
// underlying `System::Collections::Generic::Dictionary`'s own iterators, matching this project's
// established range-for-support convention for Dictionary-backed types. `operator[]`'s return
// type is `Dictionary<...>::ValueProxy` (via `auto`): implicit-converts on read (throws
// `KeyNotFoundException` if absent, matching upstream's indexer getter) and assigns-to-insert-or-
// update on write, exactly mirroring upstream's split get/set indexer.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapPropertyValue.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "System/Collections/Generic/Dictionary.hpp"

#include <optional>
#include <string>

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Color;

    /** @brief A named collection of custom TilemapPropertyValue entries attached to a tilemap element. */
    class TilemapProperties
    {
    public:
        TilemapProperties() = default;

        /** @brief Gets or sets the property value for @p key. @throws System::Collections::Generic::KeyNotFoundException on read of a missing key. */
        [[nodiscard]] auto operator[](const std::string& key) { return properties_[key]; }

        /** @brief Gets the number of properties in this collection. */
        [[nodiscard]] int getCountProperty() const { return properties_.getCountProperty(); }

        /** @brief Attempts to get the value associated with @p key. Returns false, leaving @p value unset, if not found. */
        bool TryGetValue(const std::string& key, TilemapPropertyValue& value) const { return properties_.TryGetValue(key, value); }

        /** @brief Gets a String property, or @p defaultValue if not found or a different type. */
        [[nodiscard]] std::string GetString(const std::string& key, const std::string& defaultValue = "") const;
        /** @brief Gets an Int property, or @p defaultValue if not found or a different type. */
        [[nodiscard]] int GetInt(const std::string& key, int defaultValue = 0) const;
        /** @brief Gets a Float property, or @p defaultValue if not found or a different type. */
        [[nodiscard]] float GetFloat(const std::string& key, float defaultValue = 0.0f) const;
        /** @brief Gets a Bool property, or @p defaultValue if not found or a different type. */
        [[nodiscard]] bool GetBool(const std::string& key, bool defaultValue = false) const;
        /** @brief Gets a Color property, or @p defaultValue (Color::White if not specified) if not found or a different type. */
        [[nodiscard]] Color GetColor(const std::string& key, const std::optional<Color>& defaultValue = std::nullopt) const;

        /** @brief Sets a String property. */
        void SetString(const std::string& key, const std::string& value) { properties_[key] = TilemapPropertyValue::CreateString(value); }
        /** @brief Sets an Int property. */
        void SetInt(const std::string& key, int value) { properties_[key] = TilemapPropertyValue::CreateInt(value); }
        /** @brief Sets a Float property. */
        void SetFloat(const std::string& key, float value) { properties_[key] = TilemapPropertyValue::CreateFloat(value); }
        /** @brief Sets a Bool property. */
        void SetBool(const std::string& key, bool value) { properties_[key] = TilemapPropertyValue::CreateBool(value); }
        /** @brief Sets a Color property. */
        void SetColor(const std::string& key, const Color& value) { properties_[key] = TilemapPropertyValue::CreateColor(value); }

        [[nodiscard]] auto begin() { return properties_.begin(); }
        [[nodiscard]] auto end() { return properties_.end(); }
        [[nodiscard]] auto begin() const { return properties_.begin(); }
        [[nodiscard]] auto end() const { return properties_.end(); }

    private:
        System::Collections::Generic::Dictionary<std::string, TilemapPropertyValue> properties_;
    };
}
