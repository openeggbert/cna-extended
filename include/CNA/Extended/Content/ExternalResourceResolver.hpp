// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Content/ExternalResourceResolver.cs. Upstream lives in the
// MonoGame.Extended.Content namespace; ported into a matching CNA::Extended::Content
// sub-namespace. A named C# delegate type -> a `using` alias for the equivalent
// std::function signature, matching this project's established Func/Action-to-std::function
// convention (a named alias here, rather than an anonymous std::function everywhere it's used,
// preserves upstream's own "this delegate type has a name and meaning" intent).
#pragma once

#include "System/IO/Stream.hpp"

#include <functional>
#include <memory>
#include <string>

namespace CNA::Extended::Content
{
    /**
     * @brief Opens a stream for an external resource referenced by another asset.
     * @param path The resource path requested by the asset loader. May be absolute or relative,
     * depending on the loader and resolver being used.
     * @return A readable stream positioned at the beginning of the resource. The asset loader
     * that invokes the resolver is responsible for disposing the returned stream.
     */
    using ExternalResourceResolver = std::function<std::unique_ptr<System::IO::Stream>(const std::string& path)>;
}
