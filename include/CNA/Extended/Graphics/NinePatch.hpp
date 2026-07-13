// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/NinePatch.cs. `ReadOnlySpan<Texture2DRegion> Patches`
// has no direct C++ equivalent that also preserves the shared_ptr ownership documented in
// Texture2DRegion.hpp; exposed as `const std::vector<std::shared_ptr<Texture2DRegion>>&` instead,
// matching this project's established non-owning-view-of-owned-storage convention.
#pragma once

#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "CNA/Extended/Thickness.hpp"

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::Graphics
{
    /**
     * @brief Represents a nine-patch texture: a single texture region subdivided into nine
     * subregions for scalable UI rendering (corners unscaled, edges stretched along one axis,
     * center scaled along both).
     */
    class NinePatch
    {
    public:
        /** @brief The index representing the top-left patch. */
        static constexpr int TopLeft = 0;
        /** @brief The index representing the top-middle patch. */
        static constexpr int TopMiddle = 1;
        /** @brief The index representing the top-right patch. */
        static constexpr int TopRight = 2;
        /** @brief The index representing the middle-left patch. */
        static constexpr int MiddleLeft = 3;
        /** @brief The index representing the middle patch. */
        static constexpr int Middle = 4;
        /** @brief The index representing the middle-right patch. */
        static constexpr int MiddleRight = 5;
        /** @brief The index representing the bottom-left patch. */
        static constexpr int BottomLeft = 6;
        /** @brief The index representing the bottom-middle patch. */
        static constexpr int BottomMiddle = 7;
        /** @brief The index representing the bottom-right patch. */
        static constexpr int BottomRight = 8;

        /**
         * @brief Initializes a nine-patch from the specified patches, in TopLeft..BottomRight order.
         * @throws std::invalid_argument @p patches does not contain exactly nine elements.
         */
        explicit NinePatch(std::vector<std::shared_ptr<Texture2DRegion>> patches);

        /**
         * @brief Initializes a nine-patch from the specified patches and name, in
         * TopLeft..BottomRight order. If @p name is empty, a name is generated from the
         * top-left patch's texture name.
         * @throws std::invalid_argument @p patches does not contain exactly nine elements.
         */
        NinePatch(std::vector<std::shared_ptr<Texture2DRegion>> patches, const std::string& name);

        /** @brief Gets the name assigned to this nine-patch. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }

        /** @brief Gets the size of the border patches around the middle patch. */
        [[nodiscard]] const Thickness& getPaddingProperty() const { return padding_; }

        /** @brief Gets the texture regions that make up the nine-patch, in TopLeft..BottomRight order. */
        [[nodiscard]] const std::vector<std::shared_ptr<Texture2DRegion>>& getPatchesProperty() const { return patches_; }

    private:
        std::vector<std::shared_ptr<Texture2DRegion>> patches_;
        std::string name_;
        Thickness padding_;
    };
}
