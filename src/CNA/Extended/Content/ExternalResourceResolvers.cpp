// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Content/ExternalResourceResolvers.hpp"

#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "System/IO/FileStream.hpp"

#include <stdexcept>

namespace CNA::Extended::Content
{
    std::unique_ptr<System::IO::Stream> OpenFile(const std::string& path)
    {
        if (path.empty())
        {
            throw std::invalid_argument("path must not be empty.");
        }
        return std::make_unique<System::IO::FileStream>(path);
    }

    std::unique_ptr<System::IO::Stream> OpenTitleContainerStream(const std::string& path)
    {
        if (path.empty())
        {
            throw std::invalid_argument("path must not be empty.");
        }
        return Microsoft::Xna::Framework::TitleContainer::OpenStream(path);
    }
}
