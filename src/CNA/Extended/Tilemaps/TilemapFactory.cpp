// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapFactory.hpp"

#include "CNA/Extended/Content/ExternalResourceResolvers.hpp"
#include "CNA/Extended/Tilemaps/Parsers/TilemapParseException.hpp"
#include "CNA/Extended/Tilemaps/TilemapDataLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapEllipseObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapImageLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapObjectLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapPointObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapPolygonObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapPolylineObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapRectangleObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapTextObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileObject.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/Convert.hpp"
#include "System/IO/MemoryStream.hpp"
#include "System/IO/Path.hpp"

#include <algorithm>
#include <cctype>

namespace CNA::Extended::Tilemaps
{
    using Parsers::TilemapParseException;

    namespace
    {
        // Loaded textures are handed to `tilemap.AddOwnedTexture(...)` for a stable, tilemap-
        // lifetime-scoped home -- see Tilemap.hpp's header comment. Every LoadTexture* /
        // Build* helper below therefore threads `Tilemap&` through, not just `GraphicsDevice&`.

        Texture2D* LoadTextureFromDataUri(const std::string& dataUri, GraphicsDevice& graphicsDevice, Tilemap& tilemap)
        {
            const std::size_t commaIndex = dataUri.find(',');
            if (commaIndex == std::string::npos)
            {
                throw TilemapParseException("Invalid data URI: missing comma separator.");
            }

            try
            {
                const std::vector<SharpRuntime::bytecs> bytes = System::Convert::FromBase64String(dataUri.substr(commaIndex + 1));
                System::IO::MemoryStream ms(bytes.data(), static_cast<SharpRuntime::intcs>(bytes.size()));
                auto texture = std::make_unique<Texture2D>(Texture2D::FromStream(graphicsDevice, ms));
                Texture2D* raw = texture.get();
                tilemap.AddOwnedTexture(std::move(texture));
                return raw;
            }
            catch (const TilemapParseException&)
            {
                throw;
            }
            catch (const std::exception&)
            {
                throw TilemapParseException("Failed to load texture from data URI.");
            }
        }

