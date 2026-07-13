// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Properties/TilemapPropertyValue.cs (root
// `MonoGame.Extended.Tilemaps` namespace despite the Properties/ folder -- see
// TilemapPropertyType.hpp's header comment for the same precedent). Upstream's
// `[StructLayout(LayoutKind.Explicit)] ValueData` is a raw byte-overlapping union of
// int/float/bool/uint(color-packed-value), discriminated by a separate `Type` field -- not C#
// `object`/`dynamic` typing, so it translates directly to a plain C++ `union` with the same
// layout, rather than something heavier like `std::variant` (which would store a type tag
// redundant with `Type` and wouldn't match the explicit-overlap memory layout being ported).
#pragma once

#include "CNA/Extended/Tilemaps/TilemapPropertyType.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

#include <string>

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Color;

    /** @brief A tagged-union custom property value: string, int, float, bool, color, file path, or object-reference ID. */
    class TilemapPropertyValue
    {
    public:
        /** @brief Default-constructs an empty String-typed value (matches C#'s implicit `default(TilemapPropertyValue)`). */
        TilemapPropertyValue() = default;

        [[nodiscard]] static TilemapPropertyValue CreateString(std::string value);
        [[nodiscard]] static TilemapPropertyValue CreateInt(int value);
        [[nodiscard]] static TilemapPropertyValue CreateFloat(float value);
        [[nodiscard]] static TilemapPropertyValue CreateBool(bool value);
        [[nodiscard]] static TilemapPropertyValue CreateColor(const Color& value);
        [[nodiscard]] static TilemapPropertyValue CreateFile(std::string path);
        [[nodiscard]] static TilemapPropertyValue CreateObject(int objectId);

        /** @brief Gets the type of this property value. */
        [[nodiscard]] TilemapPropertyType getTypeProperty() const { return type_; }

        /** @throws System::InvalidOperationException getTypeProperty() is not TilemapPropertyType::String. */
        [[nodiscard]] const std::string& AsString() const;
        /** @throws System::InvalidOperationException getTypeProperty() is not TilemapPropertyType::Int. */
        [[nodiscard]] int AsInt() const;
        /** @throws System::InvalidOperationException getTypeProperty() is not TilemapPropertyType::Float. */
        [[nodiscard]] float AsFloat() const;
        /** @throws System::InvalidOperationException getTypeProperty() is not TilemapPropertyType::Bool. */
        [[nodiscard]] bool AsBool() const;
        /** @throws System::InvalidOperationException getTypeProperty() is not TilemapPropertyType::Color. */
        [[nodiscard]] Color AsColor() const;
        /** @throws System::InvalidOperationException getTypeProperty() is not TilemapPropertyType::File. */
        [[nodiscard]] const std::string& AsFile() const;
        /** @throws System::InvalidOperationException getTypeProperty() is not TilemapPropertyType::Object. */
        [[nodiscard]] int AsObject() const;

    private:
        union ValueData
        {
            int Int32;
            float Single;
            bool Boolean;
            SharpRuntime::UInt32 ColorPackedValue;

            ValueData() : Int32(0) {}
            explicit ValueData(int value) : Int32(value) {}
            explicit ValueData(float value) : Single(value) {}
            explicit ValueData(bool value) : Boolean(value) {}
            explicit ValueData(SharpRuntime::UInt32 value) : ColorPackedValue(value) {}
        };

        TilemapPropertyValue(TilemapPropertyType type, ValueData valueData, std::string stringValue = "");

        TilemapPropertyType type_ = TilemapPropertyType::String;
        ValueData valueData_;
        std::string stringValue_;
    };
}
