// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Content/TexturePacker/TexturePackerFileReader.hpp"

#include "System/IO/File.hpp"
#include "System/Text/Json/JsonSerializer.hpp"

#include <vector>

namespace CNA::Extended::Content::TexturePacker
{
    namespace
    {
        std::string ReadAllText(System::IO::Stream& stream)
        {
            const SharpRuntime::intcs length = stream.getLengthProperty();
            std::vector<SharpRuntime::bytecs> buffer(static_cast<std::size_t>(length));

            SharpRuntime::intcs totalRead = 0;
            while (totalRead < length)
            {
                const SharpRuntime::intcs read = stream.Read(buffer.data(), totalRead, length - totalRead);
                if (read == 0)
                {
                    break;
                }
                totalRead += read;
            }

            return std::string(buffer.begin(), buffer.begin() + totalRead);
        }
    }

    TexturePackerFileContent Read(const std::string& path)
    {
        const std::string json = System::IO::File::ReadAllText(path);
        return System::Text::Json::JsonSerializer::Deserialize<TexturePackerFileContent>(json);
    }

    TexturePackerFileContent Read(System::IO::Stream& stream)
    {
        const std::string json = ReadAllText(stream);
        return System::Text::Json::JsonSerializer::Deserialize<TexturePackerFileContent>(json);
    }
}
