// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Tiled/TiledTmxParser.cs. Upstream deserializes TMX/TSX
// XML via `System.Xml.Serialization.XmlSerializer`, driven by reflection over [XmlAttribute]/
// [XmlElement]/[XmlText] attributes -- a genuine reflection dependency with no C++ equivalent (see
// TiledDocument.hpp's header comment). `DeserializeMap`/tileset parsing below hand-write the same
// element/attribute walk using the already-ported System::Xml DOM (XmlDocument/XmlElement)
// directly, producing the identical Document-model object graph XmlSerializer would have built.
#include "CNA/Extended/Tilemaps/Tiled/TiledTmxParser.hpp"

#include "CNA/Extended/Content/ExternalResourceResolvers.hpp"
#include "CNA/Extended/Tilemaps/Parsers/TilemapParseException.hpp"
#include "CNA/Extended/Tilemaps/Tiled/Converters/TiledTilemapDataConverter.hpp"
#include "CNA/Extended/Tilemaps/TilemapFactory.hpp"
#include "System/IO/File.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/IO/Path.hpp"
#include "System/Int32.hpp"
#include "System/Single.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlElement.hpp"
#include "System/Xml/XmlNodeList.hpp"

#include <filesystem>
#include <memory>

namespace CNA::Extended::Tilemaps::Tiled
{
    using Parsers::TilemapParseException;
    using System::Xml::XmlDocument;
    using System::Xml::XmlElement;
    using System::Xml::XmlNode;
    using System::Xml::XmlNodeList;
    using System::Xml::XmlNodeType;

    namespace
    {
        std::string ReadAllText(System::IO::Stream& stream)
        {
            const SharpRuntime::intcs length = stream.getLengthProperty();
            std::vector<SharpRuntime::bytecs> buffer(static_cast<std::size_t>(length));

            SharpRuntime::intcs totalRead = 0;
            while (totalRead < length)
            {
                const SharpRuntime::intcs read = stream.Read(buffer.data(), totalRead, length - totalRead);
                if (read == 0)
                {
                    break;
                }
                totalRead += read;
            }

            return std::string(buffer.begin(), buffer.begin() + totalRead);
        }

        // Direct-child-only element enumeration (unlike XmlElement::GetElementsByTagName, which is
        // recursive over all descendants -- wrong here, since e.g. a nested <group>'s own <layer>
        // children must not be picked up when reading the top-level map's direct <layer> children).
        std::vector<XmlElement*> ChildElements(const XmlElement* parent, const std::string& tagName)
        {
            std::vector<XmlElement*> result;
            if (parent == nullptr)
            {
                return result;
            }

            // getChildNodesProperty() returns a non-owning observer pointer: the XmlNodeList is
            // cached and owned by `parent` itself (its childNodesSnapshot_ member), recreated on
            // each call. Wrapping it in a unique_ptr here would double-free it -- the caller must
            // not take ownership.
            XmlNodeList* children = parent->getChildNodesProperty();
            for (SharpRuntime::intcs i = 0; i < children->getCountProperty(); ++i)
            {
                XmlNode* node = children->Item(i);
                if (node->getNodeTypeProperty() == XmlNodeType::Element && node->getNameProperty() == tagName)
                {
                    result.push_back(static_cast<XmlElement*>(node));
                }
            }
            return result;
        }

        XmlElement* FirstChildElement(const XmlElement* parent, const std::string& tagName)
        {
            const std::vector<XmlElement*> found = ChildElements(parent, tagName);
            return found.empty() ? nullptr : found.front();
        }

        // All direct child elements regardless of tag, for polymorphic layer lists.
        std::vector<XmlElement*> AllChildElements(const XmlElement* parent)
        {
            std::vector<XmlElement*> result;
            if (parent == nullptr)
            {
                return result;
            }

            // See ChildElements() above: non-owning pointer, do not wrap in unique_ptr.
            XmlNodeList* children = parent->getChildNodesProperty();
            for (SharpRuntime::intcs i = 0; i < children->getCountProperty(); ++i)
            {
                XmlNode* node = children->Item(i);
                if (node->getNodeTypeProperty() == XmlNodeType::Element)
                {
                    result.push_back(static_cast<XmlElement*>(node));
                }
            }
            return result;
        }

