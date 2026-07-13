// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/Texture2DRegion.cs. Notable translation decisions:
//  - Texture2DRegion is a C# reference type that ends up aliased from multiple independent
//    owners at once (a Texture2DAtlas indexes it by both position and name, a Sprite holds it,
//    a NinePatch holds nine of them, GetSubregion hands out fresh ones derived from an existing
//    one) and its mutable `Tag` field means identity (not just value) must be preserved across
//    all of those holders -- so every reference to a Texture2DRegion in this port is a
//    std::shared_ptr<Texture2DRegion>, not a value or a non-owning pointer, matching the
//    project's per-type ownership-reasoning discipline (see ScreenManager.hpp's Screen*/
//    Transition* split for the precedent of reasoning this explicitly rather than defaulting).
//  - `Texture2D Texture { get; }` is a non-owning reference to a GPU resource the region does not
//    own; stored as a raw `Texture2D*` (nullable -- the two test-only constructors below leave it
//    null), matching this project's established non-owning-pointer convention for externally
//    owned graphics resources (e.g. Graphics/Effects/ITextureEffect.hpp).
//  - `ArgumentNullException.ThrowIfNull(texture)` is dropped for the raw-pointer `texture`
//    parameter only where a reference type would make null unrepresentable; here `texture` is a
//    pointer (it can legitimately be null via the test-only constructors' code path), so the
//    check is kept as an explicit `std::invalid_argument` throw on the public texture-taking
//    constructors, matching ActorPairKey.hpp's precedent for pointer parameters.
//  - The `Texture2DRegion(Texture2D texture, string name)` constructor has a genuine upstream bug
//    preserved faithfully: its body ignores the `name` parameter entirely and always delegates
//    with `null`, so the name ends up defaulted from `texture.Name` regardless of what the caller
//    passed. This is exactly what Texture2DRegion.cs does; not a porting mistake.
//  - The two `internal` "used for unit tests only" constructors have no C++ visibility
//    equivalent; kept public and documented as implementation detail, matching
//    CollisionShapeKind2D.hpp/ActorPairKey.hpp's precedent elsewhere in this port.
#pragma once

#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "CNA/Extended/Size.hpp"
#include "System/Object.hpp"

