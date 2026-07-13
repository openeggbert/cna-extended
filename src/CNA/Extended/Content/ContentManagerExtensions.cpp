// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Content/ContentManagerExtensions.hpp"

#include "Microsoft/Xna/Framework/Graphics/IGraphicsDeviceService.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "System/IServiceProvider.hpp"

namespace CNA::Extended::Content
{
    std::unique_ptr<System::IO::Stream> OpenStream(ContentManager& contentManager, const std::string& path)
    {
        return Microsoft::Xna::Framework::TitleContainer::OpenStream(contentManager.getRootDirectoryProperty() + DirectorySeparatorChar + path);
    }

    GraphicsDevice* GetGraphicsDevice(const ContentManager& contentManager)
    {
        System::IServiceProvider* serviceProvider = contentManager.getServiceProviderProperty();
        if (serviceProvider == nullptr)
        {
            return nullptr;
        }

        auto* graphicsDeviceService = static_cast<Microsoft::Xna::Framework::Graphics::IGraphicsDeviceService*>(
            serviceProvider->GetService(typeid(Microsoft::Xna::Framework::Graphics::IGraphicsDeviceService)));
        return graphicsDeviceService == nullptr ? nullptr : graphicsDeviceService->getGraphicsDeviceProperty();
    }
}