        int AttrInt(const XmlElement* el, const std::string& name, int defaultValue = 0)
        {
            const std::string value = el->GetAttribute(name);
            if (value.empty())
            {
                return defaultValue;
            }
            try
            {
                return System::Int32::Parse(value);
            }
            catch (const std::exception&)
            {
                return defaultValue;
            }
        }

        std::uint32_t AttrUInt32(const XmlElement* el, const std::string& name, std::uint32_t defaultValue = 0)
        {
            const std::string value = el->GetAttribute(name);
            if (value.empty())
            {
                return defaultValue;
            }
            try
            {
                return static_cast<std::uint32_t>(std::stoul(value));
            }
            catch (const std::exception&)
            {
                return defaultValue;
            }
        }

        float AttrFloat(const XmlElement* el, const std::string& name, float defaultValue = 0.0f)
        {
            const std::string value = el->GetAttribute(name);
            if (value.empty())
            {
                return defaultValue;
            }
            try
            {
                return System::Single::Parse(value);
            }
            catch (const std::exception&)
            {
                return defaultValue;
            }
        }

        std::string AttrString(const XmlElement* el, const std::string& name, const std::string& defaultValue = "")
        {
            const std::string value = el->GetAttribute(name);
            return value.empty() ? defaultValue : value;
        }

        std::unique_ptr<Document::TiledPropertiesXml> ParseProperties(const XmlElement* parent)
        {
            XmlElement* propsEl = FirstChildElement(parent, "properties");
            if (propsEl == nullptr)
            {
                return nullptr;
            }

            auto props = std::make_unique<Document::TiledPropertiesXml>();
            for (XmlElement* propEl : ChildElements(propsEl, "property"))
            {
                Document::TiledPropertyXml prop;
                prop.Name = AttrString(propEl, "name");
                prop.Type = AttrString(propEl, "type");
                prop.Value = AttrString(propEl, "value");
                props->Properties.push_back(std::move(prop));
            }
            return props;
        }

        std::unique_ptr<Document::TiledImageXml> ParseImage(const XmlElement* parent)
        {
            XmlElement* imgEl = FirstChildElement(parent, "image");
            if (imgEl == nullptr)
            {
                return nullptr;
            }

            auto image = std::make_unique<Document::TiledImageXml>();
            image->Source = AttrString(imgEl, "source");
            image->Width = AttrInt(imgEl, "width");
            image->Height = AttrInt(imgEl, "height");
            image->Trans = AttrString(imgEl, "trans");
            return image;
        }

        std::unique_ptr<Document::TiledObjectXml> ParseObject(const XmlElement* objEl)
        {
            auto obj = std::make_unique<Document::TiledObjectXml>();
            obj->Id = AttrInt(objEl, "id");
            obj->Name = AttrString(objEl, "name");
            obj->Type = AttrString(objEl, "type");
            obj->Class = AttrString(objEl, "class");
            obj->X = AttrFloat(objEl, "x");
            obj->Y = AttrFloat(objEl, "y");
            obj->Width = AttrFloat(objEl, "width");
            obj->Height = AttrFloat(objEl, "height");
            obj->Rotation = AttrFloat(objEl, "rotation");
            obj->Gid = AttrUInt32(objEl, "gid");
            obj->Visible = AttrInt(objEl, "visible", 1);
            obj->Properties = ParseProperties(objEl);

            if (FirstChildElement(objEl, "ellipse") != nullptr)
            {
                obj->Ellipse = std::make_unique<Document::TiledEllipseXml>();
            }
            if (FirstChildElement(objEl, "point") != nullptr)
            {
                obj->Point = std::make_unique<Document::TiledPointXml>();
            }
            if (XmlElement* polyEl = FirstChildElement(objEl, "polygon"))
            {
                auto polygon = std::make_unique<Document::TiledPolygonXml>();
                polygon->Points = AttrString(polyEl, "points");
                obj->Polygon = std::move(polygon);
            }
            if (XmlElement* lineEl = FirstChildElement(objEl, "polyline"))
            {
                auto polyline = std::make_unique<Document::TiledPolylineXml>();
                polyline->Points = AttrString(lineEl, "points");
                obj->Polyline = std::move(polyline);
            }
            if (XmlElement* textEl = FirstChildElement(objEl, "text"))
            {
                auto text = std::make_unique<Document::TiledTextXml>();
                text->FontFamily = AttrString(textEl, "fontfamily");
                text->PixelSize = AttrInt(textEl, "pixelsize", 16);
                text->Wrap = AttrInt(textEl, "wrap");
                text->Color = AttrString(textEl, "color", "#000000");
                text->Bold = AttrInt(textEl, "bold");
                text->Italic = AttrInt(textEl, "italic");
                text->Underline = AttrInt(textEl, "underline");
                text->Strikeout = AttrInt(textEl, "strikeout");
                text->Kerning = AttrInt(textEl, "kerning", 1);
                text->HAlign = AttrString(textEl, "halign", "left");
                text->VAlign = AttrString(textEl, "valign", "top");
                text->Value = textEl->getInnerTextProperty();
                obj->Text = std::move(text);
            }

            return obj;
        }

