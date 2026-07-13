// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collisions/UndefinedLayerException.hpp"

#include "System/ArgumentException.hpp"

namespace CNA::Extended::Collisions
{
    UndefinedLayerException::UndefinedLayerException(const std::string& layerName)
        : System::Exception(CreateMessage(layerName)), layerName_(layerName)
    {
    }

    std::string UndefinedLayerException::CreateMessage(const std::string& layerName)
    {
        System::ArgumentException::ThrowIfNullOrWhiteSpace(layerName);
        return "Layer '" + layerName + "' is not defined.";
    }
}