#include <optional>
#include <string>

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    /** @brief Represents a region of a texture. */
    class Texture2DRegion
    {
    public:
        /**
         * @brief Initializes a region representing the entire texture.
         * @throws std::invalid_argument @p texture is null.
         * @throws System::ObjectDisposedException @p texture was already disposed.
         */
        explicit Texture2DRegion(Texture2D* texture);

        /**
         * @brief Initializes a region representing the entire texture. Matches upstream's
         * bugged overload: @p name is accepted but never used (see this file's header comment).
         * @throws std::invalid_argument @p texture is null.
         * @throws System::ObjectDisposedException @p texture was already disposed.
         */
        Texture2DRegion(Texture2D* texture, const std::string& name);

        /**
         * @throws std::invalid_argument @p texture is null.
         * @throws System::ObjectDisposedException @p texture was already disposed.
         */
        Texture2DRegion(Texture2D* texture, const Rectangle& region);

        /**
         * @throws std::invalid_argument @p texture is null.
         * @throws System::ObjectDisposedException @p texture was already disposed.
         */
        Texture2DRegion(Texture2D* texture, int x, int y, int width, int height);

        /**
         * @throws std::invalid_argument @p texture is null.
         * @throws System::ObjectDisposedException @p texture was already disposed.
         */
        Texture2DRegion(Texture2D* texture, const Rectangle& region, const std::string& name);

        /**
         * @throws std::invalid_argument @p texture is null.
         * @throws System::ObjectDisposedException @p texture was already disposed.
         */
        Texture2DRegion(Texture2D* texture, int x, int y, int width, int height, const std::string& name);

        /**
         * @brief Full constructor; all other public constructors delegate here.
         * @throws std::invalid_argument @p texture is null.
         * @throws System::ObjectDisposedException @p texture was already disposed.
         */
        Texture2DRegion(Texture2D* texture, int x, int y, int width, int height, bool isRotated,
            const Size& originalSize, const Vector2& offset, const std::optional<Vector2>& originNormalized,
            const std::string& name);

        /** @brief Test-only constructor (upstream: `internal`, "used for unit tests only"). No texture; UV coordinates are computed against 1.0f instead of texture dimensions. */
        Texture2DRegion(const std::string& name, const Rectangle& bounds);

        /** @brief Test-only constructor (upstream: `internal`, "used for unit tests only"). No texture; UV coordinates are computed against 1.0f instead of texture dimensions. */
        Texture2DRegion(const std::string& name, int x, int y, int width, int height);

        /** @brief Gets the name assigned to this texture region when it was created. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }

        /** @brief Gets the texture associated with this texture region, or nullptr (test-only constructors). */
        [[nodiscard]] Texture2D* getTextureProperty() const { return texture_; }

        /** @brief Gets the top-left x-coordinate of the texture region within the texture. */
        [[nodiscard]] int getXProperty() const { return x_; }

        /** @brief Gets the top-left y-coordinate of the texture region within the texture. */
        [[nodiscard]] int getYProperty() const { return y_; }

        /** @brief Gets the width, in pixels, of the texture region. */
        [[nodiscard]] int getWidthProperty() const { return width_; }

        /** @brief Gets the height, in pixels, of the texture region. */
        [[nodiscard]] int getHeightProperty() const { return height_; }

        /** @brief Gets the size of the texture region. */
        [[nodiscard]] Size getSizeProperty() const { return size_; }

        /** @brief Gets the user-defined data associated with this texture region. */
        [[nodiscard]] System::Object* getTagProperty() const { return tag_; }
        /** @brief Sets the user-defined data associated with this texture region. */
        void setTagProperty(System::Object* value) { tag_ = value; }

        /** @brief Gets the bounds of the texture region within the texture. */
        [[nodiscard]] Rectangle getBoundsProperty() const { return bounds_; }

        /** @brief Gets the top UV coordinate of the texture region. */
        [[nodiscard]] float getTopUVProperty() const { return topUV_; }

        /** @brief Gets the right UV coordinate of the texture region. */
        [[nodiscard]] float getRightUVProperty() const { return rightUV_; }

        /** @brief Gets the bottom UV coordinate of the texture region. */
        [[nodiscard]] float getBottomUVProperty() const { return bottomUV_; }

        /** @brief Gets the left UV coordinate of the texture region. */
        [[nodiscard]] float getLeftUVProperty() const { return leftUV_; }

        /** @brief Gets whether this texture region is rotated 90 degrees clockwise in the atlas. */
        [[nodiscard]] bool getIsRotatedProperty() const { return isRotated_; }

        /** @brief Gets the original size of the texture region before trimming. */
        [[nodiscard]] Size getOriginalSizeProperty() const { return originalSize_; }

        /** @brief Gets the offset between the top-left corner of the original sprite and the top-left corner of the trimmed sprite. */
        [[nodiscard]] Vector2 getOffsetProperty() const { return offset_; }

        /** @brief Gets the normalized origin point of the texture region, or empty if no origin is specified. */
        [[nodiscard]] const std::optional<Vector2>& getOriginNormalizedProperty() const { return originNormalized_; }

        [[nodiscard]] std::string ToString() const;

    private:
        std::string name_;
        Texture2D* texture_ = nullptr;
        int x_ = 0;
        int y_ = 0;
        int width_ = 0;
        int height_ = 0;
        Size size_;
        System::Object* tag_ = nullptr;
        Rectangle bounds_;
        float topUV_ = 0.0f;
        float rightUV_ = 0.0f;
        float bottomUV_ = 0.0f;
        float leftUV_ = 0.0f;
        bool isRotated_ = false;
        Size originalSize_;
        Vector2 offset_;
        std::optional<Vector2> originNormalized_;
    };
}
