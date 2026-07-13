// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapObjects/TilemapObject.cs.
#pragma once

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/Tilemaps/TilemapProperties.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief Abstract base for all tilemap objects (points, shapes, tiles, text) placed on an object layer. */
    class TilemapObject
    {
    public:
        virtual ~TilemapObject() = default;

        /** @brief Gets the unique identifier for this object. */
        [[nodiscard]] int getIdProperty() const { return id_; }

        /** @brief Gets/sets the name of the object. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        void setNameProperty(std::string value) { name_ = std::move(value); }

        /** @brief Gets/sets the class/type identifier for the object. */
        [[nodiscard]] const std::string& getClassProperty() const { return class_; }
        void setClassProperty(std::string value) { class_ = std::move(value); }

        /** @brief Gets/sets the position of the object in world coordinates. */
        [[nodiscard]] const Vector2& getPositionProperty() const { return position_; }
        void setPositionProperty(const Vector2& value) { position_ = value; }

        /** @brief Gets/sets the rotation of the object in radians. */
        [[nodiscard]] float getRotationProperty() const { return rotation_; }
        void setRotationProperty(float value) { rotation_ = value; }

        /** @brief Gets/sets whether the object is visible. */
        [[nodiscard]] bool getIsVisibleProperty() const { return isVisible_; }
        void setIsVisibleProperty(bool value) { isVisible_ = value; }

        /** @brief Gets the custom properties of the object. */
        [[nodiscard]] TilemapProperties& getPropertiesProperty() { return properties_; }
        [[nodiscard]] const TilemapProperties& getPropertiesProperty() const { return properties_; }

        /** @brief Gets the axis-aligned bounding box that encloses the object. */
        [[nodiscard]] virtual BoundingBox2D getBoundsProperty() const = 0;

    protected:
        TilemapObject(int id, const Vector2& position);

    private:
        int id_;
        std::string name_;
        std::string class_;
        Vector2 position_;
        float rotation_ = 0.0f;
        bool isVisible_ = true;
        TilemapProperties properties_;
    };
}
