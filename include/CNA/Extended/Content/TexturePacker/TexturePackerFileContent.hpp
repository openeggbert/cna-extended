// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Content/TexturePacker/TexturePackerFileContent.cs. Upstream's
// `record`s with `[property: JsonPropertyName(...)]` positional properties -> get-only
// getXProperty() classes here (matching this project's C# read-only-property convention), each
// with a `from_json` free function providing the custom JSON-key-name mapping that
// `JsonPropertyName` supplies in C# (sharp-runtime's `JsonSerializer::Deserialize<T>` uses
// nlohmann's ADL `from_json` customization point in place of reflection -- see
// `System/Text/Json/JsonSerializer.hpp`'s own header comment). Only `from_json` is provided, not
// `to_json`: this format is never written by this project (upstream's writer,
// `TexturePackerWriter.cs`, lives in the excluded `MonoGame.Extended.Content.Pipeline` assembly).
#pragma once

#include "System/Collections/Generic/Dictionary.hpp"
#include "nlohmann/json.hpp"

#include <optional>
#include <string>
#include <vector>

namespace CNA::Extended::Content::TexturePacker
{
    /** @brief A single (x, y) integer point, as found in a TexturePacker JSON "offset"/"pivot" (int) field. */
    class TexturePackerPoint
    {
    public:
        TexturePackerPoint() = default;
        TexturePackerPoint(int x, int y) : x_(x), y_(y) {}

        [[nodiscard]] int getXProperty() const { return x_; }
        [[nodiscard]] int getYProperty() const { return y_; }

    private:
        int x_ = 0;
        int y_ = 0;

        friend void from_json(const nlohmann::ordered_json& j, TexturePackerPoint& value);
    };

    /** @brief A single (x, y) floating-point point, as found in a TexturePacker JSON "pivot" field. */
    class TexturePackerPointF
    {
    public:
        TexturePackerPointF() = default;
        TexturePackerPointF(double x, double y) : x_(x), y_(y) {}

        [[nodiscard]] double getXProperty() const { return x_; }
        [[nodiscard]] double getYProperty() const { return y_; }

    private:
        double x_ = 0.0;
        double y_ = 0.0;

        friend void from_json(const nlohmann::ordered_json& j, TexturePackerPointF& value);
    };

    /** @brief A (width, height) integer size, as found in a TexturePacker JSON "size" field. */
    class TexturePackerSize
    {
    public:
        TexturePackerSize() = default;
        TexturePackerSize(int width, int height) : width_(width), height_(height) {}

        [[nodiscard]] int getWidthProperty() const { return width_; }
        [[nodiscard]] int getHeightProperty() const { return height_; }

    private:
        int width_ = 0;
        int height_ = 0;

        friend void from_json(const nlohmann::ordered_json& j, TexturePackerSize& value);
    };

    /** @brief An (x, y, w, h) integer rectangle, as found in a TexturePacker JSON "frame"/"spriteSourceSize"/"scale9" field. */
    class TexturePackerRectangle
    {
    public:
        TexturePackerRectangle() = default;
        TexturePackerRectangle(int x, int y, int width, int height) : x_(x), y_(y), width_(width), height_(height) {}

        [[nodiscard]] int getXProperty() const { return x_; }
        [[nodiscard]] int getYProperty() const { return y_; }
        [[nodiscard]] int getWidthProperty() const { return width_; }
        [[nodiscard]] int getHeightProperty() const { return height_; }

    private:
        int x_ = 0;
        int y_ = 0;
        int width_ = 0;
        int height_ = 0;

        friend void from_json(const nlohmann::ordered_json& j, TexturePackerRectangle& value);
    };

    /** @brief One sub-image frame nested under a TexturePacker JSON "textures[].frames" hash (per-texture-map export format). */
    class TexturePackerTextureFrame
    {
    public:
        TexturePackerTextureFrame() = default;

        [[nodiscard]] const TexturePackerRectangle& getFrameProperty() const { return frame_; }
        [[nodiscard]] int getRotatedProperty() const { return rotated_; }
        [[nodiscard]] const std::optional<TexturePackerSize>& getSizeProperty() const { return size_; }
        [[nodiscard]] const std::optional<TexturePackerPoint>& getOffsetProperty() const { return offset_; }
        [[nodiscard]] const std::optional<TexturePackerPointF>& getPivotProperty() const { return pivot_; }
        [[nodiscard]] const std::optional<TexturePackerRectangle>& getScale9Property() const { return scale9_; }

    private:
        TexturePackerRectangle frame_;
        int rotated_ = 0;
        std::optional<TexturePackerSize> size_;
        std::optional<TexturePackerPoint> offset_;
        std::optional<TexturePackerPointF> pivot_;
        std::optional<TexturePackerRectangle> scale9_;

        friend void from_json(const nlohmann::ordered_json& j, TexturePackerTextureFrame& value);
    };

    /** @brief One entry of a TexturePacker JSON "textures" array (the per-texture-map export format). */
    class TexturePackerTexture
    {
    public:
        TexturePackerTexture() = default;

