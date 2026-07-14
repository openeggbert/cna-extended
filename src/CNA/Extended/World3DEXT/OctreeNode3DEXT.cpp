// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/OctreeNode3DEXT.hpp"

#include "CNA/Extended/World3DEXT/OctreeNodeData3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Collections/Generic/Queue.hpp"

#include <array>
#include <stdexcept>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;
    using Microsoft::Xna::Framework::Vector3;

    OctreeNode3DEXT::OctreeNode3DEXT(const BoundingBox& bounds) : NodeBounds(bounds)
    {
    }

    int OctreeNode3DEXT::NumTargets()
    {
        std::vector<OctreeNodeData3DEXT*> dirtyItems;
        int objectCount = 0;
        System::Collections::Generic::Queue<OctreeNode3DEXT*> process;
        process.Enqueue(this);

        while (process.getCountProperty() > 0)
        {
            OctreeNode3DEXT* processing = process.Dequeue();

            if (!processing->getIsLeafProperty())
            {
                for (const auto& child : processing->Children)
                {
                    process.Enqueue(child.get());
                }
            }
            else
            {
                for (OctreeNodeData3DEXT* data : processing->Contents)
                {
                    if (!data->getDirtyProperty())
                    {
                        objectCount++;
                        data->MarkDirty();
                        dirtyItems.push_back(data);
                    }
                }
            }
        }

        for (OctreeNodeData3DEXT* nodeData : dirtyItems)
        {
            nodeData->MarkClean();
        }

        return objectCount;
    }

    void OctreeNode3DEXT::Insert(OctreeNodeData3DEXT& data)
    {
        const BoundingBox actorBounds = data.getBoundsProperty();

        if (!NodeBounds.Intersects(actorBounds))
        {
            return;
        }

        if (getIsLeafProperty() && Contents.getCountProperty() >= MaxObjectsPerNode)
        {
            Split();
        }

        if (getIsLeafProperty())
        {
            AddToLeaf(data);
        }
        else
        {
            for (auto& child : Children)
            {
                child->Insert(data);
            }
        }
    }

    void OctreeNode3DEXT::Remove(OctreeNodeData3DEXT& data)
    {
        if (getIsLeafProperty())
        {
            data.RemoveParent(*this);
            Contents.Remove(&data);
        }
        else
        {
            throw std::logic_error("Cannot remove from a non-leaf OctreeNode3DEXT.");
        }
    }

    void OctreeNode3DEXT::Shake()
    {
        if (getIsLeafProperty())
        {
            return;
        }

        std::vector<OctreeNodeData3DEXT*> dirtyItems;
        const int numObjects = NumTargets();

        if (numObjects == 0)
        {
            Children.clear();
        }
        else if (numObjects < MaxObjectsPerNode)
        {
            System::Collections::Generic::Queue<OctreeNode3DEXT*> process;
            process.Enqueue(this);

            while (process.getCountProperty() > 0)
            {
                OctreeNode3DEXT* processing = process.Dequeue();

                if (!processing->getIsLeafProperty())
                {
                    for (auto& subTree : processing->Children)
                    {
                        process.Enqueue(subTree.get());
                    }
                }
                else
                {
                    for (OctreeNodeData3DEXT* data : processing->Contents)
                    {
                        if (!data->getDirtyProperty())
                        {
                            AddToLeaf(*data);
                            data->MarkDirty();
                            dirtyItems.push_back(data);
                        }
                    }
                }
            }

            Children.clear();
        }

        for (OctreeNodeData3DEXT* nodeData : dirtyItems)
        {
            nodeData->MarkClean();
        }
    }

    void OctreeNode3DEXT::Split()
    {
        if (CurrentDepth + 1 >= MaxDepth)
        {
            return;
        }

        const Vector3 min = NodeBounds.Min;
        const Vector3 max = NodeBounds.Max;
        const Vector3 center = (min + max) * 0.5f;

        // 8 octants: every combination of (min or center) .. (center or max) per axis.
        const std::array<BoundingBox, 8> childAreas{
            BoundingBox(Vector3(min.X, min.Y, min.Z), Vector3(center.X, center.Y, center.Z)),
            BoundingBox(Vector3(center.X, min.Y, min.Z), Vector3(max.X, center.Y, center.Z)),
            BoundingBox(Vector3(min.X, center.Y, min.Z), Vector3(center.X, max.Y, center.Z)),
            BoundingBox(Vector3(center.X, center.Y, min.Z), Vector3(max.X, max.Y, center.Z)),
            BoundingBox(Vector3(min.X, min.Y, center.Z), Vector3(center.X, center.Y, max.Z)),
            BoundingBox(Vector3(center.X, min.Y, center.Z), Vector3(max.X, center.Y, max.Z)),
            BoundingBox(Vector3(min.X, center.Y, center.Z), Vector3(center.X, max.Y, max.Z)),
            BoundingBox(Vector3(center.X, center.Y, center.Z), Vector3(max.X, max.Y, max.Z)),
        };

        for (const BoundingBox& childArea : childAreas)
        {
            auto node = std::make_unique<OctreeNode3DEXT>(childArea);
            node->CurrentDepth = CurrentDepth + 1;
            Children.push_back(std::move(node));
        }

        for (OctreeNodeData3DEXT* contentData : Contents)
        {
            for (auto& childNode : Children)
            {
                childNode->Insert(*contentData);
            }
        }

        Clear();
    }

    void OctreeNode3DEXT::ClearAll()
    {
        for (auto& childNode : Children)
        {
            childNode->ClearAll();
        }

        Clear();
    }

    std::vector<OctreeNodeData3DEXT*> OctreeNode3DEXT::Query(const BoundingBox& area) const
    {
        std::vector<OctreeNodeData3DEXT*> recursiveResult;
        QueryWithoutReset(area, recursiveResult);

        for (OctreeNodeData3DEXT* nodeData : recursiveResult)
        {
            nodeData->MarkClean();
        }

        return recursiveResult;
    }

    void OctreeNode3DEXT::AddToLeaf(OctreeNodeData3DEXT& data)
    {
        data.AddParent(*this);
        Contents.Add(&data);
    }

    void OctreeNode3DEXT::Clear()
    {
        for (OctreeNodeData3DEXT* nodeData : Contents)
        {
            nodeData->RemoveParent(*this);
        }

        Contents.Clear();
    }

    void OctreeNode3DEXT::QueryWithoutReset(const BoundingBox& area, std::vector<OctreeNodeData3DEXT*>& recursiveResult) const
    {
        if (!NodeBounds.Intersects(area))
        {
            return;
        }

        if (getIsLeafProperty())
        {
            for (OctreeNodeData3DEXT* nodeData : Contents)
            {
                if (!nodeData->getDirtyProperty() && nodeData->getBoundsProperty().Intersects(area))
                {
                    recursiveResult.push_back(nodeData);
                    nodeData->MarkDirty();
                }
            }
        }
        else
        {
            for (const auto& child : Children)
            {
                child->QueryWithoutReset(area, recursiveResult);
            }
        }
    }
}
