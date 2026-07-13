// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collisions/QuadtreeData.hpp"

#include "CNA/Extended/Collisions/ICollisionActor.hpp"
#include "CNA/Extended/Collisions/QuadTree.hpp"

namespace CNA::Extended::Collisions
{
    QuadtreeData::QuadtreeData(ICollisionActor& target) : target_(&target)
    {
        bounds_ = target.getShapeProperty().getBoundingBoxProperty();
    }

    void QuadtreeData::RemoveParent(QuadTree& parent)
    {
        parents_.Remove(&parent);
    }

    void QuadtreeData::AddParent(QuadTree& parent)
    {
        parents_.Add(&parent);
        bounds_ = target_->getShapeProperty().getBoundingBoxProperty();
    }

    void QuadtreeData::RemoveFromAllParents()
    {
        for (QuadTree* parent : parents_.ToArray())
        {
            parent->Remove(*this);
        }

        parents_.Clear();
    }
}
