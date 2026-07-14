// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for OctreeNode3DEXT (see 3d.md/plan3d.md) -- there is no upstream
// MonoGame.Extended test suite to port here (MonoGame.Extended is 2D-only). Mirrors
// CNA::Extended::Collisions::QuadTreeTests.cpp's full coverage (2026-07-14, user-requested
// parity work), extended from 4 quadrants to 8 octants. As in QuadTreeTests.cpp: every
// BasicActor3DEXT/OctreeNodeData3DEXT a test creates must be kept alive for the test's
// duration since OctreeNodeData3DEXT/OctreeNode3DEXT hold only non-owning raw pointers (see
// OctreeNodeData3DEXT.hpp/OctreeNode3DEXT.hpp's ownership rationale) -- done here via
// std::vector<std::unique_ptr<T>> (pointer-stable), plus a small MakeEntry helper mirroring
// EntryPool's shape.
#include "CNA/Extended/World3DEXT/OctreeNode3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "CNA/Extended/World3DEXT/OctreeNodeData3DEXT.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    namespace
    {
        class BasicActor3DEXT final : public ICollisionActor3DEXT
        {
        public:
            BasicActor3DEXT() : id_(NextId()) { SetBounds(BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f))); }

            explicit BasicActor3DEXT(const BoundingBox& bounds) : id_(NextId()) { SetBounds(bounds); }

            [[nodiscard]] int getIdProperty() const override { return id_; }

            [[nodiscard]] CollisionShape3DEXT getShapeProperty() const override { return shape_; }

            void SetBounds(const BoundingBox& bounds) { shape_ = CollisionShape3DEXT(bounds); }

        private:
            static int NextId()
            {
                static int nextId = 1;
                return nextId++;
            }

            int id_;
            CollisionShape3DEXT shape_;
        };

        const BoundingBox kOctreeArea(Vector3(-10.0f, -15.0f, -20.0f), Vector3(10.0f, 15.0f, 20.0f));

        OctreeNode3DEXT CreateTree()
        {
            return OctreeNode3DEXT(kOctreeArea);
        }

        struct EntryPool
        {
            std::vector<std::unique_ptr<BasicActor3DEXT>> actors;
            std::vector<std::unique_ptr<OctreeNodeData3DEXT>> entries;

            OctreeNodeData3DEXT& MakeEntry()
            {
                actors.push_back(std::make_unique<BasicActor3DEXT>());
                entries.push_back(std::make_unique<OctreeNodeData3DEXT>(*actors.back()));
                return *entries.back();
            }

            OctreeNodeData3DEXT& MakeEntry(const BoundingBox& bounds)
            {
                actors.push_back(std::make_unique<BasicActor3DEXT>(bounds));
                entries.push_back(std::make_unique<OctreeNodeData3DEXT>(*actors.back()));
                return *entries.back();
            }
        };
    }

    TEST(OctreeNode3DEXTTests, ConstructorStoresBoundsAndStartsAsLeaf)
    {
        const BoundingBox bounds(Vector3(-10.0f, -15.0f, -20.0f), Vector3(10.0f, 15.0f, 20.0f));
        OctreeNode3DEXT tree(bounds);

        EXPECT_EQ(tree.getNodeBoundsProperty().Min, bounds.Min);
        EXPECT_EQ(tree.getNodeBoundsProperty().Max, bounds.Max);
        EXPECT_TRUE(tree.getIsLeafProperty());
    }

    TEST(OctreeNode3DEXTTests, NumTargetsWhenTreeIsEmptyReturnsZero)
    {
        OctreeNode3DEXT tree = CreateTree();
        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(OctreeNode3DEXTTests, NumTargetsWhenTreeContainsOneActorReturnsOne)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;

        tree.Insert(pool.MakeEntry());

        EXPECT_EQ(tree.NumTargets(), 1);
    }

    TEST(OctreeNode3DEXTTests, NumTargetsWhenTreeContainsMultipleActorsReturnsCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;

        for (int i = 0; i < 5; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        EXPECT_EQ(tree.NumTargets(), 5);
    }

    TEST(OctreeNode3DEXTTests, NumTargetsWhenActorsAreInsertedIncrementallyReturnsRunningCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;

        for (int i = 0; i < 1000; i++)
        {
            tree.Insert(pool.MakeEntry());
            EXPECT_EQ(tree.NumTargets(), i + 1);
        }

        EXPECT_EQ(tree.NumTargets(), 1000);
    }

    TEST(OctreeNode3DEXTTests, InsertWhenOneActorIsInsertedIncreasesTargetCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;

        tree.Insert(pool.MakeEntry());

        EXPECT_EQ(tree.NumTargets(), 1);
    }

    TEST(OctreeNode3DEXTTests, InsertWhenOneActorOverlapsOctantsCountsActorOnce)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;

        tree.Insert(pool.MakeEntry(BoundingBox(Vector3(-2.5f, -2.5f, -2.5f), Vector3(2.5f, 2.5f, 2.5f))));

        EXPECT_EQ(tree.NumTargets(), 1);
    }

    TEST(OctreeNode3DEXTTests, InsertWhenMultipleActorsAreInsertedIncreasesTargetCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;

        for (int i = 0; i < 10; i++)
        {
            tree.Insert(pool.MakeEntry(BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f))));
        }

        EXPECT_EQ(tree.NumTargets(), 10);
    }

    TEST(OctreeNode3DEXTTests, InsertWhenManyActorsAreInsertedIncreasesTargetCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;

        for (int i = 0; i < 1000; i++)
        {
            tree.Insert(pool.MakeEntry(BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f))));
        }

        EXPECT_EQ(tree.NumTargets(), 1000);
    }

    TEST(OctreeNode3DEXTTests, InsertWhenMultipleActorsOverlapOctantsCountsActorsOnceEach)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;

        for (int i = 0; i < 10; i++)
        {
            tree.Insert(pool.MakeEntry(kOctreeArea));
        }

        EXPECT_EQ(tree.NumTargets(), 10);
    }

    TEST(OctreeNode3DEXTTests, RemoveWhenOnlyActorIsRemovedLeavesTreeEmpty)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        OctreeNodeData3DEXT& data = pool.MakeEntry(BoundingBox(Vector3(-5.0f, -7.0f, -9.0f), Vector3(5.0f, 8.0f, 6.0f)));

        tree.Insert(data);
        tree.Remove(data);

        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(OctreeNode3DEXTTests, RemoveWhenTwoActorsAreRemovedUpdatesTargetCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        std::vector<OctreeNodeData3DEXT*> inserted;
        const int numTargets = 2;

        for (int i = 0; i < numTargets; i++)
        {
            OctreeNodeData3DEXT& data = pool.MakeEntry(BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));
            tree.Insert(data);
            inserted.push_back(&data);
        }

        int inTree = numTargets;
        EXPECT_EQ(tree.NumTargets(), inTree);

        for (OctreeNodeData3DEXT* data : inserted)
        {
            tree.Remove(*data);
            --inTree;
            EXPECT_EQ(tree.NumTargets(), inTree);
        }
    }

    TEST(OctreeNode3DEXTTests, RemoveWhenThreeActorsAreRemovedUpdatesTargetCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        std::vector<OctreeNodeData3DEXT*> inserted;
        const int numTargets = 3;

        for (int i = 0; i < numTargets; i++)
        {
            OctreeNodeData3DEXT& data = pool.MakeEntry(BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));
            tree.Insert(data);
            inserted.push_back(&data);
        }

        int inTree = numTargets;
        EXPECT_EQ(tree.NumTargets(), inTree);

        for (OctreeNodeData3DEXT* data : inserted)
        {
            tree.Remove(*data);
            --inTree;
            EXPECT_EQ(tree.NumTargets(), inTree);
        }
    }

    TEST(OctreeNode3DEXTTests, RemoveFromAllParentsWhenManyActorsAreRemovedUpdatesTargetCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        std::vector<OctreeNodeData3DEXT*> inserted;
        const int numTargets = 1000;

        for (int i = 0; i < numTargets; i++)
        {
            OctreeNodeData3DEXT& data = pool.MakeEntry(BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));
            tree.Insert(data);
            inserted.push_back(&data);
        }

        int inTree = numTargets;
        EXPECT_EQ(tree.NumTargets(), inTree);

        for (OctreeNodeData3DEXT* data : inserted)
        {
            data->RemoveFromAllParents();
            --inTree;
            EXPECT_EQ(tree.NumTargets(), inTree);
        }
    }

    TEST(OctreeNode3DEXTTests, ShakeWhenTreeIsEmptyKeepsTargetCountAtZero)
    {
        OctreeNode3DEXT tree = CreateTree();
        tree.Shake();
        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(OctreeNode3DEXTTests, ShakeWhenSplitTreeIsEmptyKeepsTargetCountAtZero)
    {
        OctreeNode3DEXT tree = CreateTree();
        tree.Split();
        tree.Shake();
        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(OctreeNode3DEXTTests, ShakeWhenSplitTreeContainsActorKeepsTargetCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;

        tree.Split();
        tree.Insert(pool.MakeEntry());
        tree.Shake();

        EXPECT_EQ(tree.NumTargets(), 1);
    }

    TEST(OctreeNode3DEXTTests, ShakeWhenTreeContainsOneActorKeepsTargetCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        const int numTargets = 1;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        tree.Shake();

        EXPECT_EQ(tree.NumTargets(), numTargets);
    }

    TEST(OctreeNode3DEXTTests, ShakeWhenTreeContainsTwoActorsKeepsTargetCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        const int numTargets = 2;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        tree.Shake();

        EXPECT_EQ(tree.NumTargets(), numTargets);
    }

    TEST(OctreeNode3DEXTTests, ShakeWhenTreeContainsThreeActorsKeepsTargetCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        const int numTargets = 3;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        tree.Shake();

        EXPECT_EQ(tree.NumTargets(), numTargets);
    }

    TEST(OctreeNode3DEXTTests, ShakeWhenTreeContainsManyActorsKeepsTargetCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        const int numTargets = OctreeNode3DEXT::DefaultMaxObjectsPerNode + 1;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        tree.Shake();

        EXPECT_EQ(tree.NumTargets(), numTargets);
    }

    TEST(OctreeNode3DEXTTests, QueryWhenTreeIsEmptyReturnsNoResults)
    {
        OctreeNode3DEXT tree = CreateTree();

        const std::vector<OctreeNodeData3DEXT*> query = tree.Query(kOctreeArea);

        EXPECT_TRUE(query.empty());
        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(OctreeNode3DEXTTests, QueryWhenAreaDoesNotOverlapTreeReturnsNoResults)
    {
        OctreeNode3DEXT tree = CreateTree();
        const BoundingBox area(Vector3(100.0f, 100.0f, 100.0f), Vector3(101.0f, 101.0f, 101.0f));

        const std::vector<OctreeNodeData3DEXT*> query = tree.Query(area);

        EXPECT_TRUE(query.empty());
        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(OctreeNode3DEXTTests, QueryWhenLeafNodeContainsActorReturnsActor)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;

        tree.Insert(pool.MakeEntry());

        const std::vector<OctreeNodeData3DEXT*> query = tree.Query(kOctreeArea);

        EXPECT_EQ(query.size(), 1u);
        EXPECT_EQ(static_cast<int>(query.size()), tree.NumTargets());
    }

    TEST(OctreeNode3DEXTTests, QueryWhenLeafNodeDoesNotOverlapAreaReturnsNoResults)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        const BoundingBox area(Vector3(100.0f, 100.0f, 100.0f), Vector3(101.0f, 101.0f, 101.0f));

        tree.Insert(pool.MakeEntry());

        const std::vector<OctreeNodeData3DEXT*> query = tree.Query(area);

        EXPECT_TRUE(query.empty());
    }

    TEST(OctreeNode3DEXTTests, QueryWhenLeafNodeContainsMultipleActorsReturnsAllActors)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        const int numTargets = OctreeNode3DEXT::DefaultMaxObjectsPerNode;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        const std::vector<OctreeNodeData3DEXT*> query = tree.Query(kOctreeArea);

        EXPECT_EQ(static_cast<int>(query.size()), numTargets);
        EXPECT_EQ(static_cast<int>(query.size()), tree.NumTargets());
    }

    TEST(OctreeNode3DEXTTests, QueryWhenNonLeafTreeContainsManyActorsReturnsAllActors)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        const int numTargets = 2 * OctreeNode3DEXT::DefaultMaxObjectsPerNode;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        const std::vector<OctreeNodeData3DEXT*> query = tree.Query(kOctreeArea);

        EXPECT_EQ(static_cast<int>(query.size()), numTargets);
        EXPECT_EQ(static_cast<int>(query.size()), tree.NumTargets());
    }

    TEST(OctreeNode3DEXTTests, QueryWhenCalledTwiceConsecutivelyReturnsSameResultCount)
    {
        OctreeNode3DEXT tree = CreateTree();
        EntryPool pool;
        const int numTargets = 2 * OctreeNode3DEXT::DefaultMaxObjectsPerNode;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        const std::vector<OctreeNodeData3DEXT*> query1 = tree.Query(kOctreeArea);
        const std::vector<OctreeNodeData3DEXT*> query2 = tree.Query(kOctreeArea);

        EXPECT_EQ(static_cast<int>(query1.size()), numTargets);
        EXPECT_EQ(static_cast<int>(query1.size()), tree.NumTargets());
        EXPECT_EQ(query1.size(), query2.size());
    }
}