        std::unique_ptr<Document::TiledObjectGroupXml> ParseObjectGroup(const XmlElement* parent)
        {
            XmlElement* groupEl = FirstChildElement(parent, "objectgroup");
            if (groupEl == nullptr)
            {
                return nullptr;
            }

            auto group = std::make_unique<Document::TiledObjectGroupXml>();
            group->DrawOrder = AttrString(groupEl, "draworder");
            for (XmlElement* objEl : ChildElements(groupEl, "object"))
            {
                group->Objects.push_back(ParseObject(objEl));
            }
            return group;
        }

        std::unique_ptr<Document::TiledAnimationXml> ParseAnimation(const XmlElement* parent)
        {
            XmlElement* animEl = FirstChildElement(parent, "animation");
            if (animEl == nullptr)
            {
                return nullptr;
            }

            auto animation = std::make_unique<Document::TiledAnimationXml>();
            for (XmlElement* frameEl : ChildElements(animEl, "frame"))
            {
                Document::TiledAnimationFrameXml frame;
                frame.TileId = AttrInt(frameEl, "tileid");
                frame.Duration = AttrInt(frameEl, "duration");
                animation->Frames.push_back(frame);
            }
            return animation;
        }

        std::unique_ptr<Document::TiledTileXml> ParseTile(const XmlElement* tileEl)
        {
            auto tile = std::make_unique<Document::TiledTileXml>();
            tile->Id = AttrInt(tileEl, "id");
            tile->Type = AttrString(tileEl, "type");
            tile->Class = AttrString(tileEl, "class");
            tile->Probability = AttrFloat(tileEl, "probability");
            tile->Properties = ParseProperties(tileEl);
            tile->Image = ParseImage(tileEl);
            tile->ObjectGroup = ParseObjectGroup(tileEl);
            tile->Animation = ParseAnimation(tileEl);
            return tile;
        }

        void ParseTilesetBody(const XmlElement* tsEl, Document::TiledTilesetXml& tileset)
        {
            tileset.Name = AttrString(tsEl, "name");
            tileset.TileWidth = AttrInt(tsEl, "tilewidth");
            tileset.TileHeight = AttrInt(tsEl, "tileheight");
            tileset.TileCount = AttrInt(tsEl, "tilecount");
            tileset.Columns = AttrInt(tsEl, "columns");
            tileset.Spacing = AttrInt(tsEl, "spacing");
            tileset.Margin = AttrInt(tsEl, "margin");
            tileset.ObjectAlignment = AttrString(tsEl, "objectalignment");

            if (XmlElement* offsetEl = FirstChildElement(tsEl, "tileoffset"))
            {
                auto offset = std::make_unique<Document::TiledTileOffsetXml>();
                offset->X = AttrInt(offsetEl, "x");
                offset->Y = AttrInt(offsetEl, "y");
                tileset.TileOffset = std::move(offset);
            }

            if (XmlElement* gridEl = FirstChildElement(tsEl, "grid"))
            {
                auto grid = std::make_unique<Document::TiledGridXml>();
                grid->Orientation = AttrString(gridEl, "orientation");
                grid->Width = AttrInt(gridEl, "width");
                grid->Height = AttrInt(gridEl, "height");
                tileset.Grid = std::move(grid);
            }

            tileset.Image = ParseImage(tsEl);
            tileset.Properties = ParseProperties(tsEl);

            for (XmlElement* tileEl : ChildElements(tsEl, "tile"))
            {
                tileset.Tiles.push_back(ParseTile(tileEl));
            }
        }

