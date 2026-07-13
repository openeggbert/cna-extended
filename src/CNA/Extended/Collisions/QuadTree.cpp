// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collisions/QuadTree.hpp"

#include "CNA/Extended/Collisions/QuadtreeData.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/Collections/Generic/Queue.hpp"

#include <array>
#include <stdexcept>

namespace CNA::Extended::Collisions
{
    using Microsoft::Xna::Framework::Vector2;

    QuadTree::QuadTree(const BoundingBox2D& bounds) : NodeBounds(bounds)
    {
    }

    int QuadTree::NumTargets()
    {
        std::vector<QuadtreeData*> dirtyItems;
        int objectCount = 0;
        System::Collections::Generic::Queue<QuadTree*> process;
        process.Enqueue(this);

        while (process.getCountProperty() > 0)
        {
            QuadTree* processing = process.Dequeue();

            if (!processing->getIsLeafProperty())
            {
                for (const auto& child : processing->Children)
                {
                    process.Enqueue(child.get());
                }
            }
            else
            {
                for (QuadtreeData* data : processing->Contents)
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

        for (QuadtreeData* quadtreeData : dirtyItems)
        {
            quadtreeData->MarkClean();
        }

        return objectCount;
    }

    void QuadTree::Insert(QuadtreeData& data)
    {
        const BoundingBox2D actorBounds = data.getBoundsProperty();

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

    void QuadTree::Remove(QuadtreeData& data)
    {
        if (getIsLeafProperty())
        {
            data.RemoveParent(*this);
            Contents.Remove(&data);
        }
        else
        {
            throw std::logic_error("Cannot remove from a non-leaf QuadTree.");
        }
    }

    void QuadTree::Shake()
    {
        if (getIsLeafProperty())
        {
            return;
        }

        std::vector<QuadtreeData*> dirtyItems;
        const int numObjects = NumTargets();

        if (numObjects == 0)
        {
            Children.clear();
        }
        else if (numObjects < MaxObjectsPerNode)
        {
            System::Collections::Generic::Queue<QuadTree*> process;
            process.Enqueue(this);

            while (process.getCountProperty() > 0)
            {
                QuadTree* processing = process.Dequeue();

                if (!processing->getIsLeafProperty())
                {
                    for (auto& subTree : processing->Children)
                    {
                        process.Enqueue(subTree.get());
                    }
                }
                else
                {
                    for (QuadtreeData* data : processing->Contents)
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

        for (QuadtreeData* quadtreeData : dirtyItems)
        {
            quadtreeData->MarkClean();
        }
    }

    void QuadTree::Split()
    {
        if (CurrentDepth + 1 >= MaxDepth)
        {
            return;
        }

        const Vector2 min = NodeBounds.Min;
        const Vector2 max = NodeBounds.Max;
        const Vector2 center = NodeBounds.getCenterProperty();

        const std::array<BoundingBox2D, 4> childAreas{
            BoundingBox2D(min, center), BoundingBox2D(Vector2(center.X, min.Y), Vector2(max.X, center.Y)), BoundingBox2D(center, max),
            BoundingBox2D(Vector2(min.X, center.Y), Vector2(center.X, max.Y))};

        for (const BoundingBox2D& childArea : childAreas)
        {
            auto node = std::make_unique<QuadTree>(childArea);
            node->CurrentDepth = CurrentDepth + 1;
            Children.push_back(std::move(node));
        }

        for (QuadtreeData* contentQuadtree : Contents)
        {
            for (auto& childQuadtree : Children)
            {
                childQuadtree->Insert(*contentQuadtree);
            }
        }

        Clear();
    }

    void QuadTree::ClearAll()
    {
        for (auto& childQuadtree : Children)
        {
            childQuadtree->ClearAll();
        }

        Clear();
    }

    std::vector<QuadtreeData*> QuadTree::Query(const BoundingBox2D& area) const
    {
        std::vector<QuadtreeData*> recursiveResult;
        QueryWithoutReset(area, recursiveResult);

        for (QuadtreeData* quadtreeData : recursiveResult)
        {
            quadtreeData->MarkClean();
        }

        return recursiveResult;
    }

    void QuadTree::AddToLeaf(QuadtreeData& data)
    {
        data.AddParent(*this);
        Contents.Add(&data);
    }

    void QuadTree::Clear()
    {
        for (QuadtreeData* quadtreeData : Contents)
        {
            quadtreeData->RemoveParent(*this);
        }

        Contents.Clear();
    }

    void QuadTree::QueryWithoutReset(const BoundingBox2D& area, std::vector<QuadtreeData*>& recursiveResult) const
    {
        if (!NodeBounds.Intersects(area))
        {
            return;
        }

        if (getIsLeafProperty())
        {
            for (QuadtreeData* quadtreeData : Contents)
            {
                if (!quadtreeData->getDirtyProperty() && quadtreeData->getBoundsProperty().Intersects(area))
                {
                    recursiveResult.push_back(quadtreeData);
                    quadtreeData->MarkDirty();
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
