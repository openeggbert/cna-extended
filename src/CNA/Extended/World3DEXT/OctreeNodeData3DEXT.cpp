// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/OctreeNodeData3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "CNA/Extended/World3DEXT/OctreeNode3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    OctreeNodeData3DEXT::OctreeNodeData3DEXT(ICollisionActor3DEXT& target) : target_(&target)
    {
        bounds_ = target.getShapeProperty().getBoundingBoxProperty();
    }

    void OctreeNodeData3DEXT::RemoveParent(OctreeNode3DEXT& parent)
    {
        parents_.Remove(&parent);
    }

    void OctreeNodeData3DEXT::AddParent(OctreeNode3DEXT& parent)
    {
        parents_.Add(&parent);
        bounds_ = target_->getShapeProperty().getBoundingBoxProperty();
    }

    void OctreeNodeData3DEXT::RemoveFromAllParents()
    {
        for (OctreeNode3DEXT* parent : parents_.ToArray())
        {
            parent->Remove(*this);
        }

        parents_.Clear();
    }
}