        void ParseLayerBase(const XmlElement* layerEl, Document::TiledLayerXml& layer)
        {
            layer.Id = AttrInt(layerEl, "id");
            layer.Name = AttrString(layerEl, "name");
            layer.Class = AttrString(layerEl, "class");
            layer.OffsetX = AttrFloat(layerEl, "offsetx");
            layer.OffsetY = AttrFloat(layerEl, "offsety");
            layer.ParallaxX = AttrFloat(layerEl, "parallaxx", 1.0f);
            layer.ParallaxY = AttrFloat(layerEl, "parallaxy", 1.0f);
            layer.Opacity = AttrFloat(layerEl, "opacity", 1.0f);
            layer.Visible = AttrInt(layerEl, "visible", 1);
            layer.TintColor = AttrString(layerEl, "tintcolor");
            layer.Properties = ParseProperties(layerEl);
        }

        std::unique_ptr<Document::TiledLayerXml> ParseLayerElement(const XmlElement* layerEl);

        std::vector<std::unique_ptr<Document::TiledLayerXml>> ParseLayerList(const XmlElement* parent)
        {
            std::vector<std::unique_ptr<Document::TiledLayerXml>> result;
            for (XmlElement* child : AllChildElements(parent))
            {
                const std::string tag = child->getNameProperty();
                if (tag == "layer" || tag == "objectgroup" || tag == "imagelayer" || tag == "group")
                {
                    if (std::unique_ptr<Document::TiledLayerXml> layer = ParseLayerElement(child))
                    {
                        result.push_back(std::move(layer));
                    }
                }
            }
            return result;
        }

        std::unique_ptr<Document::TiledLayerXml> ParseLayerElement(const XmlElement* layerEl)
        {
            const std::string tag = layerEl->getNameProperty();

            if (tag == "layer")
            {
                auto tileLayer = std::make_unique<Document::TiledTileLayerXml>();
                ParseLayerBase(layerEl, *tileLayer);
                tileLayer->Width = AttrInt(layerEl, "width");
                tileLayer->Height = AttrInt(layerEl, "height");

                if (XmlElement* dataEl = FirstChildElement(layerEl, "data"))
                {
                    auto data = std::make_unique<Document::TiledTileLayerDataXml>();
                    data->Encoding = AttrString(dataEl, "encoding");
                    data->Compression = AttrString(dataEl, "compression");
                    data->Value = dataEl->getInnerTextProperty();

                    for (XmlElement* tileEl : ChildElements(dataEl, "tile"))
                    {
                        Document::TiledDataTileXml dataTile;
                        dataTile.Gid = AttrUInt32(tileEl, "gid");
                        data->Tiles.push_back(dataTile);
                    }

                    for (XmlElement* chunkEl : ChildElements(dataEl, "chunk"))
                    {
                        Document::TiledChunkXml chunk;
                        chunk.X = AttrInt(chunkEl, "x");
                        chunk.Y = AttrInt(chunkEl, "y");
                        chunk.Width = AttrInt(chunkEl, "width");
                        chunk.Height = AttrInt(chunkEl, "height");
                        chunk.Value = chunkEl->getInnerTextProperty();
                        data->Chunks.push_back(std::move(chunk));
                    }

                    tileLayer->Data = std::move(data);
                }

                return tileLayer;
            }

            if (tag == "objectgroup")
            {
                auto objectLayer = std::make_unique<Document::TiledObjectLayerXml>();
                ParseLayerBase(layerEl, *objectLayer);
                objectLayer->Color = AttrString(layerEl, "color");
                objectLayer->DrawOrder = AttrString(layerEl, "draworder");
                for (XmlElement* objEl : ChildElements(layerEl, "object"))
                {
                    objectLayer->Objects.push_back(ParseObject(objEl));
                }
                return objectLayer;
            }

            if (tag == "imagelayer")
            {
                auto imageLayer = std::make_unique<Document::TiledImageLayerXml>();
                ParseLayerBase(layerEl, *imageLayer);
                imageLayer->RepeatX = AttrInt(layerEl, "repeatx");
                imageLayer->RepeatY = AttrInt(layerEl, "repeaty");
                imageLayer->Image = ParseImage(layerEl);
                return imageLayer;
            }

            if (tag == "group")
            {
                auto groupLayer = std::make_unique<Document::TiledGroupLayerXml>();
                ParseLayerBase(layerEl, *groupLayer);
                groupLayer->Layers = ParseLayerList(layerEl);
                return groupLayer;
            }

            return nullptr;
        }

