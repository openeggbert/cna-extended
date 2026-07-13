// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/Parsers/TilemapParseException.hpp"

namespace CNA::Extended::Tilemaps::Parsers
{
    TilemapParseException::TilemapParseException(const std::string& message) : System::Exception(message)
    {
    }

    TilemapParseException::TilemapParseException(const std::string& message, std::exception_ptr innerException)
        : System::Exception(message, std::move(innerException))
    {
    }

    TilemapParseException::TilemapParseException(
        const std::string& message, const std::optional<std::string>& filePath, const std::optional<int>& lineNumber,
        const std::optional<int>& columnNumber)
        : System::Exception(FormatMessage(message, filePath, lineNumber, columnNumber)),
          filePath_(filePath),
          lineNumber_(lineNumber),
          columnNumber_(columnNumber)
    {
    }

    std::string TilemapParseException::FormatMessage(
        const std::string& message, const std::optional<std::string>& filePath, const std::optional<int>& lineNumber,
        const std::optional<int>& columnNumber)
    {
        if (!filePath.has_value())
        {
            return message;
        }

        if (!lineNumber.has_value())
        {
            return *filePath + ": " + message;
        }

        if (columnNumber.has_value())
        {
            return *filePath + "(" + std::to_string(*lineNumber) + "," + std::to_string(*columnNumber) + "): " + message;
        }

        return *filePath + "(" + std::to_string(*lineNumber) + "): " + message;
    }
}
