// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for these marker interfaces (they are
// plain property contracts with no behavior of their own). These tests only confirm the
// interfaces are implementable and that the getX/setX Property() pattern round-trips.
//
// ISizable is declared in this phase but still not exercised here: it returns SizeF, ported
// later in Phase 1 (task 19, "Size, SizeF, Interval, Thickness"). Its round-trip test is
// appended to this file once that type exists.
//
// IRectangularF WAS deferred for the same reason (needed RectangleF) but is no longer blocked:
// RectangleF landed in Phase 1 task 17 ("RectangleF family"), so its round-trip test below.
#include "CNA/Extended/IColorable.hpp"
#include "CNA/Extended/IEquatableByRef.hpp"
#include "CNA/Extended/IMovable.hpp"
#include "CNA/Extended/IRectangular.hpp"
#include "CNA/Extended/IRotatable.hpp"
#include "CNA/Extended/IScalable.hpp"
#include "CNA/Extended/ISizable.hpp"
#include "CNA/Extended/RectangleF.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    namespace
    {
        class MovableThing final : public IMovable
        {
        public:
            [[nodiscard]] Vector2 getPositionProperty() const override { return position_; }
            void setPositionProperty(const Vector2& value) override { position_ = value; }

        private:
            Vector2 position_{};
        };

        class RotatableThing final : public IRotatable
        {
        public:
            [[nodiscard]] float getRotationProperty() const override { return rotation_; }
            void setRotationProperty(const float value) override { rotation_ = value; }

        private:
            float rotation_ = 0.0f;
        };

        class ScalableThing final : public IScalable
        {
        public:
            [[nodiscard]] Vector2 getScaleProperty() const override { return scale_; }
            void setScaleProperty(const Vector2& value) override { scale_ = value; }

        private:
            Vector2 scale_{};
        };

        class ColorableThing final : public IColorable
        {
        public:
            [[nodiscard]] Color getColorProperty() const override { return color_; }
            void setColorProperty(const Color& value) override { color_ = value; }

        private:
            Color color_{0, 0, 0, 0};
        };

        class RectangularThing final : public IRectangular
        {
        public:
            [[nodiscard]] Rectangle getBoundingRectangleProperty() const override
            {
                return Rectangle(1, 2, 3, 4);
            }
        };

        class RectangularFThing final : public IRectangularF
        {
        public:
            [[nodiscard]] RectangleF getBoundingRectangleProperty() const override
            {
                return RectangleF(1.0f, 2.0f, 3.0f, 4.0f);
            }
        };

        struct Comparable final : IEquatableByRef<Comparable>
        {
            int value = 0;

            bool Equals(Comparable& other) override { return value == other.value; }
        };
    }

    TEST(MarkerInterfaces, IMovableRoundTrips)
    {
        MovableThing thing;
        thing.setPositionProperty(Vector2(1.0f, 2.0f));
        EXPECT_FLOAT_EQ(thing.getPositionProperty().X, 1.0f);
        EXPECT_FLOAT_EQ(thing.getPositionProperty().Y, 2.0f);
    }

    TEST(MarkerInterfaces, IRotatableRoundTrips)
    {
        RotatableThing thing;
        thing.setRotationProperty(1.5f);
        EXPECT_FLOAT_EQ(thing.getRotationProperty(), 1.5f);
    }

    TEST(MarkerInterfaces, IScalableRoundTrips)
    {
        ScalableThing thing;
        thing.setScaleProperty(Vector2(2.0f, 3.0f));
        EXPECT_FLOAT_EQ(thing.getScaleProperty().X, 2.0f);
        EXPECT_FLOAT_EQ(thing.getScaleProperty().Y, 3.0f);
    }

    TEST(MarkerInterfaces, IColorableRoundTrips)
    {
        ColorableThing thing;
        thing.setColorProperty(Color(10, 20, 30, 255));
        EXPECT_EQ(thing.getColorProperty().getRProperty(), 10);
    }

    TEST(MarkerInterfaces, IRectangularExposesBoundingRectangle)
    {
        RectangularThing thing;
        const Rectangle rect = thing.getBoundingRectangleProperty();
        EXPECT_EQ(rect.X, 1);
        EXPECT_EQ(rect.Y, 2);
    }

    TEST(MarkerInterfaces, IRectangularFExposesBoundingRectangle)
    {
        RectangularFThing thing;
        const RectangleF rect = thing.getBoundingRectangleProperty();
        EXPECT_FLOAT_EQ(rect.X, 1.0f);
        EXPECT_FLOAT_EQ(rect.Y, 2.0f);
        EXPECT_FLOAT_EQ(rect.Width, 3.0f);
        EXPECT_FLOAT_EQ(rect.Height, 4.0f);
    }

    TEST(MarkerInterfaces, IEquatableByRefComparesByReference)
    {
        Comparable a;
        a.value = 5;
        Comparable b;
        b.value = 5;
        Comparable c;
        c.value = 6;

        EXPECT_TRUE(a.Equals(b));
        EXPECT_FALSE(a.Equals(c));
    }
}
