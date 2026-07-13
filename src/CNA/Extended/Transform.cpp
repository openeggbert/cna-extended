// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Transform.hpp"

namespace CNA::Extended
{
    Transform2::Transform2(const std::optional<Vector2>& position, const float rotation, const std::optional<Vector2>& scale)
        : position_(position.value_or(Vector2::Zero)), rotation_(rotation), scale_(scale.value_or(Vector2::One))
    {
    }

    Transform2::Transform2(const float x, const float y, const float rotation, const float scaleX, const float scaleY)
        : Transform2(Vector2(x, y), rotation, Vector2(scaleX, scaleY))
    {
    }

    Vector2 Transform2::getWorldPositionProperty()
    {
        Vector2 translation;
        float unusedRotation = 0.0f;
        Vector2 unusedScale;
        getWorldMatrixProperty().Decompose(translation, unusedRotation, unusedScale);
        return translation;
    }

    Vector2 Transform2::getWorldScaleProperty()
    {
        Vector2 unusedTranslation;
        float unusedRotation = 0.0f;
        Vector2 scale;
        getWorldMatrixProperty().Decompose(unusedTranslation, unusedRotation, scale);
        return scale;
    }

    float Transform2::getWorldRotationProperty()
    {
        Vector2 unusedTranslation;
        float rotation = 0.0f;
        Vector2 unusedScale;
        getWorldMatrixProperty().Decompose(unusedTranslation, rotation, unusedScale);
        return rotation;
    }

    void Transform2::setPositionProperty(const Vector2& value)
    {
        position_ = value;
        LocalMatrixBecameDirty();
        WorldMatrixBecameDirty();
    }

    void Transform2::setRotationProperty(const float value)
    {
        rotation_ = value;
        LocalMatrixBecameDirty();
        WorldMatrixBecameDirty();
    }

    void Transform2::setScaleProperty(const Vector2& value)
    {
        scale_ = value;
        LocalMatrixBecameDirty();
        WorldMatrixBecameDirty();
    }

    void Transform2::RecalculateWorldMatrix(Matrix3x2& localMatrix, Matrix3x2& matrix)
    {
        if (getParentProperty() != nullptr)
        {
            getParentProperty()->GetWorldMatrix(matrix);
            Matrix3x2::Multiply(localMatrix, matrix, matrix);
        }
        else
        {
            matrix = localMatrix;
        }
    }

    void Transform2::RecalculateLocalMatrix(Matrix3x2& matrix)
    {
        matrix = Matrix3x2::CreateScale(scale_) * Matrix3x2::CreateRotationZ(rotation_) * Matrix3x2::CreateTranslation(position_);
    }

    std::string Transform2::ToString() const
    {
        return "Position: " + getPositionProperty().ToString() + ", Rotation: " + std::to_string(getRotationProperty()) +
               ", Scale: " + getScaleProperty().ToString();
    }

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
