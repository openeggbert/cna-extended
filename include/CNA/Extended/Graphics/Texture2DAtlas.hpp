// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/Texture2DAtlas.cs. `IEnumerable<Texture2DRegion>` ->
// plain begin()/end() over the index-ordered region list, matching this project's first use of
// this exact translation (no prior IEnumerable<T>-implementing type existed to establish a
// precedent for). The dual `_regionsByIndex: List<T>` / `_regionsByName: Dictionary<string, T>`
// storage is kept as-is (a std::vector alongside a System::Collections::Generic::Dictionary)
// rather than routed through this project's own Collections::KeyedCollection<TKey,TValue>, since
// KeyedCollection has no index-based access and upstream itself does not use its own
// KeyedCollection for this type either. `params int[]`/`params string[]` -> `const
// std::vector<int>&`/`const std::vector<std::string>&`. The `internal Texture2DRegion[]
// GetRegions(ReadOnlySpan<IAnimationFrame> frames)` overload (used nowhere else in
// MonoGame.Extended itself) is intentionally not ported in this pass -- it is dead code upstream
// (grep confirms zero call sites outside this file) and porting it would pull an
// Animations-module dependency into this header for no exercised behavior; add it later if a real
// caller needs it.
#pragma once

#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "System/Collections/Generic/Dictionary.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    class Sprite;

    /** @brief Represents a 2D texture atlas: a large image containing a collection of named/indexed sub-image regions. */
    class Texture2DAtlas
    {
    public:
        /**
         * @brief Initializes an atlas with a generated name (`"{texture.Name}Atlas"`).
         * @throws std::invalid_argument @p texture is null.
         * @throws System::ObjectDisposedException @p texture was already disposed.
         */
        explicit Texture2DAtlas(Texture2D* texture);

        /**
         * @throws std::invalid_argument @p texture is null.
         * @throws System::ObjectDisposedException @p texture was already disposed.
         */
        Texture2DAtlas(const std::string& name, Texture2D* texture);

        /** @brief Gets the name of the texture atlas. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }

        /** @brief Gets the underlying 2D texture. */
        [[nodiscard]] Texture2D* getTextureProperty() const { return texture_; }

        /** @brief Gets the number of regions in the atlas. */
        [[nodiscard]] int getRegionCountProperty() const { return static_cast<int>(regionsByIndex_.size()); }

        /**
         * @brief Gets the region at the specified index.
         * @throws std::out_of_range @p index is out of range.
         */
        [[nodiscard]] std::shared_ptr<Texture2DRegion> operator[](int index) const { return GetRegion(index); }

        /**
         * @brief Gets the region with the specified name.
         * @throws System::Collections::Generic::KeyNotFoundException no region named @p name exists.
         */
        [[nodiscard]] std::shared_ptr<Texture2DRegion> operator[](const std::string& name) const { return GetRegion(name); }

        /** @brief Creates a new unnamed texture region and adds it to this atlas. */
        std::shared_ptr<Texture2DRegion> CreateRegion(int x, int y, int width, int height);

        /**
         * @brief Creates a new named texture region and adds it to this atlas.
         * @throws std::invalid_argument a region named @p name already exists in this atlas.
         */
        std::shared_ptr<Texture2DRegion> CreateRegion(int x, int y, int width, int height, const std::string& name);

        /** @brief Creates a new unnamed texture region and adds it to this atlas. */
        std::shared_ptr<Texture2DRegion> CreateRegion(const Point& location, const Size& size);

        /**
         * @throws std::invalid_argument a region named @p name already exists in this atlas.
         */
        std::shared_ptr<Texture2DRegion> CreateRegion(const std::string& name, const Point& location, const Size& size);

        /** @brief Creates a new unnamed texture region and adds it to this atlas. */
        std::shared_ptr<Texture2DRegion> CreateRegion(const Rectangle& bounds);

        /**
         * @throws std::invalid_argument a region named @p name already exists in this atlas.
         */
        std::shared_ptr<Texture2DRegion> CreateRegion(const Rectangle& bounds, const std::string& name);

        /**
         * @throws std::invalid_argument a region named @p name already exists in this atlas.
         */
        std::shared_ptr<Texture2DRegion> CreateRegion(const Rectangle& bounds, bool isRotated, const Size& originalSize,
            const Vector2& trimOffset, const std::optional<Vector2>& originNormalized, const std::string& name);

        /** @brief Determines whether the atlas contains a region with the specified name. */
        [[nodiscard]] bool ContainsRegion(const std::string& name) const { return regionsByName_.ContainsKey(name); }

        /** @brief Gets the index of the region with the specified name, or -1 if not found. */
        [[nodiscard]] int GetIndexOfRegion(const std::string& name) const;

        /**
         * @throws std::out_of_range @p index is out of range.
         */
        [[nodiscard]] std::shared_ptr<Texture2DRegion> GetRegion(int index) const;

        /**
         * @throws System::Collections::Generic::KeyNotFoundException no region named @p name exists.
         */
        [[nodiscard]] std::shared_ptr<Texture2DRegion> GetRegion(const std::string& name) const { return regionsByName_[name]; }

        /** @brief Attempts to get the region at the specified index. */
        [[nodiscard]] bool TryGetRegion(int index, std::shared_ptr<Texture2DRegion>& region) const;

        /** @brief Attempts to get the region with the specified name. */
        [[nodiscard]] bool TryGetRegion(const std::string& name, std::shared_ptr<Texture2DRegion>& region) const
        {
            return regionsByName_.TryGetValue(name, region);
        }

        /**
         * @throws std::out_of_range any index in @p indexes is out of range.
         */
        [[nodiscard]] std::vector<std::shared_ptr<Texture2DRegion>> GetRegions(const std::vector<int>& indexes) const;

        /**
         * @throws System::Collections::Generic::KeyNotFoundException any name in @p names is not found.
         */
        [[nodiscard]] std::vector<std::shared_ptr<Texture2DRegion>> GetRegions(const std::vector<std::string>& names) const;

        /**
         * @throws std::out_of_range @p index is out of range.
         */
        bool RemoveRegion(int index);

        bool RemoveRegion(const std::string& name);

        /** @brief Removes all regions from the atlas. */
        void ClearRegions();

        /**
         * @brief Creates a new Sprite using the region from this atlas at the specified index.
         * @throws std::out_of_range @p regionIndex is out of range.
         */
        [[nodiscard]] Sprite CreateSprite(int regionIndex) const;

        /**
         * @brief Creates a new Sprite using the region from this atlas with the specified name.
         * @throws System::Collections::Generic::KeyNotFoundException no region named @p regionName exists.
         */
        [[nodiscard]] Sprite CreateSprite(const std::string& regionName) const;

        /** @brief Returns an iterator to the first region, in index order. */
        [[nodiscard]] std::vector<std::shared_ptr<Texture2DRegion>>::const_iterator begin() const { return regionsByIndex_.begin(); }
        /** @brief Returns an iterator past the last region. */
        [[nodiscard]] std::vector<std::shared_ptr<Texture2DRegion>>::const_iterator end() const { return regionsByIndex_.end(); }

        /**
         * @brief Creates a new atlas from @p texture by dividing it into a grid of regions, named
         * `"{name}_{index}"` in row-major (left-to-right, top-to-bottom) order.
         * @param maxRegionCount Maximum number of regions to create. Defaults to INT_MAX.
         * @param margin Margin, in pixels, to leave around the edges of the texture.
         * @param spacing Spacing, in pixels, between regions.
         * @throws std::invalid_argument @p texture is null.
         * @throws System::ObjectDisposedException @p texture was already disposed.
         */
        [[nodiscard]] static Texture2DAtlas Create(const std::string& name, Texture2D* texture, int regionWidth,
            int regionHeight, int maxRegionCount = 0x7fffffff, int margin = 0, int spacing = 0);

        /** @brief A rectangle/name pair computed by CalculateRegions (upstream: `internal readonly record struct CalculatedRegion`). */
        struct CalculatedRegion
        {
            Rectangle bounds;
            std::string name;
        };

        /**
         * @brief Computes the grid of regions Create() would carve out of a @p textureWidth x
         * @p textureHeight texture, without constructing an atlas. Exposed (upstream: `internal
         * static`) so this exact logic -- including the regression fix for issue #1013's
         * duplicate-name bug -- can be unit tested directly.
         */
        [[nodiscard]] static std::vector<CalculatedRegion> CalculateRegions(const std::string& atlasName, int textureWidth,
            int textureHeight, int regionWidth, int regionHeight, int maxRegionCount, int margin, int spacing);

        Texture2DAtlas(const Texture2DAtlas&) = delete;
        Texture2DAtlas& operator=(const Texture2DAtlas&) = delete;
        Texture2DAtlas(Texture2DAtlas&&) = default;
        Texture2DAtlas& operator=(Texture2DAtlas&&) = default;

    private:
        void AddRegion(const std::shared_ptr<Texture2DRegion>& region);
        bool RemoveRegionInternal(const std::shared_ptr<Texture2DRegion>& region);

        std::string name_;
        Texture2D* texture_ = nullptr;
        std::vector<std::shared_ptr<Texture2DRegion>> regionsByIndex_;
        System::Collections::Generic::Dictionary<std::string, std::shared_ptr<Texture2DRegion>> regionsByName_;
    };
}
