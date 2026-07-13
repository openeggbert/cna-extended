// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/Texture2DRegion.Extensions.cs (upstream file name
// `Texture2DRegionExtensions`, matching the static class name rather than the file name, per this
// project's convention of naming extension-method headers after the class they extend). Extension
// methods -> free functions in this namespace. `textureRegion` is nullable in C# (extension
// methods may be invoked on a null receiver, and the null is checked inside the method body), so
// it is taken here as `const std::shared_ptr<Texture2DRegion>&`, matching Texture2DRegion.hpp's
// shared-ownership convention, with an explicit std::invalid_argument check reproducing
// ArgumentNullException.ThrowIfNull. `GetSubregion` returns a null shared_ptr when the requested
// subregion does not intersect the trimmed bounds (upstream returns C# null).
#pragma once

#include "CNA/Extended/Graphics/NinePatch.hpp"
#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "CNA/Extended/Thickness.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

#include <memory>
#include <string>

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::Rectangle;

    /**
     * @brief Gets a subregion of @p textureRegion using the provided rectangle (in original
     * sprite coordinates), or nullptr if the requested subregion does not intersect the trimmed bounds.
     * @throws std::invalid_argument @p textureRegion is null.
     */
    [[nodiscard]] std::shared_ptr<Texture2DRegion> GetSubregion(
        const std::shared_ptr<Texture2DRegion>& textureRegion, const Rectangle& region);

    /**
     * @brief Gets a named subregion of @p textureRegion using the provided rectangle (in original
     * sprite coordinates), or nullptr if the requested subregion does not intersect the trimmed bounds.
     * @throws std::invalid_argument @p textureRegion is null.
     */
    [[nodiscard]] std::shared_ptr<Texture2DRegion> GetSubregion(
        const std::shared_ptr<Texture2DRegion>& textureRegion, const std::string& name, const Rectangle& region);

    /**
     * @brief Gets a subregion of @p textureRegion using the provided coordinates and dimensions
     * (in original sprite coordinates), or nullptr if the requested subregion does not intersect
     * the trimmed bounds.
     * @throws std::invalid_argument @p textureRegion is null.
     */
    [[nodiscard]] std::shared_ptr<Texture2DRegion> GetSubregion(
        const std::shared_ptr<Texture2DRegion>& textureRegion, int x, int y, int width, int height);

    /**
     * @brief Gets a named subregion of @p textureRegion using the provided coordinates and
     * dimensions (in original sprite coordinates), or nullptr if the requested subregion does not
     * intersect the trimmed bounds.
     * @throws std::invalid_argument @p textureRegion is null.
     */
    [[nodiscard]] std::shared_ptr<Texture2DRegion> GetSubregion(
        const std::shared_ptr<Texture2DRegion>& textureRegion, int x, int y, int width, int height, const std::string& name);

    /**
     * @brief Creates a nine-patch from @p textureRegion with the specified padding.
     * @throws std::invalid_argument @p textureRegion is null.
     */
    [[nodiscard]] std::shared_ptr<NinePatch> CreateNinePatch(
        const std::shared_ptr<Texture2DRegion>& textureRegion, const Thickness& padding);

    /**
     * @brief Creates a nine-patch from @p textureRegion with uniform padding on all sides.
     * @throws std::invalid_argument @p textureRegion is null.
     */
    [[nodiscard]] std::shared_ptr<NinePatch> CreateNinePatch(
        const std::shared_ptr<Texture2DRegion>& textureRegion, int padding);

    /**
     * @brief Creates a nine-patch from @p textureRegion with non-uniform padding.
     * @throws std::invalid_argument @p textureRegion is null.
     */
    [[nodiscard]] std::shared_ptr<NinePatch> CreateNinePatch(const std::shared_ptr<Texture2DRegion>& textureRegion,
        int leftPadding, int topPadding, int rightPadding, int bottomPadding);
}
