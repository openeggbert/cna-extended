// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/Layers/Layer.cs. Upstream lives in the
// MonoGame.Extended.Collisions.Layers namespace; flattened into CNA::Extended::Collisions,
// matching the same flattening decision already made for the QuadTree/SpatialHash broadphase
// implementations (see NEXT.md entry (26)) -- this project's Collisions module does not mirror
// upstream's QuadTree/Layers sub-namespaces 1:1.
//
// Upstream's `public readonly ICollisionBroadphase2D Space` is a reference-type field: the caller
// constructs a broadphase implementation and hands it to the Layer, which is then its sole owner
// for all practical purposes (nothing else in upstream ever holds a second reference to the same
// broadphase instance after constructing a Layer around it). Ported as exclusive ownership via
// `std::unique_ptr<ICollisionBroadphase2D>`, transferred into the Layer at construction --
// matching the real ownership intent GC makes invisible in the original.
#pragma once

#include <memory>

namespace CNA::Extended::Collisions
{
    class ICollisionBroadphase2D;

    /** @brief Groups collision actors that share one broadphase structure and one set of layer-level rules. */
    class Layer
    {
    public:
        /**
         * @brief Initializes a new Layer taking ownership of the specified broadphase structure.
         * @param space The broadphase structure used to store and query actors in this layer. Must not be null.
         */
        explicit Layer(std::unique_ptr<ICollisionBroadphase2D> space);

        virtual ~Layer() = default;

        /** @brief Gets the broadphase structure that stores the layer's actors. */
        [[nodiscard]] ICollisionBroadphase2D& getSpaceProperty() const { return *space_; }

        /** @brief Gets whether the broadphase is rebuilt during each reset. Defaults to true. */
        [[nodiscard]] bool getIsDynamicProperty() const { return isDynamic_; }

        /** @brief Sets whether the broadphase is rebuilt during each reset. */
        void setIsDynamicProperty(bool value) { isDynamic_ = value; }

        /** @brief Rebuilds the layer broadphase when the layer is dynamic. */
        virtual void Reset();

    private:
        std::unique_ptr<ICollisionBroadphase2D> space_;
        bool isDynamic_ = true;
    };
}
