// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Content/ExternalResourceResolvers.cs. Upstream's static class
// -> free functions in this namespace, matching this project's established convention for
// stateless static-method-only C# classes. `File.OpenRead(path)` (returning a lazily-read Stream)
// has no direct sharp-runtime equivalent -- System::IO::File only provides whole-content read
// methods (ReadAllBytes/ReadAllText/etc.), not a stream-returning open -- translated instead via
// System::IO::FileStream's single-path constructor (opens for reading), which returns the same
// lazily-readable Stream semantics upstream's File.OpenRead provides.
#pragma once

#include "CNA/Extended/Content/ExternalResourceResolver.hpp"

#include <memory>
#include <string>

namespace CNA::Extended::Content
{
    /**
     * @brief Opens a resource from the local file system.
     * @param path The absolute or relative file path to open.
     * @return A readable stream for the file.
     * @throws std::invalid_argument path is empty.
     */
    [[nodiscard]] std::unique_ptr<System::IO::Stream> OpenFile(const std::string& path);

    /**
     * @brief Opens a resource via TitleContainer.
     * @param path The title-relative resource path to open.
     * @return A readable stream for the resource.
     * @throws std::invalid_argument path is empty.
     */
    [[nodiscard]] std::unique_ptr<System::IO::Stream> OpenTitleContainerStream(const std::string& path);
}
