// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/Text3DEXT.hpp"

#include "CNA/Extended/BitmapFonts/BitmapFont.hpp"
#include "CNA/Extended/BitmapFonts/BitmapFontCharacter.hpp"
#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "CNA/Extended/World3DEXT/BillboardMeshEXT.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionTexture.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using BitmapFonts::BitmapFont;
    using CNA::Extended::Graphics::Texture2DRegion;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::IndexBuffer;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::VertexBuffer;
    using Microsoft::Xna::Framework::Graphics::VertexPositionTexture;

    namespace
    {
        struct PageBucketEXT
        {
            std::vector<VertexPositionTexture> Vertices;
            std::vector<std::uint16_t> Indices;
        };
    }

    Text3DMeshEXT BuildText3DMeshEXT(GraphicsDevice& graphicsDevice, const BitmapFont& font, const std::string& text)
    {
        // Glyphs are grouped by page texture (preserving first-seen page order via
        // pageOrder) rather than the older single-page-only design -- see this file's
        // header comment.
        std::vector<Texture2D*> pageOrder;
        std::unordered_map<Texture2D*, PageBucketEXT> pageBuckets;

        for (const BitmapFont::BitmapFontGlyph& glyph : font.GetGlyphs(text))
        {
            if (!glyph.Character)
            {
                continue;
            }

            const std::shared_ptr<Texture2DRegion>& region = glyph.Character->getTextureRegionProperty();
            if (!region || region->getTextureProperty() == nullptr)
            {
                continue;
            }

            Texture2D* texture = region->getTextureProperty();

            const Rectangle bounds = region->getBoundsProperty();
            if (bounds.Width <= 0 || bounds.Height <= 0)
            {
                continue; // whitespace-only glyphs have no visible quad
            }

            auto [bucketIt, inserted] = pageBuckets.try_emplace(texture);
            if (inserted)
            {
                pageOrder.push_back(texture);
            }
            PageBucketEXT& bucket = bucketIt->second;

            // BitmapFont::GetGlyphs lays glyphs out in Y-down pixel space; this mesh is
            // Y-up local space, so Y is negated here.
            const float left = glyph.Position.X;
            const float top = -glyph.Position.Y;
            const float right = left + static_cast<float>(bounds.Width);
            const float bottom = top - static_cast<float>(bounds.Height);

            const RectangleF uvRect = ConvertPixelRectToUvRectEXT(bounds, texture->getWidthProperty(), texture->getHeightProperty());
            const float u0 = uvRect.X;
            const float v0 = uvRect.Y;
            const float u1 = uvRect.X + uvRect.Width;
            const float v1 = uvRect.Y + uvRect.Height;

            const auto base = static_cast<std::uint16_t>(bucket.Vertices.size());
            bucket.Vertices.emplace_back(Vector3(left, top, 0.0f), Vector2(u0, v0));
            bucket.Vertices.emplace_back(Vector3(right, top, 0.0f), Vector2(u1, v0));
            bucket.Vertices.emplace_back(Vector3(right, bottom, 0.0f), Vector2(u1, v1));
            bucket.Vertices.emplace_back(Vector3(left, bottom, 0.0f), Vector2(u0, v1));

            bucket.Indices.push_back(base + 0);
            bucket.Indices.push_back(base + 1);
            bucket.Indices.push_back(base + 2);
            bucket.Indices.push_back(base + 0);
            bucket.Indices.push_back(base + 2);
            bucket.Indices.push_back(base + 3);
        }

        Text3DMeshEXT mesh;
        mesh.Parts.reserve(pageOrder.size());

        for (Texture2D* texture : pageOrder)
        {
            const PageBucketEXT& bucket = pageBuckets.at(texture);

            Text3DMeshPartEXT part;
            part.Texture = texture;
            part.PrimitiveCount = static_cast<int>(bucket.Indices.size() / 3);

            part.VertexBuffer = std::make_unique<VertexBuffer>(graphicsDevice, static_cast<int>(bucket.Vertices.size()));
            part.VertexBuffer->SetData(bucket.Vertices.data(), static_cast<int>(bucket.Vertices.size()));

            part.IndexBuffer = std::make_unique<IndexBuffer>(graphicsDevice, static_cast<int>(bucket.Indices.size()));
            part.IndexBuffer->SetData(bucket.Indices.data(), static_cast<int>(bucket.Indices.size()));

            mesh.Parts.push_back(std::move(part));
        }

        return mesh;
    }
}
