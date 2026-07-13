// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/NinePatch.hpp"

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <stdexcept>
#include <utility>

namespace CNA::Extended::Graphics
{
    NinePatch::NinePatch(std::vector<std::shared_ptr<Texture2DRegion>> patches)
        : NinePatch(std::move(patches), "")
    {
    }

    NinePatch::NinePatch(std::vector<std::shared_ptr<Texture2DRegion>> patches, const std::string& name)
        : patches_(std::move(patches))
    {
        if (patches_.size() != 9)
        {
            throw std::invalid_argument("patches must contain exactly 9 elements.");
        }

        const Size topLeft = patches_[NinePatch::TopLeft]->getOriginalSizeProperty();
        const Size bottomRight = patches_[NinePatch::BottomRight]->getOriginalSizeProperty();
        padding_ = Thickness(topLeft.Width, topLeft.Height, bottomRight.Width, bottomRight.Height);

        name_ = name.empty() ? (patches_[NinePatch::TopLeft]->getTextureProperty()->getNameProperty() + "-nine-patch") : name;
    }
}