        std::unique_ptr<Document::TiledMapXml> ParseMapDocument(const std::string& xmlText)
        {
            try
            {
                XmlDocument doc;
                doc.LoadXml(xmlText);
                XmlElement* mapEl = doc.getDocumentElementProperty();
                if (mapEl == nullptr || mapEl->getNameProperty() != "map")
                {
                    throw TilemapParseException("TMX document has no root <map> element.");
                }

                auto map = std::make_unique<Document::TiledMapXml>();
                map->Version = AttrString(mapEl, "version");
                map->TiledVersion = AttrString(mapEl, "tiledversion");
                map->Orientation = AttrString(mapEl, "orientation");
                map->RenderOrder = AttrString(mapEl, "renderorder");
                map->Width = AttrInt(mapEl, "width");
                map->Height = AttrInt(mapEl, "height");
                map->TileWidth = AttrInt(mapEl, "tilewidth");
                map->TileHeight = AttrInt(mapEl, "tileheight");
                map->Infinite = AttrInt(mapEl, "infinite");
                map->BackgroundColor = AttrString(mapEl, "backgroundcolor");
                map->ParallaxOriginX = AttrFloat(mapEl, "parallaxoriginx");
                map->ParallaxOriginY = AttrFloat(mapEl, "parallaxoriginy");
                map->HexSideLength = AttrInt(mapEl, "hexsidelength");
                map->StaggerAxis = AttrString(mapEl, "staggeraxis");
                map->StaggerIndex = AttrString(mapEl, "staggerindex");
                map->NextLayerId = AttrInt(mapEl, "nextlayerid");
                map->NextObjectId = AttrInt(mapEl, "nextobjectid");
                map->Properties = ParseProperties(mapEl);

                for (XmlElement* tsEl : ChildElements(mapEl, "tileset"))
                {
                    auto tilesetRef = std::make_unique<Document::TiledTilesetRefXml>();
                    tilesetRef->FirstGlobalId = AttrInt(tsEl, "firstgid");
                    tilesetRef->Source = AttrString(tsEl, "source");
                    if (tilesetRef->Source.empty())
                    {
                        ParseTilesetBody(tsEl, *tilesetRef);
                    }
                    map->Tilesets.push_back(std::move(tilesetRef));
                }

                map->Layers = ParseLayerList(mapEl);

                return map;
            }
            catch (const TilemapParseException&)
            {
                throw;
            }
            catch (const std::exception&)
            {
                throw TilemapParseException("Failed to deserialize TMX XML");
            }
        }

        std::unique_ptr<Document::TiledTilesetXml> ParseTilesetDocument(const std::string& xmlText)
        {
            XmlDocument doc;
            doc.LoadXml(xmlText);
            XmlElement* tsEl = doc.getDocumentElementProperty();
            if (tsEl == nullptr || tsEl->getNameProperty() != "tileset")
            {
                throw TilemapParseException("TSX document has no root <tileset> element.");
            }

            auto tileset = std::make_unique<Document::TiledTilesetXml>();
            ParseTilesetBody(tsEl, *tileset);
            return tileset;
        }
    }

