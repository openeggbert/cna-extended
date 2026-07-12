// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#pragma once

/// @file Version.hpp
/// @brief Compile-time and run-time version information for cna-extended.

#define CNA_EXTENDED_VERSION_MAJOR 0
#define CNA_EXTENDED_VERSION_MINOR 1
#define CNA_EXTENDED_VERSION_PATCH 0

namespace CNA::Extended
{
    /// @brief Major version component.
    inline constexpr int VersionMajor = CNA_EXTENDED_VERSION_MAJOR;
    /// @brief Minor version component.
    inline constexpr int VersionMinor = CNA_EXTENDED_VERSION_MINOR;
    /// @brief Patch version component.
    inline constexpr int VersionPatch = CNA_EXTENDED_VERSION_PATCH;

    /// @brief A single comparable integer, e.g. 0.1.0 -> 100.
    inline constexpr int VersionNumber() noexcept
    {
        return VersionMajor * 10000 + VersionMinor * 100 + VersionPatch;
    }

    /// @brief Human-readable version string, e.g. "0.1.0".
    /// @note Defined in Version.cpp (does not require linking CNA).
    const char* VersionString() noexcept;
}
