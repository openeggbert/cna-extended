// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Parsers/TilemapParseException.cs. Upstream declares
// the `MonoGame.Extended.Tilemaps.Parsers` namespace (unlike most of this module, which stays in
// the plain `Tilemaps` namespace) -- ported into a matching `CNA::Extended::Tilemaps::Parsers`
// sub-namespace and file path. `int? LineNumber`/`ColumnNumber` (nullable) -> `std::optional<int>`.
#pragma once

#include "System/Exception.hpp"

#include <optional>
#include <string>

namespace CNA::Extended::Tilemaps::Parsers
{
    /** @brief Thrown when a tilemap file cannot be parsed. */
    class TilemapParseException : public System::Exception
    {
    public:
        /** @brief Creates the exception with just a @p message. */
        explicit TilemapParseException(const std::string& message);

        /** @brief Creates the exception with a @p message and an @p innerException that caused it. */
        TilemapParseException(const std::string& message, std::exception_ptr innerException);

        /** @brief Creates the exception with a @p message and error location, formatting them into the exception's own message (e.g. "path(line,column): message"). */
        explicit TilemapParseException(const std::string& message, const std::optional<std::string>& filePath,
            const std::optional<int>& lineNumber = std::nullopt, const std::optional<int>& columnNumber = std::nullopt);

        /** @brief Gets the file path where the error occurred, if available. */
        [[nodiscard]] const std::optional<std::string>& getFilePathProperty() const { return filePath_; }

        /** @brief Gets the line number where the error occurred, if available. */
        [[nodiscard]] const std::optional<int>& getLineNumberProperty() const { return lineNumber_; }

        /** @brief Gets the column number where the error occurred, if available. */
        [[nodiscard]] const std::optional<int>& getColumnNumberProperty() const { return columnNumber_; }

    private:
        [[nodiscard]] static std::string FormatMessage(
            const std::string& message, const std::optional<std::string>& filePath, const std::optional<int>& lineNumber,
            const std::optional<int>& columnNumber);

        std::optional<std::string> filePath_;
        std::optional<int> lineNumber_;
        std::optional<int> columnNumber_;
    };
}
