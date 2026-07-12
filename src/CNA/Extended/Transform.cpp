// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Transform.hpp"

namespace CNA::Extended
{
    Transform3::Transform3(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
        : position_(position), rotation_(rotation), scale_(scale)
    {
    }

    Vector3 Transform3::getWorldPositionProperty()
    {
        return getWorldMatrixProperty().getTranslationProperty();
    }

    Vector3 Transform3::getWorldScaleProperty()
    {
        Vector3 scale = Vector3::Zero;
        Quaternion rotation = Quaternion::Identity;
        Vector3 translation = Vector3::Zero;
        [[maybe_unused]] const bool decomposed = getWorldMatrixProperty().Decompose(scale, rotation, translation);
        return scale;
    }

    Quaternion Transform3::getWorldRotationProperty()
    {
        Vector3 scale = Vector3::Zero;
        Quaternion rotation = Quaternion::Identity;
        Vector3 translation = Vector3::Zero;
        [[maybe_unused]] const bool decomposed = getWorldMatrixProperty().Decompose(scale, rotation, translation);
        return rotation;
    }

    void Transform3::setPositionProperty(const Vector3& value)
    {
        position_ = value;
        LocalMatrixBecameDirty();
        WorldMatrixBecameDirty();
    }

    void Transform3::setRotationProperty(const Quaternion& value)
    {
        rotation_ = value;
        LocalMatrixBecameDirty();
        WorldMatrixBecameDirty();
    }

    void Transform3::setScaleProperty(const Vector3& value)
    {
        scale_ = value;
        LocalMatrixBecameDirty();
        WorldMatrixBecameDirty();
    }

    void Transform3::RecalculateWorldMatrix(Matrix& localMatrix, Matrix& matrix)
    {
        if (getParentProperty() != nullptr)
        {
            getParentProperty()->GetWorldMatrix(matrix);
            matrix = Matrix::Multiply(localMatrix, matrix);
        }
        else
        {
            matrix = localMatrix;
        }
    }

    void Transform3::RecalculateLocalMatrix(Matrix& matrix)
    {
        matrix = Matrix::CreateScale(scale_) * Matrix::CreateFromQuaternion(rotation_) *
                 Matrix::CreateTranslation(position_);
    }

    std::string Transform3::ToString() const
    {
        return "Position: " + getPositionProperty().ToString() + ", Rotation: " + getRotationProperty().ToString() +
               ", Scale: " + getScaleProperty().ToString();
    }
}
