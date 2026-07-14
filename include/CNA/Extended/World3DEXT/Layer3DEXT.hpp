// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Layer3DEXT -- new, non-upstream addition. See 3d.md/plan3d.md
// at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::Layer, same shape with
// ICollisionBroadphase3DEXT in place of ICollisionBroadphase2D -- groups collision actors
// that share one broadphase structure and one set of layer-level rules. Exclusive ownership
// of the broadphase via std::unique_ptr, matching Layer.hpp's own real-ownership-intent
// rationale.
#pragma once

#include <memory>

namespace CNA::Extended::World3DEXT
{
    class ICollisionBroadphase3DEXT;

    /** @brief Groups 3D collision actors that share one broadphase structure and one set of layer-level rules. */
    class Layer3DEXT
    {
    public:
        /**
         * @brief Initializes a new Layer3DEXT taking ownership of the specified broadphase structure.
         * @param space The broadphase structure used to store and query actors in this layer. Must not be null.
         */
        explicit Layer3DEXT(std::unique_ptr<ICollisionBroadphase3DEXT> space);

        virtual ~Layer3DEXT() = default;

        /** @brief Gets the broadphase structure that stores the layer's actors. */
        [[nodiscard]] ICollisionBroadphase3DEXT& getSpaceProperty() const { return *space_; }

        /** @brief Gets whether the broadphase is rebuilt during each reset. Defaults to true. */
        [[nodiscard]] bool getIsDynamicProperty() const { return isDynamic_; }

        /** @brief Sets whether the broadphase is rebuilt during each reset. */
        void setIsDynamicProperty(bool value) { isDynamic_ = value; }

        /** @brief Rebuilds the layer broadphase when the layer is dynamic. */
        virtual void Reset();

    private:
        std::unique_ptr<ICollisionBroadphase3DEXT> space_;
        bool isDynamic_ = true;
    };
}
