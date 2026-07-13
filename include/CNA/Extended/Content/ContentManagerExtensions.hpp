// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Content/ContentManagerExtensions.cs. Extension methods -> free
// functions, matching this project's established convention (see Content/ExternalResourceResolvers.hpp).
// Not xnb-specific: the `Load<T>(ContentManager&, path, loader)` overloads are a generic
// pluggable-loader mechanism used by this project's own JsonContentLoader/TextContentLoader,
// unrelated to the excluded Content Pipeline's `.xnb` readers.
//
// Upstream has two loader interfaces: `IContentLoader<out T>` (T fixed per implementation, e.g.
// `TextContentLoader : IContentLoader<string>`) and a non-generic `IContentLoader` whose single
// method (`T Load<T>(...)`) is itself generic -- a "non-generic interface with a generic method"
// has no direct C++ equivalent (virtual functions cannot be templates). `IContentLoader<T>` ports
// directly as a normal interface template. For the non-generic `IContentLoader`, every real call
// site in this codebase (`JsonContentLoader`, used by `TextureAtlasJsonConverter`) always uses a
// concrete loader type known at the call site, never through a polymorphic `IContentLoader*` -- so
// the `Load<T>(ContentManager&, path, IContentLoader&)` overload below is a template that
// duck-types its loader parameter (any type exposing `template<typename T> T Load(ContentManager&,
// const std::string&)`) rather than requiring a common virtual base, matching how it's actually
// used everywhere upstream while staying valid C++.
#pragma once

#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "System/IO/Stream.hpp"

#include <memory>
#include <string>

namespace CNA::Extended::Content
{
    using Microsoft::Xna::Framework::Content::ContentManager;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;

    /** @brief Loads content of a fixed type T from @p path using a caller-supplied loading strategy. */
    template <typename T>
    class IContentLoader
    {
    public:
        virtual ~IContentLoader() = default;
        [[nodiscard]] virtual T Load(ContentManager& contentManager, const std::string& path) = 0;
    };

    /** @brief The path separator upstream's OpenStream uses when joining ContentManager::RootDirectory to a relative path. */
    inline const std::string DirectorySeparatorChar = "/";

    /** @brief Opens a stream for @p path, relative to @p contentManager's RootDirectory, via TitleContainer. */
    [[nodiscard]] std::unique_ptr<System::IO::Stream> OpenStream(ContentManager& contentManager, const std::string& path);

    /** @brief Returns the GraphicsDevice associated with @p contentManager's IGraphicsDeviceService, or nullptr if none is registered. */
    [[nodiscard]] GraphicsDevice* GetGraphicsDevice(const ContentManager& contentManager);

    /** @brief Loads content of type T from @p path using @p contentLoader's fixed-T IContentLoader<T>::Load. */
    template <typename T>
    T Load(ContentManager& contentManager, const std::string& path, IContentLoader<T>& contentLoader)
    {
        return contentLoader.Load(contentManager, path);
    }

    /** @brief Loads content of type T from @p path using any loader exposing a template Load<T>(ContentManager&, path) method. */
    template <typename T, typename TLoader>
    T Load(ContentManager& contentManager, const std::string& path, TLoader& contentLoader)
    {
        return contentLoader.template Load<T>(contentManager, path);
    }
}
