// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/Collisions/QuadTreeTests.cs.
// Upstream's `new BasicActor()`/`new QuadtreeData(actor)` create GC-tracked heap objects that live
// as long as anything references them (the tree itself, via QuadtreeData's non-owning pointer back
// to its actor, and QuadTree nodes' non-owning pointers to QuadtreeData). This port's QuadtreeData/
// QuadTree hold only non-owning raw pointers (see QuadtreeData.hpp/QuadTree.hpp's ownership
// rationale), so every BasicActor/QuadtreeData a test creates must be kept alive for the test's
// duration -- done here via std::vector<std::unique_ptr<T>> (pointer-stable: a unique_ptr's pointee
// address never changes even when the outer vector reallocates), plus a small MakeEntry helper to
// keep each test's loop body close to upstream's `new QuadtreeData(new BasicActor())` shape.
#include "CNA/Extended/Collisions/QuadTree.hpp"

#include "CNA/Extended/Collisions/Implementation/BasicActor.hpp"
#include "CNA/Extended/Collisions/QuadtreeData.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::Collisions
{
    namespace
    {
        using Tests::BasicActor;

        const BoundingBox2D kQuadTreeArea(Vector2(-10.0f, -15.0f), Vector2(10.0f, 15.0f));

        QuadTree CreateTree()
        {
            return QuadTree(kQuadTreeArea);
        }

        struct EntryPool
        {
            std::vector<std::unique_ptr<BasicActor>> actors;
            std::vector<std::unique_ptr<QuadtreeData>> entries;

            QuadtreeData& MakeEntry()
            {
                actors.push_back(std::make_unique<BasicActor>());
                entries.push_back(std::make_unique<QuadtreeData>(*actors.back()));
                return *entries.back();
            }

            QuadtreeData& MakeEntry(const BoundingBox2D& bounds)
            {
                actors.push_back(std::make_unique<BasicActor>(bounds));
                entries.push_back(std::make_unique<QuadtreeData>(*actors.back()));
                return *entries.back();
            }
        };
    }

    TEST(QuadTreeTests, ConstructorStoresBoundsAndStartsAsLeaf)
    {
        const BoundingBox2D bounds(Vector2(-10.0f, -15.0f), Vector2(10.0f, 15.0f));
        QuadTree tree(bounds);

        EXPECT_EQ(tree.getNodeBoundsProperty().Min, bounds.Min);
        EXPECT_EQ(tree.getNodeBoundsProperty().Max, bounds.Max);
        EXPECT_TRUE(tree.getIsLeafProperty());
    }

    TEST(QuadTreeTests, NumTargetsWhenTreeIsEmptyReturnsZero)
    {
        QuadTree tree = CreateTree();
        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(QuadTreeTests, NumTargetsWhenTreeContainsOneActorReturnsOne)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;

        tree.Insert(pool.MakeEntry());

        EXPECT_EQ(tree.NumTargets(), 1);
    }

    TEST(QuadTreeTests, NumTargetsWhenTreeContainsMultipleActorsReturnsCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;

        for (int i = 0; i < 5; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        EXPECT_EQ(tree.NumTargets(), 5);
    }

    TEST(QuadTreeTests, NumTargetsWhenActorsAreInsertedIncrementallyReturnsRunningCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;

        for (int i = 0; i < 1000; i++)
        {
            tree.Insert(pool.MakeEntry());
            EXPECT_EQ(tree.NumTargets(), i + 1);
        }

        EXPECT_EQ(tree.NumTargets(), 1000);
    }

    TEST(QuadTreeTests, InsertWhenOneActorIsInsertedIncreasesTargetCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;

        tree.Insert(pool.MakeEntry());

        EXPECT_EQ(tree.NumTargets(), 1);
    }

    TEST(QuadTreeTests, InsertWhenOneActorOverlapsQuadrantsCountsActorOnce)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;

        tree.Insert(pool.MakeEntry(BoundingBox2D::CreateFromPositionAndSize(Vector2(-2.5f, -2.5f), Vector2(5.0f, 5.0f))));

        EXPECT_EQ(tree.NumTargets(), 1);
    }

    TEST(QuadTreeTests, InsertWhenMultipleActorsAreInsertedIncreasesTargetCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;

        for (int i = 0; i < 10; i++)
        {
            tree.Insert(pool.MakeEntry(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2::One)));
        }

        EXPECT_EQ(tree.NumTargets(), 10);
    }

    TEST(QuadTreeTests, InsertWhenManyActorsAreInsertedIncreasesTargetCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;

        for (int i = 0; i < 1000; i++)
        {
            tree.Insert(pool.MakeEntry(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2::One)));
        }

        EXPECT_EQ(tree.NumTargets(), 1000);
    }

    TEST(QuadTreeTests, InsertWhenMultipleActorsOverlapQuadrantsCountsActorsOnceEach)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;

        for (int i = 0; i < 10; i++)
        {
            tree.Insert(pool.MakeEntry(BoundingBox2D(Vector2(-10.0f, -15.0f), Vector2(10.0f, 15.0f))));
        }

        EXPECT_EQ(tree.NumTargets(), 10);
    }

    TEST(QuadTreeTests, RemoveWhenOnlyActorIsRemovedLeavesTreeEmpty)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        QuadtreeData& data = pool.MakeEntry(BoundingBox2D::CreateFromPositionAndSize(Vector2(-5.0f, -7.0f), Vector2(10.0f, 15.0f)));

        tree.Insert(data);
        tree.Remove(data);

        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(QuadTreeTests, RemoveWhenTwoActorsAreRemovedUpdatesTargetCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        std::vector<QuadtreeData*> inserted;
        const int numTargets = 2;

        for (int i = 0; i < numTargets; i++)
        {
            QuadtreeData& data = pool.MakeEntry(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2::One));
            tree.Insert(data);
            inserted.push_back(&data);
        }

        int inTree = numTargets;
        EXPECT_EQ(tree.NumTargets(), inTree);

        for (QuadtreeData* data : inserted)
        {
            tree.Remove(*data);
            --inTree;
            EXPECT_EQ(tree.NumTargets(), inTree);
        }
    }

    TEST(QuadTreeTests, RemoveWhenThreeActorsAreRemovedUpdatesTargetCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        std::vector<QuadtreeData*> inserted;
        const int numTargets = 3;

        for (int i = 0; i < numTargets; i++)
        {
            QuadtreeData& data = pool.MakeEntry(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2::One));
            tree.Insert(data);
            inserted.push_back(&data);
        }

        int inTree = numTargets;
        EXPECT_EQ(tree.NumTargets(), inTree);

        for (QuadtreeData* data : inserted)
        {
            tree.Remove(*data);
            --inTree;
            EXPECT_EQ(tree.NumTargets(), inTree);
        }
    }

    TEST(QuadTreeTests, RemoveFromAllParentsWhenManyActorsAreRemovedUpdatesTargetCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        std::vector<QuadtreeData*> inserted;
        const int numTargets = 1000;

        for (int i = 0; i < numTargets; i++)
        {
            QuadtreeData& data = pool.MakeEntry(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2::One));
            tree.Insert(data);
            inserted.push_back(&data);
        }

        int inTree = numTargets;
        EXPECT_EQ(tree.NumTargets(), inTree);

        for (QuadtreeData* data : inserted)
        {
            data->RemoveFromAllParents();
            --inTree;
            EXPECT_EQ(tree.NumTargets(), inTree);
        }
    }

    TEST(QuadTreeTests, ShakeWhenTreeIsEmptyKeepsTargetCountAtZero)
    {
        QuadTree tree = CreateTree();
        tree.Shake();
        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(QuadTreeTests, ShakeWhenSplitTreeIsEmptyKeepsTargetCountAtZero)
    {
        QuadTree tree = CreateTree();
        tree.Split();
        tree.Shake();
        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(QuadTreeTests, ShakeWhenSplitTreeContainsActorKeepsTargetCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;

        tree.Split();
        tree.Insert(pool.MakeEntry());
        tree.Shake();

        EXPECT_EQ(tree.NumTargets(), 1);
    }

    TEST(QuadTreeTests, ShakeWhenTreeContainsOneActorKeepsTargetCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        const int numTargets = 1;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        tree.Shake();

        EXPECT_EQ(tree.NumTargets(), numTargets);
    }

    TEST(QuadTreeTests, ShakeWhenTreeContainsTwoActorsKeepsTargetCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        const int numTargets = 2;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        tree.Shake();

        EXPECT_EQ(tree.NumTargets(), numTargets);
    }

    TEST(QuadTreeTests, ShakeWhenTreeContainsThreeActorsKeepsTargetCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        const int numTargets = 3;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        tree.Shake();

        EXPECT_EQ(tree.NumTargets(), numTargets);
    }

    TEST(QuadTreeTests, ShakeWhenTreeContainsManyActorsKeepsTargetCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        const int numTargets = QuadTree::DefaultMaxObjectsPerNode + 1;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        tree.Shake();

        EXPECT_EQ(tree.NumTargets(), numTargets);
    }

    TEST(QuadTreeTests, QueryWhenTreeIsEmptyReturnsNoResults)
    {
        QuadTree tree = CreateTree();

        const std::vector<QuadtreeData*> query = tree.Query(kQuadTreeArea);

        EXPECT_TRUE(query.empty());
        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(QuadTreeTests, QueryWhenAreaDoesNotOverlapTreeReturnsNoResults)
    {
        QuadTree tree = CreateTree();
        const BoundingBox2D area(Vector2(100.0f, 100.0f), Vector2(101.0f, 101.0f));

        const std::vector<QuadtreeData*> query = tree.Query(area);

        EXPECT_TRUE(query.empty());
        EXPECT_EQ(tree.NumTargets(), 0);
    }

    TEST(QuadTreeTests, QueryWhenLeafNodeContainsActorReturnsActor)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;

        tree.Insert(pool.MakeEntry());

        const std::vector<QuadtreeData*> query = tree.Query(kQuadTreeArea);

        EXPECT_EQ(query.size(), 1u);
        EXPECT_EQ(static_cast<int>(query.size()), tree.NumTargets());
    }

    TEST(QuadTreeTests, QueryWhenLeafNodeDoesNotOverlapAreaReturnsNoResults)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        const BoundingBox2D area(Vector2(100.0f, 100.0f), Vector2(101.0f, 101.0f));

        tree.Insert(pool.MakeEntry());

        const std::vector<QuadtreeData*> query = tree.Query(area);

        EXPECT_TRUE(query.empty());
    }

    TEST(QuadTreeTests, QueryWhenLeafNodeContainsMultipleActorsReturnsAllActors)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        const int numTargets = QuadTree::DefaultMaxObjectsPerNode;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        const std::vector<QuadtreeData*> query = tree.Query(kQuadTreeArea);

        EXPECT_EQ(static_cast<int>(query.size()), numTargets);
        EXPECT_EQ(static_cast<int>(query.size()), tree.NumTargets());
    }

    TEST(QuadTreeTests, QueryWhenNonLeafTreeContainsManyActorsReturnsAllActors)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        const int numTargets = 2 * QuadTree::DefaultMaxObjectsPerNode;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        const std::vector<QuadtreeData*> query = tree.Query(kQuadTreeArea);

        EXPECT_EQ(static_cast<int>(query.size()), numTargets);
        EXPECT_EQ(static_cast<int>(query.size()), tree.NumTargets());
    }

    TEST(QuadTreeTests, QueryWhenCalledTwiceConsecutivelyReturnsSameResultCount)
    {
        QuadTree tree = CreateTree();
        EntryPool pool;
        const int numTargets = 2 * QuadTree::DefaultMaxObjectsPerNode;

        for (int i = 0; i < numTargets; i++)
        {
            tree.Insert(pool.MakeEntry());
        }

        const std::vector<QuadtreeData*> query1 = tree.Query(kQuadTreeArea);
        const std::vector<QuadtreeData*> query2 = tree.Query(kQuadTreeArea);

        EXPECT_EQ(static_cast<int>(query1.size()), numTargets);
        EXPECT_EQ(static_cast<int>(query1.size()), tree.NumTargets());
        EXPECT_EQ(query1.size(), query2.size());
    }
}