    std::unique_ptr<Document::TiledMapXml> ParseMapXml(const std::string& xmlText)
    {
        return ParseMapDocument(xmlText);
    }

    TiledTmxParser::TiledTmxParser(
        const std::optional<std::string>& baseDirectory, const std::optional<Content::ExternalResourceResolver>& resourceResolver)
        : baseDirectory_(baseDirectory), resourceResolver_(resourceResolver.value_or(Content::ExternalResourceResolver(&Content::OpenFile)))
    {
    }

    namespace
    {
        void LoadExternalTilesets(
            Document::TiledMapXml& mapXml, const std::string& baseDirectory, const Content::ExternalResourceResolver& resourceResolver)
        {
            for (const std::unique_ptr<Document::TiledTilesetRefXml>& tilesetRef : mapXml.Tilesets)
            {
                if (tilesetRef->Source.empty())
                {
                    continue;
                }

                const std::string tsxPath = System::IO::Path::Combine(baseDirectory, tilesetRef->Source);

                std::unique_ptr<System::IO::Stream> stream;
                try
                {
                    stream = resourceResolver(tsxPath);
                }
                catch (const std::exception&)
                {
                    throw TilemapParseException("External tileset '" + tilesetRef->Source + "' (firstgid=" +
                        std::to_string(tilesetRef->FirstGlobalId) + ") could not be opened. Expected at: " + tsxPath);
                }

                std::unique_ptr<Document::TiledTilesetXml> tilesetXml;
                try
                {
                    tilesetXml = ParseTilesetDocument(ReadAllText(*stream));
                }
                catch (const std::exception&)
                {
                    throw TilemapParseException("Failed to parse TSX file: " + tsxPath);
                }

                tilesetXml->FirstGlobalId = tilesetRef->FirstGlobalId;
                tilesetRef->TilesetData = std::move(tilesetXml);
            }
        }
    }

    Tilemap TiledTmxParser::ParseFromFile(const std::string& path, GraphicsDevice& graphicsDevice)
    {
        if (path.empty())
        {
            throw std::invalid_argument("path must not be empty.");
        }

        const std::string fullPath = baseDirectory_.has_value() ? System::IO::Path::Combine(*baseDirectory_, path) : path;

        if (!System::IO::File::Exists(fullPath))
        {
            throw System::IO::FileNotFoundException("Tilemap file not found: " + fullPath, fullPath);
        }

        try
        {
            const std::string baseDirectory = System::IO::Path::GetDirectoryName(System::IO::Path::GetFullPath(fullPath));

            std::unique_ptr<Document::TiledMapXml> mapXml = ParseMapDocument(System::IO::File::ReadAllText(fullPath));
            LoadExternalTilesets(*mapXml, baseDirectory, resourceResolver_);

            TilemapData data = Converters::Convert(*mapXml);
            return Build(data, graphicsDevice, baseDirectory, resourceResolver_);
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception&)
        {
            throw TilemapParseException("Failed to parse TMX file: " + fullPath);
        }
    }

    Tilemap TiledTmxParser::ParseFromStream(System::IO::Stream& stream, GraphicsDevice& graphicsDevice, const std::string& basePath)
    {
        try
        {
            const std::string baseDirectory =
                !basePath.empty() ? basePath : (baseDirectory_.has_value() ? *baseDirectory_ : std::filesystem::current_path().string());

            std::unique_ptr<Document::TiledMapXml> mapXml = ParseMapDocument(ReadAllText(stream));
            LoadExternalTilesets(*mapXml, baseDirectory, resourceResolver_);

            TilemapData data = Converters::Convert(*mapXml);
            return Build(data, graphicsDevice, baseDirectory, resourceResolver_);
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception&)
        {
            throw TilemapParseException("Failed to parse TMX from stream");
        }
    }
}
