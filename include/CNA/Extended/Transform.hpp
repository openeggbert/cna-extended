// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Transform.cs. Upstream defines three related types in one
// file (TransformFlags, BaseTransform<TMatrix>, Transform2, Transform3); this header mirrors
// that grouping. Transform2 (BaseTransform<Matrix3x2>) is now fully ported (Matrix3x2 landed in
// Phase 1, "Matrix3x2, MatrixExtensions, Vector2Extensions"). Transform3 (BaseTransform<Matrix>)
// needs only CNA's existing Matrix/Quaternion/Vector3.
#pragma once

#include "CNA/Extended/IMovable.hpp"
#include "CNA/Extended/IRotatable.hpp"
#include "CNA/Extended/IScalable.hpp"
#include "CNA/Extended/Matrix3x2.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/MulticastAction.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Quaternion;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;

    /**
     * @brief Dirty-tracking flags for BaseTransform. Mirrors MonoGame.Extended's `internal`
     * TransformFlags enum; C++ has no assembly-level `internal` visibility, so this is a regular
     * type, not part of the class's public API contract even though it is visible.
     */
    enum class TransformFlags : std::uint8_t
    {
        None = 0,
        WorldMatrixIsDirty = 1 << 0,
        LocalMatrixIsDirty = 1 << 1,
        All = WorldMatrixIsDirty | LocalMatrixIsDirty
    };

    constexpr TransformFlags operator|(const TransformFlags a, const TransformFlags b)
    {
        return static_cast<TransformFlags>(static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b));
    }

    constexpr TransformFlags operator&(const TransformFlags a, const TransformFlags b)
    {
        return static_cast<TransformFlags>(static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b));
    }

    constexpr TransformFlags operator~(const TransformFlags a)
    {
        return static_cast<TransformFlags>(static_cast<std::uint8_t>(~static_cast<std::uint8_t>(a)));
    }

    constexpr TransformFlags& operator|=(TransformFlags& a, const TransformFlags b)
    {
        return a = a | b;
    }

    constexpr TransformFlags& operator&=(TransformFlags& a, const TransformFlags b)
    {
        return a = a & b;
    }

    /**
     * @brief Base class for the position, rotation, and scale of a game object in two or three
     * dimensions.
     * @tparam TMatrix The matrix type (Matrix3x2 for 2D, Matrix for 3D).
     *
     * Every game object has a transform which is used to store and manipulate the position,
     * rotation and scale of the object. Every transform can have a parent, which allows applying
     * position, rotation and scale to game objects hierarchically.
     *
     * This class should not be used directly; use Transform2 or Transform3.
     *
     * Code derived from the top answer at
     * http://gamedev.stackexchange.com/questions/113977/should-i-store-local-forward-right-up-vector-or-calculate-when-necessary
     */
    template <typename TMatrix>
    class BaseTransform
    {
    public:
        virtual ~BaseTransform()
        {
            setParentProperty(nullptr);
        }

        BaseTransform(const BaseTransform&) = delete;
        BaseTransform& operator=(const BaseTransform&) = delete;

        /** @brief Raised when the world (or local) matrix became dirty. */
        System::MulticastAction<> TransformBecameDirty;

        /** @brief Raised after the world (or local) matrix was re-calculated. */
        System::MulticastAction<> TranformUpdated;

        /** @brief Gets the model-to-local space matrix. */
        [[nodiscard]] TMatrix getLocalMatrixProperty()
        {
            recalculateLocalMatrixIfNecessary();
            return localMatrix_;
        }

        /** @brief Gets the local-to-world space matrix. */
        [[nodiscard]] TMatrix getWorldMatrixProperty()
        {
            recalculateWorldMatrixIfNecessary();
            return worldMatrix_;
        }

        /** @brief Gets the parent instance, or nullptr if this transform has no parent. */
        [[nodiscard]] BaseTransform* getParentProperty() const
        {
            return parent_;
        }

        /**
         * @brief Sets the parent instance. A non-null parent makes this instance inherit the
         * parent's position, rotation, and scale; null disables the inheritance.
         */
        void setParentProperty(BaseTransform* value)
        {
            if (parent_ == value)
            {
                return;
            }

            BaseTransform* oldParent = parent_;
            parent_ = value;
            onParentChanged(oldParent, value);
        }

        /** @brief Gets the model-to-local space matrix. */
        void GetLocalMatrix(TMatrix& matrix)
        {
            recalculateLocalMatrixIfNecessary();
            matrix = localMatrix_;
        }

        /** @brief Gets the local-to-world space matrix. */
        void GetWorldMatrix(TMatrix& matrix)
        {
            recalculateWorldMatrixIfNecessary();
            matrix = worldMatrix_;
        }

    protected:
        BaseTransform() = default;

        void LocalMatrixBecameDirty()
        {
            flags_ |= TransformFlags::LocalMatrixIsDirty;
        }

        void WorldMatrixBecameDirty()
        {
            flags_ |= TransformFlags::WorldMatrixIsDirty;
            TransformBecameDirty();
        }

        /** @brief Recomputes @p matrix from @p localMatrix and (if present) the parent's world matrix. */
        virtual void RecalculateWorldMatrix(TMatrix& localMatrix, TMatrix& matrix) = 0;

        /** @brief Recomputes the local-space @p matrix from this transform's position/rotation/scale. */
        virtual void RecalculateLocalMatrix(TMatrix& matrix) = 0;

    private:
        using AncestorSubscription = std::pair<BaseTransform*, typename System::MulticastAction<>::Token>;

        void onParentChanged(BaseTransform* oldParent, BaseTransform* newParent)
        {
            for (BaseTransform* p = oldParent; p != nullptr; p = p->parent_)
            {
                unsubscribeFrom(p);
            }

            for (BaseTransform* p = newParent; p != nullptr; p = p->parent_)
            {
                subscribeTo(p);
            }

            flags_ |= TransformFlags::All;
        }

        void subscribeTo(BaseTransform* ancestor)
        {
            const auto token = ancestor->TransformBecameDirty.Add([this]() { parentOnTransformBecameDirty(); });
            ancestorSubscriptions_.emplace_back(ancestor, token);
        }

        void unsubscribeFrom(BaseTransform* ancestor)
        {
            const auto it = std::find_if(ancestorSubscriptions_.begin(), ancestorSubscriptions_.end(),
                [ancestor](const AncestorSubscription& entry) { return entry.first == ancestor; });
            if (it != ancestorSubscriptions_.end())
            {
                ancestor->TransformBecameDirty.Remove(it->second);
                ancestorSubscriptions_.erase(it);
            }
        }

        void parentOnTransformBecameDirty()
        {
            flags_ |= TransformFlags::All;
        }

        void recalculateWorldMatrixIfNecessary()
        {
            if ((flags_ & TransformFlags::WorldMatrixIsDirty) == TransformFlags::None)
            {
                return;
            }

            recalculateLocalMatrixIfNecessary();
            RecalculateWorldMatrix(localMatrix_, worldMatrix_);

            flags_ &= ~TransformFlags::WorldMatrixIsDirty;
            TranformUpdated();
        }

        void recalculateLocalMatrixIfNecessary()
        {
            if ((flags_ & TransformFlags::LocalMatrixIsDirty) == TransformFlags::None)
            {
                return;
            }

            RecalculateLocalMatrix(localMatrix_);

            flags_ &= ~TransformFlags::LocalMatrixIsDirty;
            WorldMatrixBecameDirty();
        }

        TransformFlags flags_ = TransformFlags::All; // dirty flags, set all dirty flags when created
        TMatrix localMatrix_{};                      // model space to local space
        TMatrix worldMatrix_{};                      // local space to world space
        BaseTransform* parent_ = nullptr;
        std::vector<AncestorSubscription> ancestorSubscriptions_;
    };

    /**
     * @brief Represents the position, rotation, and scale of a two-dimensional game object.
     *
     * Every game object has a transform which is used to store and manipulate the position,
     * rotation and scale of the object. Every transform can have a parent, which allows applying
     * position, rotation and scale to game objects hierarchically.
     */
    class Transform2 final : public BaseTransform<Matrix3x2>, public IMovable, public IRotatable, public IScalable
    {
    public:
        explicit Transform2(const std::optional<Vector2>& position = std::nullopt, float rotation = 0.0f,
            const std::optional<Vector2>& scale = std::nullopt);

        Transform2(float x, float y, float rotation = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f);

        /** @brief Gets the world position. */
        [[nodiscard]] Vector2 getWorldPositionProperty();

        /** @brief Gets the world scale. */
        [[nodiscard]] Vector2 getWorldScaleProperty();

        /** @brief Gets the world rotation angle in radians. */
        [[nodiscard]] float getWorldRotationProperty();

        /** @brief Gets the local position. */
        [[nodiscard]] Vector2 getPositionProperty() const override { return position_; }
        /** @brief Sets the local position. */
        void setPositionProperty(const Vector2& value) override;

        /** @brief Gets the local rotation angle in radians. */
        [[nodiscard]] float getRotationProperty() const override { return rotation_; }
        /** @brief Sets the local rotation angle in radians. */
        void setRotationProperty(float value) override;

        /** @brief Gets the local scale. */
        [[nodiscard]] Vector2 getScaleProperty() const override { return scale_; }
        /** @brief Sets the local scale. */
        void setScaleProperty(const Vector2& value) override;

        [[nodiscard]] std::string ToString() const;

    protected:
        void RecalculateWorldMatrix(Matrix3x2& localMatrix, Matrix3x2& matrix) override;
        void RecalculateLocalMatrix(Matrix3x2& matrix) override;

    private:
        Vector2 position_;
        float rotation_ = 0.0f;
        Vector2 scale_ = Vector2::One;
    };

    /**
     * @brief Represents the position, rotation, and scale of a three-dimensional game object.
     *
     * Every game object has a transform which is used to store and manipulate the position,
     * rotation and scale of the object. Every transform can have a parent, which allows applying
     * position, rotation and scale to game objects hierarchically.
     */
    class Transform3 final : public BaseTransform<Matrix>
    {
    public:
        explicit Transform3(const Vector3& position = Vector3::Zero,
            const Quaternion& rotation = Quaternion::Identity, const Vector3& scale = Vector3::One);

        /** @brief Gets the world position. */
        [[nodiscard]] Vector3 getWorldPositionProperty();

        /** @brief Gets the world scale. */
        [[nodiscard]] Vector3 getWorldScaleProperty();

        /** @brief Gets the world rotation quaternion. */
        [[nodiscard]] Quaternion getWorldRotationProperty();

        /** @brief Gets the local position. */
        [[nodiscard]] Vector3 getPositionProperty() const { return position_; }

        /** @brief Sets the local position. */
        void setPositionProperty(const Vector3& value);

        /** @brief Gets the local rotation quaternion. */
        [[nodiscard]] Quaternion getRotationProperty() const { return rotation_; }

        /** @brief Sets the local rotation quaternion. */
        void setRotationProperty(const Quaternion& value);

        /** @brief Gets the local scale. */
        [[nodiscard]] Vector3 getScaleProperty() const { return scale_; }

        /** @brief Sets the local scale. */
        void setScaleProperty(const Vector3& value);

        [[nodiscard]] std::string ToString() const;

    protected:
        void RecalculateWorldMatrix(Matrix& localMatrix, Matrix& matrix) override;
        void RecalculateLocalMatrix(Matrix& matrix) override;

    private:
        Vector3 position_;
        Quaternion rotation_;
        Vector3 scale_;
    };
}
