// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/Layer3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionBroadphase3DEXT.hpp"

#include <stdexcept>

namespace CNA::Extended::World3DEXT
{
    Layer3DEXT::Layer3DEXT(std::unique_ptr<ICollisionBroadphase3DEXT> space) : space_(std::move(space))
    {
        if (!space_)
        {
            throw std::invalid_argument("space must not be null.");
        }
    }

    void Layer3DEXT::Reset()
    {
        if (isDynamic_)
        {
            space_->Reset();
        }
    }
}