        [[nodiscard]] const std::string& getFileNameProperty() const { return fileName_; }
        [[nodiscard]] const std::optional<std::string>& getFormatProperty() const { return format_; }
        [[nodiscard]] const std::optional<TexturePackerSize>& getSizeProperty() const { return size_; }
        [[nodiscard]] const std::optional<std::string>& getScaleProperty() const { return scale_; }
        [[nodiscard]] bool getPremultipliedProperty() const { return premultiplied_; }
        [[nodiscard]] const std::optional<System::Collections::Generic::Dictionary<std::string, TexturePackerTextureFrame>>&
        getFramesProperty() const
        {
            return frames_;
        }

    private:
        std::string fileName_;
        std::optional<std::string> format_;
        // Upstream's `Size` property default is `= default` on a reference-type record
        // (TexturePackerSize is `record`, not `record struct`) -- so it is genuinely nullable at
        // runtime despite its non-nullable-looking C# type, matching Format/Scale/Frames below.
        std::optional<TexturePackerSize> size_;
        std::optional<std::string> scale_;
        bool premultiplied_ = false;
        std::optional<System::Collections::Generic::Dictionary<std::string, TexturePackerTextureFrame>> frames_;

        friend void from_json(const nlohmann::ordered_json& j, TexturePackerTexture& value);
    };

    /** @brief One entry of a TexturePacker JSON top-level "frames" array (the flat/legacy export format). */
    class TexturePackerFrame
    {
    public:
        TexturePackerFrame() = default;

        [[nodiscard]] const std::string& getFileNameProperty() const { return fileName_; }
        [[nodiscard]] const TexturePackerRectangle& getFrameProperty() const { return frame_; }
        [[nodiscard]] bool getRotatedProperty() const { return rotated_; }
        [[nodiscard]] bool getTrimmedProperty() const { return trimmed_; }
        [[nodiscard]] const TexturePackerRectangle& getSpriteSourceSizeProperty() const { return spriteSourceSize_; }
        [[nodiscard]] const TexturePackerSize& getSourceSizeProperty() const { return sourceSize_; }
        [[nodiscard]] const TexturePackerPointF& getPivotPointProperty() const { return pivotPoint_; }

    private:
        std::string fileName_;
        TexturePackerRectangle frame_;
        bool rotated_ = false;
        bool trimmed_ = false;
        TexturePackerRectangle spriteSourceSize_;
        TexturePackerSize sourceSize_;
        TexturePackerPointF pivotPoint_;

        friend void from_json(const nlohmann::ordered_json& j, TexturePackerFrame& value);
    };

    /** @brief The "meta" block of a TexturePacker JSON file. */
    class TexturePackerMeta
    {
    public:
        TexturePackerMeta() = default;

        [[nodiscard]] const std::string& getAppProperty() const { return app_; }
        [[nodiscard]] const std::string& getVersionProperty() const { return version_; }
        [[nodiscard]] const std::string& getImageProperty() const { return image_; }
        [[nodiscard]] const std::string& getDataFormatProperty() const { return dataFormat_; }
        [[nodiscard]] const std::string& getSmartUpdateProperty() const { return smartUpdate_; }

    private:
        std::string app_;
        std::string version_;
        std::string image_;
        std::string dataFormat_;
        std::string smartUpdate_;

        friend void from_json(const nlohmann::ordered_json& j, TexturePackerMeta& value);
    };

    /** @brief The root of a parsed TexturePacker JSON export file. */
    class TexturePackerFileContent
    {
    public:
        TexturePackerFileContent() = default;

        [[nodiscard]] const std::vector<TexturePackerFrame>& getRegionsProperty() const { return regions_; }
        [[nodiscard]] const std::vector<TexturePackerTexture>& getTexturesProperty() const { return textures_; }
        [[nodiscard]] const TexturePackerMeta& getMetaProperty() const { return meta_; }

    private:
        std::vector<TexturePackerFrame> regions_;
        std::vector<TexturePackerTexture> textures_;
        TexturePackerMeta meta_;

        friend void from_json(const nlohmann::ordered_json& j, TexturePackerFileContent& value);
    };

    void from_json(const nlohmann::ordered_json& j, TexturePackerPoint& value);
    void from_json(const nlohmann::ordered_json& j, TexturePackerPointF& value);
    void from_json(const nlohmann::ordered_json& j, TexturePackerSize& value);
    void from_json(const nlohmann::ordered_json& j, TexturePackerRectangle& value);
    void from_json(const nlohmann::ordered_json& j, TexturePackerTextureFrame& value);
    void from_json(const nlohmann::ordered_json& j, TexturePackerTexture& value);
    void from_json(const nlohmann::ordered_json& j, TexturePackerFrame& value);
    void from_json(const nlohmann::ordered_json& j, TexturePackerMeta& value);
    void from_json(const nlohmann::ordered_json& j, TexturePackerFileContent& value);
}
