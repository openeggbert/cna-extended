// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/Collisions/Implementation/
// BasicActor.cs: a minimal ICollisionActor implementation shared across the QuadTree/QuadTreeSpace/
// SpatialHash/CollisionWorld2D upstream test suites. Test-only fixture, not part of the library --
// lives under tests/, header-only since it is simple and only ever included from .cpp test files
// (each translation unit gets its own instantiation, but the shared `static int s_nextId` upstream
// -> an `inline static` counter here, C++17's header-safe equivalent of a single shared static
// across translation units).
#pragma once

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/Collisions/ICollisionActor.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended::Collisions::Tests
{
    using Microsoft::Xna::Framework::Vector2;

    class BasicActor : public ICollisionActor
    {
    public:
        BasicActor() : id_(NextId())
        {
            SetBounds(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));
        }

        explicit BasicActor(const BoundingBox2D& bounds) : id_(NextId()) { SetBounds(bounds); }

        explicit BasicActor(const BoundingCircle2D& bounds) : id_(NextId()) { SetBounds(bounds); }

        explicit BasicActor(const OrientedBoundingBox2D& bounds) : id_(NextId()) { SetBounds(bounds); }

        [[nodiscard]] int getIdProperty() const override { return id_; }

        [[nodiscard]] Vector2 getPositionProperty() const { return position_; }

        [[nodiscard]] CollisionShape2D getShapeProperty() const override { return shape_; }

        void SetBounds(const BoundingBox2D& bounds)
        {
            position_ = bounds.Min;
            shape_ = CollisionShape2D(bounds);
        }

        void SetBounds(const BoundingCircle2D& bounds)
        {
            position_ = bounds.Center;
            shape_ = CollisionShape2D(bounds);
        }

        void SetBounds(const OrientedBoundingBox2D& bounds)
        {
            position_ = bounds.Center;
            shape_ = CollisionShape2D(bounds);
        }

    private:
        static int NextId()
        {
            static int nextId = 1;
            return nextId++;
        }

        int id_;
        Vector2 position_;
        CollisionShape2D shape_;
    };
}