        bool StartsWithCaseInsensitive(const std::string& value, const std::string& prefix)
        {
            if (value.size() < prefix.size())
            {
                return false;
            }
            return std::equal(prefix.begin(), prefix.end(), value.begin(),
                [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
        }

        Texture2D* LoadTexture(const std::string& path, GraphicsDevice& graphicsDevice, const std::string& baseDirectory,
            const Content::ExternalResourceResolver& resourceResolver, Tilemap& tilemap)
        {
            // Support Ogmo-style embedded tileset images stored as base64 data URIs.
            if (StartsWithCaseInsensitive(path, "data:image"))
            {
                return LoadTextureFromDataUri(path, graphicsDevice, tilemap);
            }

            const std::string fullPath = System::IO::Path::IsPathRooted(path) ? path : System::IO::Path::Combine(baseDirectory, path);

            try
            {
                const std::unique_ptr<System::IO::Stream> stream = resourceResolver(fullPath);
                auto texture = std::make_unique<Texture2D>(Texture2D::FromStream(graphicsDevice, *stream));
                Texture2D* raw = texture.get();
                tilemap.AddOwnedTexture(std::move(texture));
                return raw;
            }
            catch (const std::exception&)
            {
                throw TilemapParseException("Failed to load texture: " + fullPath);
            }
        }

        void ApplyProperties(const std::vector<TilemapPropertyData>& source, TilemapProperties& target)
        {
            for (const TilemapPropertyData& prop : source)
            {
                if (prop.Key.empty())
                {
                    continue;
                }

                switch (prop.Type)
                {
                    case TilemapPropertyType::String:
                    case TilemapPropertyType::File:
                        target.SetString(prop.Key, prop.StringValue);
                        break;

                    case TilemapPropertyType::Int:
                    case TilemapPropertyType::Object:
                        target.SetInt(prop.Key, prop.IntValue);
                        break;

                    case TilemapPropertyType::Float:
                        target.SetFloat(prop.Key, prop.FloatValue);
                        break;

                    case TilemapPropertyType::Bool:
                        target.SetBool(prop.Key, prop.BoolValue);
                        break;

                    case TilemapPropertyType::Color:
                        target.SetColor(prop.Key, prop.ColorValue);
                        break;
                }
            }
        }

        std::unique_ptr<TilemapObject> BuildObject(TilemapObjectData* data)
        {
            if (data == nullptr)
            {
                return nullptr;
            }

            const Vector2 position(data->X, data->Y);
            std::unique_ptr<TilemapObject> obj;

            if (auto* r = dynamic_cast<TilemapRectangleObjectData*>(data))
            {
                obj = std::make_unique<TilemapRectangleObject>(r->Id, position, Vector2(r->Width, r->Height));
            }
            else if (auto* e = dynamic_cast<TilemapEllipseObjectData*>(data))
            {
                obj = std::make_unique<TilemapEllipseObject>(e->Id, position, Vector2(e->Width, e->Height));
            }
            else if (dynamic_cast<TilemapPointObjectData*>(data) != nullptr)
            {
                obj = std::make_unique<TilemapPointObject>(data->Id, position);
            }
            else if (auto* poly = dynamic_cast<TilemapPolygonObjectData*>(data))
            {
                obj = std::make_unique<TilemapPolygonObject>(data->Id, position, poly->Points);
            }
            else if (auto* line = dynamic_cast<TilemapPolylineObjectData*>(data))
            {
                obj = std::make_unique<TilemapPolylineObject>(data->Id, position, line->Points);
            }
            else if (auto* t = dynamic_cast<TilemapTileObjectData*>(data))
            {
                const TilemapTile tile(t->GlobalId, t->FlipFlags);
                obj = std::make_unique<TilemapTileObject>(t->Id, position, tile, Vector2(t->Width, t->Height));
            }
            else if (auto* txt = dynamic_cast<TilemapTextObjectData*>(data))
            {
                auto textObj = std::make_unique<TilemapTextObject>(txt->Id, position, Vector2(txt->Width, txt->Height), txt->Text);
                textObj->setFontFamilyProperty(txt->FontFamily.empty() ? "sans-serif" : txt->FontFamily);
                textObj->setPixelSizeProperty(txt->PixelSize);
                textObj->setWordWrapProperty(txt->WordWrap);
                textObj->setColorProperty(txt->Color);
                textObj->setBoldProperty(txt->Bold);
                textObj->setItalicProperty(txt->Italic);
                textObj->setUnderlineProperty(txt->Underline);
                textObj->setStrikethroughProperty(txt->Strikethrough);
                textObj->setHorizontalAlignProperty(txt->HorizontalAlign);
                textObj->setVerticalAlignProperty(txt->VerticalAlign);
                obj = std::move(textObj);
            }
            else
            {
                return nullptr;
            }

            obj->setNameProperty(data->Name);
            obj->setClassProperty(data->Class);
            obj->setRotationProperty(data->Rotation);
            obj->setIsVisibleProperty(data->IsVisible);

            ApplyProperties(data->Properties, obj->getPropertiesProperty());

            return obj;
        }

        std::unique_ptr<TilemapTileData> BuildTileData(TilemapTileEntryData& entryData, GraphicsDevice& graphicsDevice,
            const std::string& baseDirectory, const Content::ExternalResourceResolver& resourceResolver, Tilemap& tilemap)
        {
            auto tileData = std::make_unique<TilemapTileData>(entryData.LocalId);
            tileData->setClassProperty(entryData.Class);
            tileData->setProbabilityProperty(entryData.Probability);

            ApplyProperties(entryData.Properties, tileData->getPropertiesProperty());

            if (!entryData.ImagePath.empty())
            {
                try
                {
                    tileData->setCustomImageProperty(LoadTexture(entryData.ImagePath, graphicsDevice, baseDirectory, resourceResolver, tilemap));
                }
                catch (const TilemapParseException& ex)
                {
                    throw TilemapParseException(
                        "Failed to load image for tile " + std::to_string(entryData.LocalId) + ": " + ex.getMessageProperty());
                }
            }

            if (entryData.Animation.has_value() && !entryData.Animation->Frames.empty())
            {
                std::vector<TilemapTileAnimationFrame> frames;
                frames.reserve(entryData.Animation->Frames.size());
                for (const TilemapAnimationFrameData& frameData : entryData.Animation->Frames)
                {
                    frames.emplace_back(frameData.TileId, frameData.Duration);
                }
                tileData->setAnimationProperty(std::make_unique<TilemapTileAnimation>(std::move(frames)));
            }

            for (const std::unique_ptr<TilemapObjectData>& objData : entryData.CollisionObjects)
            {
                if (std::unique_ptr<TilemapObject> obj = BuildObject(objData.get()))
                {
                    tileData->getCollisionObjectsProperty().push_back(std::move(obj));
                }
            }

            return tileData;
        }

        std::unique_ptr<TilemapTileset> BuildTileset(TilemapTilesetEntry& entry, GraphicsDevice& graphicsDevice,
            const std::string& baseDirectory, const Content::ExternalResourceResolver& resourceResolver, Tilemap& tilemap)
        {
            TilemapTilesetData& tilesetData = *entry.InlineData;
            Texture2D* texture = nullptr;

            if (!tilesetData.TexturePath.empty())
            {
                try
                {
                    texture = LoadTexture(tilesetData.TexturePath, graphicsDevice, baseDirectory, resourceResolver, tilemap);
                }
                catch (const TilemapParseException& ex)
                {
                    throw TilemapParseException("Failed to load texture for tileset '" + tilesetData.Name + "': " + ex.getMessageProperty());
                }
            }

            auto tileset = std::make_unique<TilemapTileset>(tilesetData.Name, texture, tilesetData.TileWidth, tilesetData.TileHeight,
                tilesetData.TileCount, tilesetData.Columns, tilesetData.Spacing, tilesetData.Margin);

            tileset->setFirstGlobalIdProperty(entry.FirstGlobalId);
            tileset->setTileOffsetProperty(Vector2(tilesetData.DrawOffsetX, tilesetData.DrawOffsetY));

            ApplyProperties(tilesetData.Properties, tileset->getPropertiesProperty());

            for (TilemapTileEntryData& tileEntry : tilesetData.Tiles)
            {
                tileset->AddTileData(BuildTileData(tileEntry, graphicsDevice, baseDirectory, resourceResolver, tilemap));
            }

            return tileset;
        }

        void ApplyLayerBase(const TilemapLayerData& source, TilemapLayer& target)
        {
            target.setClassProperty(source.Class);
            target.setIsVisibleProperty(source.IsVisible);
            target.setOpacityProperty(source.Opacity);
            target.setTintColorProperty(source.TintColor);
            target.setParallaxFactorProperty(Vector2(source.ParallaxX, source.ParallaxY));

            // Chunked tile layers carry a chunk-origin offset in OffsetX/OffsetY in addition to
            // the editor-set offset. These are stored together in the DTO; apply as a single offset.
            target.setOffsetProperty(Vector2(source.OffsetX, source.OffsetY));

            ApplyProperties(source.Properties, target.getPropertiesProperty());
        }

        std::unique_ptr<TilemapTileLayer> BuildTileLayer(TilemapTileLayerData& data, const TilemapData& mapData)
        {
            auto layer = std::make_unique<TilemapTileLayer>(data.Name, data.Width, data.Height, mapData.TileWidth, mapData.TileHeight);

            for (const TilemapDecodedTile& tile : data.Tiles)
            {
                layer->SetTile(tile.X, tile.Y, TilemapTile(tile.GlobalId, tile.FlipFlags));
            }

            return layer;
        }

        std::unique_ptr<TilemapObjectLayer> BuildObjectLayer(TilemapObjectLayerData& data)
        {
            auto layer = std::make_unique<TilemapObjectLayer>(data.Name);
            layer->setDrawOrderProperty(data.DrawOrder);

            for (const std::unique_ptr<TilemapObjectData>& objData : data.Objects)
            {
                if (std::unique_ptr<TilemapObject> obj = BuildObject(objData.get()))
                {
                    layer->AddObject(std::move(obj));
                }
            }

            return layer;
        }

        std::unique_ptr<TilemapImageLayer> BuildImageLayer(TilemapImageLayerData& data, GraphicsDevice& graphicsDevice,
            const std::string& baseDirectory, const Content::ExternalResourceResolver& resourceResolver, Tilemap& tilemap)
        {
            if (data.TexturePath.empty())
            {
                return nullptr;
            }

            Texture2D* texture;

            try
            {
                texture = LoadTexture(data.TexturePath, graphicsDevice, baseDirectory, resourceResolver, tilemap);
            }
            catch (const TilemapParseException& ex)
            {
                throw TilemapParseException("Failed to load texture for image layer '" + data.Name + "': " + ex.getMessageProperty());
            }

            auto layer = std::make_unique<TilemapImageLayer>(data.Name, texture, Vector2::Zero);
            layer->setRepeatXProperty(data.RepeatX);
            layer->setRepeatYProperty(data.RepeatY);

            return layer;
        }

        std::unique_ptr<TilemapDataLayer> BuildDataLayer(const TilemapDataLayerData& data, const TilemapData& mapData)
        {
            return std::make_unique<TilemapDataLayer>(data.Name, data.Width, data.Height, mapData.TileWidth, mapData.TileHeight);
        }

        std::unique_ptr<TilemapLayer> BuildLayer(TilemapLayerData& layerData, const TilemapData& mapData, GraphicsDevice& graphicsDevice,
            const std::string& baseDirectory, const Content::ExternalResourceResolver& resourceResolver, Tilemap& tilemap)
        {
            std::unique_ptr<TilemapLayer> layer;

            if (auto* tileLayerData = dynamic_cast<TilemapTileLayerData*>(&layerData))
            {
                layer = BuildTileLayer(*tileLayerData, mapData);
            }
            else if (auto* objectLayerData = dynamic_cast<TilemapObjectLayerData*>(&layerData))
            {
                layer = BuildObjectLayer(*objectLayerData);
            }
            else if (auto* imageLayerData = dynamic_cast<TilemapImageLayerData*>(&layerData))
            {
                layer = BuildImageLayer(*imageLayerData, graphicsDevice, baseDirectory, resourceResolver, tilemap);
            }
            else if (auto* dataLayerData = dynamic_cast<TilemapDataLayerData*>(&layerData))
            {
                layer = BuildDataLayer(*dataLayerData, mapData);
            }
            else
            {
                return nullptr;
            }

            if (layer != nullptr)
            {
                ApplyLayerBase(layerData, *layer);
            }

            return layer;
        }

        void FlattenLayers(TilemapLayerData& layerData, const std::string& pathPrefix, const TilemapData& mapData, GraphicsDevice& graphicsDevice,
            const std::string& baseDirectory, const Content::ExternalResourceResolver& resourceResolver, Tilemap& tilemap)
        {
            if (auto* groupData = dynamic_cast<TilemapGroupLayerData*>(&layerData))
            {
                const std::string groupPath = pathPrefix.empty() ? groupData->Name : pathPrefix + "/" + groupData->Name;

                for (std::unique_ptr<TilemapLayerData>& child : groupData->Layers)
                {
                    FlattenLayers(*child, groupPath, mapData, graphicsDevice, baseDirectory, resourceResolver, tilemap);
                }
            }
            else
            {
                // Apply path prefix to the DTO name before building so the layer
                // is constructed with the full "Group/SubGroup/Layer" name.
                if (!pathPrefix.empty())
                {
                    layerData.Name = pathPrefix + "/" + layerData.Name;
                }

                if (std::unique_ptr<TilemapLayer> layer = BuildLayer(layerData, mapData, graphicsDevice, baseDirectory, resourceResolver, tilemap))
                {
                    tilemap.getLayersProperty().Add(std::move(layer));
                }
            }
        }
    }

    Tilemap Build(TilemapData& data, GraphicsDevice& graphicsDevice, const std::string& baseDirectory)
    {
        return Build(data, graphicsDevice, baseDirectory, Content::OpenFile);
    }

    Tilemap Build(TilemapData& data, GraphicsDevice& graphicsDevice, const std::string& baseDirectory,
        const Content::ExternalResourceResolver& resourceResolver)
    {
        Tilemap tilemap(data.Name.empty() ? "Untitled" : data.Name, data.Width, data.Height, data.TileWidth, data.TileHeight, data.Orientation);

        tilemap.setBackgroundColorProperty(data.BackgroundColor);
        tilemap.setParallaxOriginProperty(Vector2(data.ParallaxOriginX, data.ParallaxOriginY));
        tilemap.setWorldPositionProperty(Vector2(static_cast<float>(data.WorldX), static_cast<float>(data.WorldY)));
        tilemap.setWorldDepthProperty(data.WorldDepth);
        tilemap.setHexSideLengthProperty(data.HexSideLength);
        tilemap.setStaggerAxisProperty(data.StaggerAxis);
        tilemap.setStaggerIndexProperty(data.StaggerIndex);

        ApplyProperties(data.Properties, tilemap.getPropertiesProperty());

        for (TilemapTilesetEntry& entry : data.Tilesets)
        {
            if (entry.IsExternal || !entry.InlineData.has_value())
            {
                // External tilesets are resolved by the content pipeline; not expected at runtime.
                continue;
            }

            tilemap.getTilesetsProperty().Add(BuildTileset(entry, graphicsDevice, baseDirectory, resourceResolver, tilemap));
        }

        for (std::unique_ptr<TilemapLayerData>& layerData : data.Layers)
        {
            FlattenLayers(*layerData, "", data, graphicsDevice, baseDirectory, resourceResolver, tilemap);
        }

        return tilemap;
    }
}
