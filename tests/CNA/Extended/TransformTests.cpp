// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for Transform.cs. These tests cover both
// BaseTransform<TMatrix> specializations (Transform2, Transform3) and the ancestor
// dirty-propagation mechanism in BaseTransform itself, since that is what motivated extending
// sharp-runtime's MulticastAction.
#include "CNA/Extended/Transform.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(Transform3Tests, DefaultsMatchUpstreamDefaults)
    {
        const Transform3 transform;
        EXPECT_EQ(transform.getPositionProperty(), Vector3::Zero);
        EXPECT_EQ(transform.getScaleProperty(), Vector3::One);
    }

    TEST(Transform3Tests, PositionRotationScaleRoundTrip)
    {
        Transform3 transform;
        transform.setPositionProperty(Vector3(1.0f, 2.0f, 3.0f));
        transform.setScaleProperty(Vector3(2.0f, 2.0f, 2.0f));

        EXPECT_EQ(transform.getPositionProperty(), Vector3(1.0f, 2.0f, 3.0f));
        EXPECT_EQ(transform.getScaleProperty(), Vector3(2.0f, 2.0f, 2.0f));
    }

    TEST(Transform3Tests, WorldMatrixWithNoParentEqualsLocalMatrix)
    {
        Transform3 transform;
        transform.setPositionProperty(Vector3(5.0f, 0.0f, 0.0f));

        EXPECT_EQ(transform.getWorldPositionProperty(), Vector3(5.0f, 0.0f, 0.0f));
    }

    TEST(Transform3Tests, WorldPositionCombinesParentAndLocalTranslation)
    {
        Transform3 parent;
        parent.setPositionProperty(Vector3(10.0f, 0.0f, 0.0f));

        Transform3 child;
        child.setParentProperty(&parent);
        child.setPositionProperty(Vector3(1.0f, 0.0f, 0.0f));

        EXPECT_EQ(child.getWorldPositionProperty(), Vector3(11.0f, 0.0f, 0.0f));
    }

    TEST(Transform3Tests, ChangingAncestorMarksDescendantWorldMatrixDirty)
    {
        // Exercises BaseTransform's ancestor-subscription mechanism (child subscribes to every
        // ancestor's TransformBecameDirty), the motivating case for MulticastAction::Add/Remove.
        Transform3 grandparent;
        Transform3 parent;
        parent.setParentProperty(&grandparent);
        Transform3 child;
        child.setParentProperty(&parent);

        // Settle all three transforms once.
        EXPECT_EQ(child.getWorldPositionProperty(), Vector3::Zero);

        grandparent.setPositionProperty(Vector3(7.0f, 0.0f, 0.0f));

        EXPECT_EQ(child.getWorldPositionProperty(), Vector3(7.0f, 0.0f, 0.0f));
    }

    TEST(Transform3Tests, ReparentingUnsubscribesFromTheOldAncestorChain)
    {
        Transform3 oldParent;
        Transform3 newParent;
        Transform3 child;

        child.setParentProperty(&oldParent);
        child.setParentProperty(&newParent); // re-parent: must unsubscribe from oldParent

        // Settle the child once under the new parent.
        EXPECT_EQ(child.getWorldPositionProperty(), Vector3::Zero);

        // Changing the old parent must no longer affect the child.
        oldParent.setPositionProperty(Vector3(100.0f, 0.0f, 0.0f));
        EXPECT_EQ(child.getWorldPositionProperty(), Vector3::Zero);

        // Changing the new parent must still affect the child.
        newParent.setPositionProperty(Vector3(3.0f, 0.0f, 0.0f));
        EXPECT_EQ(child.getWorldPositionProperty(), Vector3(3.0f, 0.0f, 0.0f));
    }

    TEST(Transform3Tests, TransformBecameDirtyFiresOnPropertyChange)
    {
        Transform3 transform;
        int dirtyCount = 0;
        transform.TransformBecameDirty += [&dirtyCount]() { ++dirtyCount; };

        transform.setPositionProperty(Vector3(1.0f, 0.0f, 0.0f));
        EXPECT_EQ(dirtyCount, 1);
    }

    TEST(Transform3Tests, ToStringIncludesPositionRotationScale)
    {
        Transform3 transform;
        const std::string text = transform.ToString();
        EXPECT_NE(text.find("Position"), std::string::npos);
        EXPECT_NE(text.find("Rotation"), std::string::npos);
        EXPECT_NE(text.find("Scale"), std::string::npos);
    }

    TEST(Transform2Tests, DefaultsMatchUpstreamDefaults)
    {
        const Transform2 transform;
        EXPECT_EQ(transform.getPositionProperty(), Vector2::Zero);
        EXPECT_FLOAT_EQ(transform.getRotationProperty(), 0.0f);
        EXPECT_EQ(transform.getScaleProperty(), Vector2::One);
    }

    TEST(Transform2Tests, FourArgConstructorMatchesUpstreamOverload)
    {
        const Transform2 transform(1.0f, 2.0f, 0.5f, 3.0f, 4.0f);
        EXPECT_EQ(transform.getPositionProperty(), Vector2(1.0f, 2.0f));
        EXPECT_FLOAT_EQ(transform.getRotationProperty(), 0.5f);
        EXPECT_EQ(transform.getScaleProperty(), Vector2(3.0f, 4.0f));
    }

    TEST(Transform2Tests, PositionRotationScaleRoundTrip)
    {
        Transform2 transform;
        transform.setPositionProperty(Vector2(1.0f, 2.0f));
        transform.setRotationProperty(1.5f);
        transform.setScaleProperty(Vector2(2.0f, 2.0f));

        EXPECT_EQ(transform.getPositionProperty(), Vector2(1.0f, 2.0f));
        EXPECT_FLOAT_EQ(transform.getRotationProperty(), 1.5f);
        EXPECT_EQ(transform.getScaleProperty(), Vector2(2.0f, 2.0f));
    }

    TEST(Transform2Tests, WorldMatrixWithNoParentEqualsLocalMatrix)
    {
        Transform2 transform;
        transform.setPositionProperty(Vector2(5.0f, 0.0f));

        EXPECT_EQ(transform.getWorldPositionProperty(), Vector2(5.0f, 0.0f));
    }

    TEST(Transform2Tests, WorldPositionCombinesParentAndLocalTranslation)
    {
        Transform2 parent;
        parent.setPositionProperty(Vector2(10.0f, 0.0f));

        Transform2 child;
        child.setParentProperty(&parent);
        child.setPositionProperty(Vector2(1.0f, 0.0f));

        EXPECT_EQ(child.getWorldPositionProperty(), Vector2(11.0f, 0.0f));
    }

    TEST(Transform2Tests, ChangingAncestorMarksDescendantWorldMatrixDirty)
    {
        Transform2 grandparent;
        Transform2 parent;
        parent.setParentProperty(&grandparent);
        Transform2 child;
        child.setParentProperty(&parent);

        EXPECT_EQ(child.getWorldPositionProperty(), Vector2::Zero);

        grandparent.setPositionProperty(Vector2(7.0f, 0.0f));

        EXPECT_EQ(child.getWorldPositionProperty(), Vector2(7.0f, 0.0f));
    }

    TEST(Transform2Tests, IMovableIRotatableIScalableInterfacesAreImplemented)
    {
        // Transform2, unlike Transform3, implements IMovable/IRotatable/IScalable upstream.
        Transform2 transform;
        IMovable& movable = transform;
        IRotatable& rotatable = transform;
        IScalable& scalable = transform;

        movable.setPositionProperty(Vector2(3.0f, 4.0f));
        rotatable.setRotationProperty(0.25f);
        scalable.setScaleProperty(Vector2(5.0f, 6.0f));

        EXPECT_EQ(transform.getPositionProperty(), Vector2(3.0f, 4.0f));
        EXPECT_FLOAT_EQ(transform.getRotationProperty(), 0.25f);
        EXPECT_EQ(transform.getScaleProperty(), Vector2(5.0f, 6.0f));
    }

    TEST(Transform2Tests, ToStringIncludesPositionRotationScale)
    {
        Transform2 transform;
        const std::string text = transform.ToString();
        EXPECT_NE(text.find("Position"), std::string::npos);
        EXPECT_NE(text.find("Rotation"), std::string::npos);
        EXPECT_NE(text.find("Scale"), std::string::npos);
    }
}
