// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/Layers/UndefinedLayerException.cs. Upstream's
// `Exception` base -> sharp-runtime's `System::Exception` (itself derived from `std::exception`),
// matching this project's established convention of reusing sharp-runtime's BCL types rather than
// deriving from std::exception directly. Flattened into CNA::Extended::Collisions, matching the
// same Layers-namespace flattening decision as Layer.hpp.
#pragma once

#include "System/Exception.hpp"

#include <string>

namespace CNA::Extended::Collisions
{
    /** @brief Thrown when a collision layer name is not registered in a collision world. */
    class UndefinedLayerException : public System::Exception
    {
    public:
        /**
         * @brief Initializes a new UndefinedLayerException for the specified unresolved layer name.
         * @param layerName The layer name that could not be resolved. Must not be null, empty, or whitespace.
         */
        explicit UndefinedLayerException(const std::string& layerName);

        /** @brief Gets the layer name that could not be resolved. */
        [[nodiscard]] const std::string& getLayerNameProperty() const { return layerName_; }

    private:
        static std::string CreateMessage(const std::string& layerName);

        std::string layerName_;
    };
}
