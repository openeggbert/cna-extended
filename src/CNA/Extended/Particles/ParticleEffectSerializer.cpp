// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// See ParticleEffectSerializer.hpp for the read-side DOM-vs-streaming-reader translation
// rationale. Two small path-string utilities (RemoveExtension/ShortenRelativePath) are ported
// locally from MonoGame.Extended's Content/ContentReaderExtensions.cs -- that file is otherwise
// entirely xnb-ContentReader glue and correctly excluded per plan.md, but these two specific
// methods are plain, generic path manipulation with no xnb dependency, so only they are ported
// here rather than the whole (still-excluded) file.
#include "CNA/Extended/Particles/ParticleEffectSerializer.hpp"

#include "CNA/Extended/HslColor.hpp"
#include "CNA/Extended/Particles/Modifiers/AgeModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/Containers/CircleContainerModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/Containers/RectangleContainerModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/Containers/RectangleLoopContainerModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/DragModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/ColorInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/HueInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/OpacityInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/RotationInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/ScaleInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/VelocityInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/LinearGravityModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/OpacityFastFadeModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/RotationModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/VelocityColorModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/VelocityModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/VortexModifier.hpp"
#include "CNA/Extended/Particles/Profiles/BoxFillProfile.hpp"
#include "CNA/Extended/Particles/Profiles/BoxProfile.hpp"
#include "CNA/Extended/Particles/Profiles/BoxUniformProfile.hpp"
#include "CNA/Extended/Particles/Profiles/CircleProfile.hpp"
#include "CNA/Extended/Particles/Profiles/LineProfile.hpp"
#include "CNA/Extended/Particles/Profiles/PointProfile.hpp"
#include "CNA/Extended/Particles/Profiles/RingProfile.hpp"
#include "CNA/Extended/Particles/Profiles/SprayProfile.hpp"
#include "CNA/Extended/Serialization/Xml/XmlNodeExtensions.hpp"
#include "CNA/Extended/Serialization/Xml/XmlWriterExtensions.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/Int32.hpp"
#include "System/IO/File.hpp"
#include "System/IO/Path.hpp"
#include "System/Single.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlElement.hpp"
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/XmlWriterSettings.hpp"

#include <functional>
#include <stdexcept>
#include <vector>

namespace CNA::Extended::Particles::ParticleEffectSerializer
{
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;
    using namespace CNA::Extended::Serialization::Xml;
    using namespace CNA::Extended::Particles::Data;
    using namespace CNA::Extended::Particles::Modifiers;
    using namespace CNA::Extended::Particles::Modifiers::Containers;
    using namespace CNA::Extended::Particles::Modifiers::Interpolators;
    using namespace CNA::Extended::Particles::Profiles;

    namespace
    {
        // --- ContentReaderExtensions' two non-xnb path utilities (see file header comment) -----

        std::string RemoveExtensionLocal(std::string path)
        {
            const std::size_t dot = path.find_last_of('.');
            const std::size_t slash = path.find_last_of("/\\");
            if (dot != std::string::npos && (slash == std::string::npos || dot > slash))
            {
                path.erase(dot);
            }
            return path;
        }

        std::string ShortenRelativePath(std::string relativePath)
        {
            std::size_t ellipseIndex = relativePath.find("/../");
            while (ellipseIndex != std::string::npos)
            {
                const std::size_t lastDirectoryIndex =
                    ellipseIndex == 0 ? 0 : relativePath.find_last_of('/', ellipseIndex - 1) + 1;
                relativePath.erase(lastDirectoryIndex, (ellipseIndex + 4) - lastDirectoryIndex);
                ellipseIndex = relativePath.find("/../");
            }
            return relativePath;
        }

        // Minimal Path.GetRelativePath(relativeTo, path) equivalent for the one narrow upstream
        // call site (Path.Combine(baseDirectory, name) unexpectedly produced a rooted path because
        // `name` was itself already absolute) -- sharp-runtime's System::IO::Path has no
        // GetRelativePath. Strips relativeTo's prefix (plus separator) if path starts with it;
        // returns path unchanged otherwise, matching a reasonable fallback for this edge case.
        std::string GetRelativePathLocal(const std::string& relativeTo, const std::string& path)
        {
            if (path.size() > relativeTo.size() && path.compare(0, relativeTo.size(), relativeTo) == 0)
            {
                std::size_t start = relativeTo.size();
                if (path[start] == '/' || path[start] == '\\')
                {
                    ++start;
                }
                return path.substr(start);
            }
            return path;
        }

        // --- DOM-based attribute helpers (XmlReaderExtensions equivalent for XmlNode) -----------
        // See ParticleEffectSerializer.hpp's header comment for why these exist. Semantics match
        // XmlReaderExtensions' `GetAttributeXxx(reader, name, defaultValue)` overloads (the
        // non-throwing ones, which every call site in this file uses): a comma-delimited
        // attribute value, split and parsed strictly (System::Single::Parse/System::Int32::Parse,
        // not std::stof/std::stoi -- see XmlReaderExtensions.cpp's own header comment for why that
        // matters), falling back to a caller-supplied default on any missing/malformed input
        // rather than throwing.

        std::vector<std::string> SplitNonEmptyLocal(const std::string& value, char delimiter)
        {
            std::vector<std::string> result;
            std::string token;
            for (const char c : value)
            {
                if (c == delimiter)
                {
                    if (!token.empty())
                    {
                        result.push_back(token);
                    }
                    token.clear();
                }
                else
                {
                    token.push_back(c);
                }
            }
            if (!token.empty())
            {
                result.push_back(token);
            }
            return result;
        }

        Vector2 GetAttributeVector2Local(const System::Xml::XmlNode* node, const std::string& attributeName, const Vector2& defaultValue)
        {
            const std::string value = GetStringAttribute(node, attributeName);
            const std::vector<std::string> split = SplitNonEmptyLocal(value, ',');
            if (split.size() != 2)
            {
                return defaultValue;
            }
            try
            {
                return Vector2(System::Single::Parse(split[0]), System::Single::Parse(split[1]));
            }
            catch (const std::exception&)
            {
                return defaultValue;
            }
        }

        Vector3 GetAttributeVector3Local(const System::Xml::XmlNode* node, const std::string& attributeName, const Vector3& defaultValue)
        {
            const std::string value = GetStringAttribute(node, attributeName);
            const std::vector<std::string> split = SplitNonEmptyLocal(value, ',');
            if (split.size() != 3)
            {
                return defaultValue;
            }
            try
            {
                return Vector3(System::Single::Parse(split[0]), System::Single::Parse(split[1]), System::Single::Parse(split[2]));
            }
            catch (const std::exception&)
            {
                return defaultValue;
            }
        }

        Rectangle GetAttributeRectangleLocal(const System::Xml::XmlNode* node, const std::string& attributeName, const Rectangle& defaultValue)
        {
            const std::string value = GetStringAttribute(node, attributeName);
            const std::vector<std::string> split = SplitNonEmptyLocal(value, ',');
            if (split.size() != 4)
            {
                return defaultValue;
            }
            try
            {
                return Rectangle(System::Int32::Parse(split[0]), System::Int32::Parse(split[1]), System::Int32::Parse(split[2]),
                    System::Int32::Parse(split[3]));
            }
            catch (const std::exception&)
            {
                return defaultValue;
            }
        }

        template <typename T>
        T GetAttributeEnumLocal(
            const System::Xml::XmlNode* node, const std::string& attributeName, T defaultValue,
            const std::function<bool(const std::string&, T&)>& parse)
        {
            const std::string value = GetStringAttribute(node, attributeName);
            if (value.empty())
            {
                return defaultValue;
            }
            T result{};
            return parse(value, result) ? result : defaultValue;
        }

        bool ParseParticleValueKind(const std::string& text, ParticleValueKind& out)
        {
            if (text == "Constant")
            {
                out = ParticleValueKind::Constant;
                return true;
            }
            if (text == "Random")
            {
                out = ParticleValueKind::Random;
                return true;
            }
            return false;
        }

        bool ParseParticleRenderingOrder(const std::string& text, ParticleRenderingOrder& out)
        {
            if (text == "FrontToBack")
            {
                out = ParticleRenderingOrder::FrontToBack;
                return true;
            }
            if (text == "BackToFront")
            {
                out = ParticleRenderingOrder::BackToFront;
                return true;
            }
            return false;
        }

        bool ParseCircleRadiation(const std::string& text, CircleRadiation& out)
        {
            if (text == "None")
            {
                out = CircleRadiation::None;
                return true;
            }
            if (text == "In")
            {
                out = CircleRadiation::In;
                return true;
            }
            if (text == "Out")
            {
                out = CircleRadiation::Out;
                return true;
            }
            return false;
        }

        bool ParseLineRadiation(const std::string& text, LineRadiation& out)
        {
            if (text == "None")
            {
                out = LineRadiation::None;
                return true;
            }
            if (text == "Directional")
            {
                out = LineRadiation::Directional;
                return true;
            }
            if (text == "PerpendicularUp")
            {
                out = LineRadiation::PerpendicularUp;
                return true;
            }
            if (text == "PerpendicularDown")
            {
                out = LineRadiation::PerpendicularDown;
                return true;
            }
            return false;
        }

        // --- Deserialize ------------------------------------------------------------------------

        std::unique_ptr<ParticleEmitter> ReadParticleEmitter(
            const System::Xml::XmlNode* node, ContentManager& content, const std::string& baseDirectory);
        std::shared_ptr<Graphics::Texture2DRegion> ReadTexture2DRegion(
            const System::Xml::XmlNode* node, ContentManager& content, const std::string& baseDirectory, ParticleEmitter& emitter);
        Data::ParticleReleaseParameters ReadParticleReleaseParameters(const System::Xml::XmlNode* node);
        Data::ParticleInt32Parameter ReadParticleInt32Parameter(const System::Xml::XmlNode* node);
        Data::ParticleFloatParameter ReadParticleFloatParameter(const System::Xml::XmlNode* node);
        Data::ParticleVector2Parameter ReadParticleVector2Parameter(const System::Xml::XmlNode* node);
        Data::ParticleColorParameter ReadParticleColorParameter(const System::Xml::XmlNode* node);
        std::unique_ptr<Profile> ReadProfile(const System::Xml::XmlNode* node);
        void ReadModifiers(const System::Xml::XmlNode* modifiersNode, std::vector<std::unique_ptr<Modifier>>& modifiers);
        std::unique_ptr<Modifier> ReadModifier(const System::Xml::XmlNode* node);
        void ReadInterpolators(
            const System::Xml::XmlNode* interpolatorsNode, std::vector<std::unique_ptr<Interpolator>>& interpolators);
        std::unique_ptr<Interpolator> ReadInterpolator(const System::Xml::XmlNode* node);

        std::unique_ptr<ParticleEffect> DeserializeDocument(
            System::Xml::XmlDocument& document, ContentManager& content, const std::string& baseDirectory)
        {
            System::Xml::XmlElement* root = document.getDocumentElementProperty();
            if (root == nullptr || root->getLocalNameProperty() != "ParticleEffect")
            {
                throw System::Xml::XmlException("Expected ParticleEffect root element");
            }

            const std::string name = GetStringAttribute(root, "Name");
            auto effect = std::make_unique<ParticleEffect>(name.empty() ? "ParticleEffect" : name);

            effect->setPositionProperty(GetAttributeVector2Local(root, "Position", Vector2()));
            effect->setRotationProperty(GetSingleAttribute(root, "Rotation"));
            effect->setScaleProperty(GetAttributeVector2Local(root, "Scale", Vector2()));
            effect->setAutoTriggerProperty(GetBoolAttribute(root, "AutoTrigger"));
            effect->setAutoTriggerFrequencyProperty(GetSingleAttribute(root, "AutoTriggerFrequency"));

            const std::unique_ptr<System::Xml::XmlNodeList> emitterNodes(root->SelectNodes("Emitters/ParticleEmitter"));
            for (SharpRuntime::intcs i = 0; i < emitterNodes->getCountProperty(); ++i)
            {
                std::unique_ptr<ParticleEmitter> emitter = ReadParticleEmitter((*emitterNodes)[i], content, baseDirectory);
                effect->getEmittersProperty().push_back(std::move(emitter));
            }

            return effect;
        }

        std::unique_ptr<ParticleEmitter> ReadParticleEmitter(
            const System::Xml::XmlNode* node, ContentManager& content, const std::string& baseDirectory)
        {
            const int capacity = GetInt32Attribute(node, "Capacity");

            auto emitter = std::make_unique<ParticleEmitter>(capacity);
            const std::string name = GetStringAttribute(node, "Name");
            emitter->setNameProperty(name.empty() ? "ParticleEmitter" : name);
            emitter->setLifeSpanProperty(GetSingleAttribute(node, "LifeSpan"));
            emitter->setOffsetProperty(GetAttributeVector2Local(node, "Offset", Vector2()));
            emitter->setLayerDepthProperty(GetSingleAttribute(node, "LayerDepth"));
            emitter->setReclaimFrequencyProperty(GetSingleAttribute(node, "ReclaimFrequency"));

            const std::string strategy = GetStringAttribute(node, "ModifierExecutionStrategy");
            if (strategy == "Parallel")
            {
                emitter->setModifierExecutionStrategyProperty(ModifierExecutionStrategy::getParallelProperty());
            }
            else
            {
                emitter->setModifierExecutionStrategyProperty(ModifierExecutionStrategy::getSerialProperty());
            }

            emitter->setRenderingOrderProperty(GetAttributeEnumLocal<ParticleRenderingOrder>(
                node, "RenderingOrder", ParticleRenderingOrder::FrontToBack, ParseParticleRenderingOrder));

            if (const System::Xml::XmlNode* textureRegionNode = node->SelectSingleNode("TextureRegion"))
            {
                emitter->setTextureRegionProperty(ReadTexture2DRegion(textureRegionNode, content, baseDirectory, *emitter));
            }

            if (const System::Xml::XmlNode* parametersNode = node->SelectSingleNode("Parameters"))
            {
                emitter->setParametersProperty(ReadParticleReleaseParameters(parametersNode));
            }

            if (const System::Xml::XmlNode* profileNode = node->SelectSingleNode("Profile"))
            {
                emitter->setProfileProperty(ReadProfile(profileNode));
            }

            if (const System::Xml::XmlNode* modifiersNode = node->SelectSingleNode("Modifiers"))
            {
                ReadModifiers(modifiersNode, emitter->getModifiersProperty());
            }

            return emitter;
        }

        std::shared_ptr<Graphics::Texture2DRegion> ReadTexture2DRegion(
            const System::Xml::XmlNode* node, ContentManager& content, const std::string& baseDirectory, ParticleEmitter& emitter)
        {
            const std::string name = GetStringAttribute(node, "Name");
            if (name.empty())
            {
                return nullptr;
            }

            std::string path = System::IO::Path::Combine(baseDirectory, name);

            // ContentManager throws if given a rooted path; if Combine produced one (because `name`
            // was itself already absolute), make it relative to baseDirectory again.
            if (System::IO::Path::IsPathRooted(path))
            {
                path = GetRelativePathLocal(baseDirectory, path);
            }

            // Path needs to be normalized before loading (upstream reference: issue #1162).
            for (char& c : path)
            {
                if (c == '\\')
                {
                    c = '/';
                }
            }
            path = RemoveExtensionLocal(path);
            path = ShortenRelativePath(path);

            auto texture = std::make_unique<Texture2D>(content.Load<Texture2D>(path));
            if (texture->getNameProperty().empty())
            {
                texture->setNameProperty(name);
            }

            Rectangle bounds = GetAttributeRectangleLocal(node, "Bounds", Rectangle());
            if (bounds.getIsEmptyProperty())
            {
                bounds = texture->getBoundsProperty();
            }

            auto region = std::make_shared<Graphics::Texture2DRegion>(texture.get(), bounds);
            emitter.AdoptOwnedTexture(std::move(texture));
            return region;
        }

        Data::ParticleReleaseParameters ReadParticleReleaseParameters(const System::Xml::XmlNode* node)
        {
            Data::ParticleReleaseParameters parameters;

            if (const System::Xml::XmlNode* n = node->SelectSingleNode("Quantity"))
            {
                parameters.Quantity = ReadParticleInt32Parameter(n);
            }
            if (const System::Xml::XmlNode* n = node->SelectSingleNode("Speed"))
            {
                parameters.Speed = ReadParticleFloatParameter(n);
            }
            if (const System::Xml::XmlNode* n = node->SelectSingleNode("Color"))
            {
                parameters.Color = ReadParticleColorParameter(n);
            }
            if (const System::Xml::XmlNode* n = node->SelectSingleNode("Opacity"))
            {
                parameters.Opacity = ReadParticleFloatParameter(n);
            }
            if (const System::Xml::XmlNode* n = node->SelectSingleNode("Scale"))
            {
                parameters.Scale = ReadParticleVector2Parameter(n);
            }
            if (const System::Xml::XmlNode* n = node->SelectSingleNode("Rotation"))
            {
                parameters.Rotation = ReadParticleFloatParameter(n);
            }
            if (const System::Xml::XmlNode* n = node->SelectSingleNode("Mass"))
            {
                parameters.Mass = ReadParticleFloatParameter(n);
            }

            return parameters;
        }

        Data::ParticleInt32Parameter ReadParticleInt32Parameter(const System::Xml::XmlNode* node)
        {
            const ParticleValueKind kind =
                GetAttributeEnumLocal<ParticleValueKind>(node, "Kind", ParticleValueKind::Constant, ParseParticleValueKind);

            if (kind == ParticleValueKind::Constant)
            {
                return Data::ParticleInt32Parameter(GetInt32Attribute(node, "Constant"));
            }
            if (kind == ParticleValueKind::Random)
            {
                return Data::ParticleInt32Parameter(GetInt32Attribute(node, "RandomMin"), GetInt32Attribute(node, "RandomMax"));
            }
            return Data::ParticleInt32Parameter(0);
        }

        Data::ParticleFloatParameter ReadParticleFloatParameter(const System::Xml::XmlNode* node)
        {
            const ParticleValueKind kind =
                GetAttributeEnumLocal<ParticleValueKind>(node, "Kind", ParticleValueKind::Constant, ParseParticleValueKind);

            if (kind == ParticleValueKind::Constant)
            {
                return Data::ParticleFloatParameter(GetSingleAttribute(node, "Constant"));
            }
            if (kind == ParticleValueKind::Random)
            {
                return Data::ParticleFloatParameter(GetSingleAttribute(node, "RandomMin"), GetSingleAttribute(node, "RandomMax"));
            }
            return Data::ParticleFloatParameter(0.0f);
        }

        Data::ParticleVector2Parameter ReadParticleVector2Parameter(const System::Xml::XmlNode* node)
        {
            const ParticleValueKind kind =
                GetAttributeEnumLocal<ParticleValueKind>(node, "Kind", ParticleValueKind::Constant, ParseParticleValueKind);

            if (kind == ParticleValueKind::Constant)
            {
                return Data::ParticleVector2Parameter(GetAttributeVector2Local(node, "Constant", Vector2()));
            }
            if (kind == ParticleValueKind::Random)
            {
                const Vector2 min = GetAttributeVector2Local(node, "RandomMin", Vector2());
                const Vector2 max = GetAttributeVector2Local(node, "RandomMax", Vector2());
                const bool uniform = GetBoolAttribute(node, "Uniform");
                Data::ParticleVector2Parameter parameter(min, max);
                parameter.Uniform = uniform;
                return parameter;
            }
            return Data::ParticleVector2Parameter(Vector2::Zero);
        }

        Data::ParticleColorParameter ReadParticleColorParameter(const System::Xml::XmlNode* node)
        {
            const ParticleValueKind kind =
                GetAttributeEnumLocal<ParticleValueKind>(node, "Kind", ParticleValueKind::Constant, ParseParticleValueKind);

            if (kind == ParticleValueKind::Constant)
            {
                return Data::ParticleColorParameter(GetAttributeVector3Local(node, "Constant", Vector3::Zero));
            }
            if (kind == ParticleValueKind::Random)
            {
                const Vector3 min = GetAttributeVector3Local(node, "RandomMin", Vector3::Zero);
                const Vector3 max = GetAttributeVector3Local(node, "RandomMax", Vector3::Zero);
                return Data::ParticleColorParameter(min, max);
            }
            return Data::ParticleColorParameter(Vector3::Zero);
        }

        std::unique_ptr<Profile> ReadProfile(const System::Xml::XmlNode* node)
        {
            const std::string type = GetStringAttribute(node, "Type");

            if (type == "BoxProfile")
            {
                auto profile = std::make_unique<BoxProfile>();
                profile->Width = GetSingleAttribute(node, "Width");
                profile->Height = GetSingleAttribute(node, "Height");
                return profile;
            }
            if (type == "BoxFillProfile")
            {
                auto profile = std::make_unique<BoxFillProfile>();
                profile->Width = GetSingleAttribute(node, "Width");
                profile->Height = GetSingleAttribute(node, "Height");
                return profile;
            }
            if (type == "BoxUniformProfile")
            {
                auto profile = std::make_unique<BoxUniformProfile>();
                profile->Width = GetSingleAttribute(node, "Width");
                profile->Height = GetSingleAttribute(node, "Height");
                return profile;
            }
            if (type == "CircleProfile")
            {
                auto profile = std::make_unique<CircleProfile>();
                profile->Radius = GetSingleAttribute(node, "Radius");
                profile->Radiate = GetAttributeEnumLocal<CircleRadiation>(node, "Radiate", CircleRadiation::None, ParseCircleRadiation);
                return profile;
            }
            if (type == "LineProfile")
            {
                auto profile = std::make_unique<LineProfile>();
                profile->Axis = GetAttributeVector2Local(node, "Axis", Vector2());
                profile->Length = GetSingleAttribute(node, "Length");
                profile->Direction = GetAttributeVector2Local(node, "Direction", Vector2());
                profile->Radiate = GetAttributeEnumLocal<LineRadiation>(node, "Radiate", LineRadiation::None, ParseLineRadiation);
                return profile;
            }
            if (type == "RingProfile")
            {
                auto profile = std::make_unique<RingProfile>();
                profile->Radius = GetSingleAttribute(node, "Radius");
                profile->Radiate = GetAttributeEnumLocal<CircleRadiation>(node, "Radiate", CircleRadiation::None, ParseCircleRadiation);
                return profile;
            }
            if (type == "SprayProfile")
            {
                auto profile = std::make_unique<SprayProfile>();
                profile->Direction = GetAttributeVector2Local(node, "Direction", Vector2());
                profile->Spread = GetSingleAttribute(node, "Spread");
                return profile;
            }

            // PointProfile, and the "unrecognized type" fallback -- matches upstream's switch
            // expression, whose default case is also ReadPointProfile(reader).
            return std::make_unique<PointProfile>();
        }

        void ReadModifiers(const System::Xml::XmlNode* modifiersNode, std::vector<std::unique_ptr<Modifier>>& modifiers)
        {
            const std::unique_ptr<System::Xml::XmlNodeList> nodes(modifiersNode->SelectNodes("Modifier"));
            for (SharpRuntime::intcs i = 0; i < nodes->getCountProperty(); ++i)
            {
                std::unique_ptr<Modifier> modifier = ReadModifier((*nodes)[i]);
                if (modifier)
                {
                    modifiers.push_back(std::move(modifier));
                }
            }
        }

        std::unique_ptr<Modifier> ReadModifier(const System::Xml::XmlNode* node)
        {
            const std::string type = GetStringAttribute(node, "Type");
            const std::string name = GetStringAttribute(node, "Name");
            const float frequency = GetSingleAttribute(node, "Frequency");
            const bool enabled = GetBoolAttribute(node, "Enabled");

            std::unique_ptr<Modifier> modifier;

            if (type == "AgeModifier")
            {
                auto ageModifier = std::make_unique<AgeModifier>();
                if (const System::Xml::XmlNode* interpolatorsNode = node->SelectSingleNode("Interpolators"))
                {
                    ReadInterpolators(interpolatorsNode, ageModifier->getInterpolatorsProperty());
                }
                modifier = std::move(ageModifier);
            }
            else if (type == "DragModifier")
            {
                auto dragModifier = std::make_unique<DragModifier>();
                dragModifier->DragCoefficient = GetSingleAttribute(node, "DragCoefficient");
                dragModifier->Density = GetSingleAttribute(node, "Density");
                modifier = std::move(dragModifier);
            }
            else if (type == "LinearGravityModifier")
            {
                auto gravityModifier = std::make_unique<LinearGravityModifier>();
                gravityModifier->Direction = GetAttributeVector2Local(node, "Direction", Vector2());
                gravityModifier->Strength = GetSingleAttribute(node, "Strength");
                modifier = std::move(gravityModifier);
            }
            else if (type == "OpacityFastFadeModifier")
            {
                modifier = std::make_unique<OpacityFastFadeModifier>();
            }
            else if (type == "RotationModifier")
            {
                auto rotationModifier = std::make_unique<RotationModifier>();
                rotationModifier->RotationRate = GetSingleAttribute(node, "RotationRate");
                modifier = std::move(rotationModifier);
            }
            else if (type == "VelocityColorModifier")
            {
                const Vector3 stationary = GetAttributeVector3Local(node, "StationaryColor", Vector3::Zero);
                const Vector3 velocity = GetAttributeVector3Local(node, "VelocityColor", Vector3::Zero);
                auto velocityColorModifier = std::make_unique<VelocityColorModifier>();
                velocityColorModifier->StationaryColor = HslColor(stationary.X, stationary.Y, stationary.Z);
                velocityColorModifier->VelocityColor = HslColor(velocity.X, velocity.Y, velocity.Z);
                velocityColorModifier->VelocityThreshold = GetSingleAttribute(node, "VelocityThreshold");
                modifier = std::move(velocityColorModifier);
            }
            else if (type == "VelocityModifier")
            {
                auto velocityModifier = std::make_unique<VelocityModifier>();
                velocityModifier->VelocityThreshold = GetSingleAttribute(node, "VelocityThreshold");
                if (const System::Xml::XmlNode* interpolatorsNode = node->SelectSingleNode("Interpolators"))
                {
                    ReadInterpolators(interpolatorsNode, velocityModifier->getInterpolatorsProperty());
                }
                modifier = std::move(velocityModifier);
            }
            else if (type == "VortexModifier")
            {
                auto vortexModifier = std::make_unique<VortexModifier>();
                vortexModifier->Position = GetAttributeVector2Local(node, "Position", Vector2());
                vortexModifier->Strength = GetSingleAttribute(node, "Strength");
                vortexModifier->OuterRadius = GetSingleAttribute(node, "OuterRadius");
                vortexModifier->InnerRadius = GetSingleAttribute(node, "InnerRadius");
                vortexModifier->MaxVelocity = GetSingleAttribute(node, "MaxVelocity");
                vortexModifier->setRotationAngleProperty(GetSingleAttribute(node, "RotationAngle"));
                modifier = std::move(vortexModifier);
            }
            else if (type == "CircleContainerModifier")
            {
                auto circleContainerModifier = std::make_unique<CircleContainerModifier>();
                circleContainerModifier->Radius = GetSingleAttribute(node, "Radius");
                circleContainerModifier->Inside = GetBoolAttribute(node, "Inside");
                circleContainerModifier->RestitutionCoefficient = GetSingleAttribute(node, "RestitutionCoefficient");
                modifier = std::move(circleContainerModifier);
            }
            else if (type == "RectangleContainerModifier")
            {
                auto rectangleContainerModifier = std::make_unique<RectangleContainerModifier>();
                rectangleContainerModifier->Width = GetInt32Attribute(node, "Width");
                rectangleContainerModifier->Height = GetInt32Attribute(node, "Height");
                rectangleContainerModifier->RestitutionCoefficient = GetSingleAttribute(node, "RestitutionCoefficient");
                modifier = std::move(rectangleContainerModifier);
            }
            else if (type == "RectangleLoopContainerModifier")
            {
                auto rectangleLoopContainerModifier = std::make_unique<RectangleLoopContainerModifier>();
                rectangleLoopContainerModifier->Width = GetInt32Attribute(node, "Width");
                rectangleLoopContainerModifier->Height = GetInt32Attribute(node, "Height");
                modifier = std::move(rectangleLoopContainerModifier);
            }

            if (modifier)
            {
                modifier->setNameProperty(name);
                // Matches upstream exactly: an XML <Modifier> element with no (or a zero/negative)
                // Frequency attribute throws here, since Modifier::setFrequencyProperty rejects
                // values <= 0 -- a genuine upstream strictness (every valid serialized Modifier
                // element must specify Frequency), not something this port relaxes.
                modifier->setFrequencyProperty(frequency);
                modifier->setEnabledProperty(enabled);
            }

            return modifier;
        }

        void ReadInterpolators(
            const System::Xml::XmlNode* interpolatorsNode, std::vector<std::unique_ptr<Interpolator>>& interpolators)
        {
            const std::unique_ptr<System::Xml::XmlNodeList> nodes(interpolatorsNode->SelectNodes("Interpolator"));
            for (SharpRuntime::intcs i = 0; i < nodes->getCountProperty(); ++i)
            {
                std::unique_ptr<Interpolator> interpolator = ReadInterpolator((*nodes)[i]);
                if (interpolator)
                {
                    interpolators.push_back(std::move(interpolator));
                }
            }
        }

        std::unique_ptr<Interpolator> ReadInterpolator(const System::Xml::XmlNode* node)
        {
            const std::string type = GetStringAttribute(node, "Type");
            const std::string name = GetStringAttribute(node, "Name");
            const bool enabled = GetBoolAttribute(node, "Enabled");

            std::unique_ptr<Interpolator> interpolator;

            if (type == "ColorInterpolator")
            {
                const Vector3 start = GetAttributeVector3Local(node, "StartValue", Vector3::Zero);
                const Vector3 end = GetAttributeVector3Local(node, "EndValue", Vector3::Zero);
                auto colorInterpolator = std::make_unique<ColorInterpolator>();
                colorInterpolator->StartValue = HslColor(start.X, start.Y, start.Z);
                colorInterpolator->EndValue = HslColor(end.X, end.Y, end.Z);
                interpolator = std::move(colorInterpolator);
            }
            else if (type == "HueInterpolator")
            {
                auto hueInterpolator = std::make_unique<HueInterpolator>();
                hueInterpolator->StartValue = GetSingleAttribute(node, "StartValue");
                hueInterpolator->EndValue = GetSingleAttribute(node, "EndValue");
                interpolator = std::move(hueInterpolator);
            }
            else if (type == "OpacityInterpolator")
            {
                auto opacityInterpolator = std::make_unique<OpacityInterpolator>();
                opacityInterpolator->StartValue = GetSingleAttribute(node, "StartValue");
                opacityInterpolator->EndValue = GetSingleAttribute(node, "EndValue");
                interpolator = std::move(opacityInterpolator);
            }
            else if (type == "RotationInterpolator")
            {
                auto rotationInterpolator = std::make_unique<RotationInterpolator>();
                rotationInterpolator->StartValue = GetSingleAttribute(node, "StartValue");
                rotationInterpolator->EndValue = GetSingleAttribute(node, "EndValue");
                interpolator = std::move(rotationInterpolator);
            }
            else if (type == "ScaleInterpolator")
            {
                auto scaleInterpolator = std::make_unique<ScaleInterpolator>();
                scaleInterpolator->StartValue = GetAttributeVector2Local(node, "StartValue", Vector2());
                scaleInterpolator->EndValue = GetAttributeVector2Local(node, "EndValue", Vector2());
                interpolator = std::move(scaleInterpolator);
            }
            else if (type == "VelocityInterpolator")
            {
                auto velocityInterpolator = std::make_unique<VelocityInterpolator>();
                velocityInterpolator->StartValue = GetAttributeVector2Local(node, "StartValue", Vector2());
                velocityInterpolator->EndValue = GetAttributeVector2Local(node, "EndValue", Vector2());
                interpolator = std::move(velocityInterpolator);
            }

            // Upstream bug, preserved faithfully: `interpolator.Enabled = enabled;` sits *outside*
            // the `if (interpolator != null)` guard in ReadInterpolator, so an unrecognized `Type`
            // (interpolator left null) is a NullReferenceException crash in C#. Translated per this
            // project's established "implicit null-deref crash -> explicit, catchable exception"
            // convention (see AnimatedSprite's single-arg-constructor precedent) rather than
            // replicating undefined behavior.
            if (!interpolator)
            {
                throw std::logic_error("Unrecognized Interpolator Type '" + type + "'.");
            }

            interpolator->setNameProperty(name);
            interpolator->setEnabledProperty(enabled);

            return interpolator;
        }

        // --- Serialize ----------------------------------------------------------------------------

        void WriteParticleEmitter(System::Xml::XmlWriter& writer, ParticleEmitter& emitter);
        void WriteTexture2DRegion(System::Xml::XmlWriter& writer, const Graphics::Texture2DRegion& region);
        void WriteParticleReleaseParameters(System::Xml::XmlWriter& writer, const Data::ParticleReleaseParameters& parameters);
        void WriteParticleInt32Parameter(System::Xml::XmlWriter& writer, const std::string& name, const Data::ParticleInt32Parameter& parameter);
        void WriteParticleFloatParameter(System::Xml::XmlWriter& writer, const std::string& name, const Data::ParticleFloatParameter& parameter);
        void WriteParticleVector2Parameter(
            System::Xml::XmlWriter& writer, const std::string& name, const Data::ParticleVector2Parameter& parameter);
        void WriteParticleColorParameter(System::Xml::XmlWriter& writer, const std::string& name, const Data::ParticleColorParameter& parameter);
        void WriteProfile(System::Xml::XmlWriter& writer, Profile& profile);
        void WriteModifier(System::Xml::XmlWriter& writer, Modifier& modifier);
        void WriteInterpolator(System::Xml::XmlWriter& writer, Interpolator& interpolator);

        void WriteAttributeVector2Local(System::Xml::XmlWriter& writer, const std::string& name, const Vector2& value)
        {
            WriteAttributeVector2(writer, name, value);
        }

        void SerializeEffect(System::Xml::XmlWriter& writer, ParticleEffect& effect)
        {
            writer.WriteStartDocument();
            writer.WriteStartElement("ParticleEffect");

            writer.WriteAttributeString("Name", effect.getNameProperty());
            WriteAttributeVector2Local(writer, "Position", effect.getPositionProperty());
            WriteAttributeFloat(writer, "Rotation", effect.getRotationProperty());
            WriteAttributeVector2Local(writer, "Scale", effect.getScaleProperty());
            WriteAttributeBool(writer, "AutoTrigger", effect.getAutoTriggerProperty());
            WriteAttributeFloat(writer, "AutoTriggerFrequency", effect.getAutoTriggerFrequencyProperty());

            if (!effect.getEmittersProperty().empty())
            {
                writer.WriteStartElement("Emitters");
                for (const std::unique_ptr<ParticleEmitter>& emitter : effect.getEmittersProperty())
                {
                    writer.WriteStartElement("ParticleEmitter");
                    WriteParticleEmitter(writer, *emitter);
                    writer.WriteEndElement();
                }
                writer.WriteEndElement();
            }

            writer.WriteEndElement();
            writer.WriteEndDocument();
        }

        void WriteParticleEmitter(System::Xml::XmlWriter& writer, ParticleEmitter& emitter)
        {
            writer.WriteAttributeString("Name", emitter.getNameProperty());
            WriteAttributeFloat(writer, "LifeSpan", emitter.getLifeSpanProperty());
            WriteAttributeVector2Local(writer, "Offset", emitter.getOffsetProperty());
            WriteAttributeFloat(writer, "LayerDepth", emitter.getLayerDepthProperty());
            WriteAttributeFloat(writer, "ReclaimFrequency", emitter.getReclaimFrequencyProperty());
            WriteAttributeInt(writer, "Capacity", emitter.getCapacityProperty());
            writer.WriteAttributeString("ModifierExecutionStrategy", emitter.getModifierExecutionStrategyProperty().ToString());
            writer.WriteAttributeString(
                "RenderingOrder", emitter.getRenderingOrderProperty() == ParticleRenderingOrder::FrontToBack ? "FrontToBack" : "BackToFront");

            if (emitter.getTextureRegionProperty())
            {
                writer.WriteStartElement("TextureRegion");
                WriteTexture2DRegion(writer, *emitter.getTextureRegionProperty());
                writer.WriteEndElement();
            }

            writer.WriteStartElement("Parameters");
            WriteParticleReleaseParameters(writer, emitter.getParametersProperty());
            writer.WriteEndElement();

            writer.WriteStartElement("Profile");
            WriteProfile(writer, emitter.getProfileProperty());
            writer.WriteEndElement();

            if (!emitter.getModifiersProperty().empty())
            {
                writer.WriteStartElement("Modifiers");
                for (const std::unique_ptr<Modifier>& modifier : emitter.getModifiersProperty())
                {
                    writer.WriteStartElement("Modifier");
                    WriteModifier(writer, *modifier);
                    writer.WriteEndElement();
                }
                writer.WriteEndElement();
            }
        }

        void WriteTexture2DRegion(System::Xml::XmlWriter& writer, const Graphics::Texture2DRegion& region)
        {
            writer.WriteAttributeString("Name", region.getTextureProperty()->getNameProperty());
            WriteAttributeRectangle(writer, "Bounds", region.getBoundsProperty());
        }

        void WriteParticleReleaseParameters(System::Xml::XmlWriter& writer, const Data::ParticleReleaseParameters& parameters)
        {
            WriteParticleInt32Parameter(writer, "Quantity", parameters.Quantity);
            WriteParticleFloatParameter(writer, "Speed", parameters.Speed);
            WriteParticleColorParameter(writer, "Color", parameters.Color);
            WriteParticleFloatParameter(writer, "Opacity", parameters.Opacity);
            WriteParticleVector2Parameter(writer, "Scale", parameters.Scale);
            WriteParticleFloatParameter(writer, "Rotation", parameters.Rotation);
            WriteParticleFloatParameter(writer, "Mass", parameters.Mass);
        }

        std::string ToString(ParticleValueKind kind)
        {
            return kind == ParticleValueKind::Constant ? "Constant" : "Random";
        }

        void WriteParticleInt32Parameter(System::Xml::XmlWriter& writer, const std::string& name, const Data::ParticleInt32Parameter& parameter)
        {
            writer.WriteStartElement(name);
            writer.WriteAttributeString("Kind", ToString(parameter.Kind));
            if (parameter.Kind == ParticleValueKind::Constant)
            {
                WriteAttributeInt(writer, "Constant", parameter.Constant);
            }
            else
            {
                WriteAttributeInt(writer, "RandomMin", parameter.RandomMin);
                WriteAttributeInt(writer, "RandomMax", parameter.RandomMax);
            }
            writer.WriteEndElement();
        }

        void WriteParticleFloatParameter(System::Xml::XmlWriter& writer, const std::string& name, const Data::ParticleFloatParameter& parameter)
        {
            writer.WriteStartElement(name);
            writer.WriteAttributeString("Kind", ToString(parameter.Kind));
            if (parameter.Kind == ParticleValueKind::Constant)
            {
                WriteAttributeFloat(writer, "Constant", parameter.Constant);
            }
            else
            {
                WriteAttributeFloat(writer, "RandomMin", parameter.RandomMin);
                WriteAttributeFloat(writer, "RandomMax", parameter.RandomMax);
            }
            writer.WriteEndElement();
        }

        void WriteParticleVector2Parameter(
            System::Xml::XmlWriter& writer, const std::string& name, const Data::ParticleVector2Parameter& parameter)
        {
            writer.WriteStartElement(name);
            writer.WriteAttributeString("Kind", ToString(parameter.Kind));
            if (parameter.Kind == ParticleValueKind::Constant)
            {
                WriteAttributeVector2Local(writer, "Constant", parameter.Constant);
            }
            else
            {
                WriteAttributeVector2Local(writer, "RandomMin", parameter.RandomMin);
                WriteAttributeVector2Local(writer, "RandomMax", parameter.RandomMax);
                if (parameter.Uniform)
                {
                    writer.WriteAttributeString("Uniform", "true");
                }
            }
            writer.WriteEndElement();
        }

        void WriteParticleColorParameter(System::Xml::XmlWriter& writer, const std::string& name, const Data::ParticleColorParameter& parameter)
        {
            writer.WriteStartElement(name);
            writer.WriteAttributeString("Kind", ToString(parameter.Kind));
            if (parameter.Kind == ParticleValueKind::Constant)
            {
                WriteAttributeVector3(writer, "Constant", parameter.Constant);
            }
            else
            {
                WriteAttributeVector3(writer, "RandomMin", parameter.RandomMin);
                WriteAttributeVector3(writer, "RandomMax", parameter.RandomMax);
            }
            writer.WriteEndElement();
        }

        void WriteProfile(System::Xml::XmlWriter& writer, Profile& profile)
        {
            if (auto* p = dynamic_cast<BoxFillProfile*>(&profile))
            {
                writer.WriteAttributeString("Type", "BoxFillProfile");
                WriteAttributeFloat(writer, "Width", p->Width);
                WriteAttributeFloat(writer, "Height", p->Height);
            }
            else if (auto* p2 = dynamic_cast<BoxUniformProfile*>(&profile))
            {
                writer.WriteAttributeString("Type", "BoxUniformProfile");
                WriteAttributeFloat(writer, "Width", p2->Width);
                WriteAttributeFloat(writer, "Height", p2->Height);
            }
            else if (auto* p3 = dynamic_cast<BoxProfile*>(&profile))
            {
                writer.WriteAttributeString("Type", "BoxProfile");
                WriteAttributeFloat(writer, "Width", p3->Width);
                WriteAttributeFloat(writer, "Height", p3->Height);
            }
            else if (auto* p4 = dynamic_cast<CircleProfile*>(&profile))
            {
                writer.WriteAttributeString("Type", "CircleProfile");
                WriteAttributeFloat(writer, "Radius", p4->Radius);
                writer.WriteAttributeString("Radiate", p4->Radiate == CircleRadiation::None ? "None" : p4->Radiate == CircleRadiation::In ? "In" : "Out");
            }
            else if (auto* p5 = dynamic_cast<LineProfile*>(&profile))
            {
                writer.WriteAttributeString("Type", "LineProfile");
                WriteAttributeVector2Local(writer, "Axis", p5->Axis);
                WriteAttributeFloat(writer, "Length", p5->Length);
                const char* radiate = "None";
                switch (p5->Radiate)
                {
                    case LineRadiation::Directional: radiate = "Directional"; break;
                    case LineRadiation::PerpendicularUp: radiate = "PerpendicularUp"; break;
                    case LineRadiation::PerpendicularDown: radiate = "PerpendicularDown"; break;
                    case LineRadiation::None: default: break;
                }
                writer.WriteAttributeString("Radiate", radiate);
                WriteAttributeVector2Local(writer, "Direction", p5->Direction);
            }
            else if (dynamic_cast<PointProfile*>(&profile))
            {
                writer.WriteAttributeString("Type", "PointProfile");
            }
            else if (auto* p6 = dynamic_cast<RingProfile*>(&profile))
            {
                writer.WriteAttributeString("Type", "RingProfile");
                WriteAttributeFloat(writer, "Radius", p6->Radius);
                writer.WriteAttributeString("Radiate", p6->Radiate == CircleRadiation::None ? "None" : p6->Radiate == CircleRadiation::In ? "In" : "Out");
            }
            else if (auto* p7 = dynamic_cast<SprayProfile*>(&profile))
            {
                writer.WriteAttributeString("Type", "SprayProfile");
                WriteAttributeVector2Local(writer, "Direction", p7->Direction);
                WriteAttributeFloat(writer, "Spread", p7->Spread);
            }
            else
            {
                writer.WriteAttributeString("Type", typeid(profile).name());
            }
        }

        void WriteModifier(System::Xml::XmlWriter& writer, Modifier& modifier)
        {
            writer.WriteAttributeString("Name", modifier.getNameProperty());
            WriteAttributeBool(writer, "Enabled", modifier.getEnabledProperty());
            WriteAttributeFloat(writer, "Frequency", modifier.getFrequencyProperty());

            if (auto* m = dynamic_cast<AgeModifier*>(&modifier))
            {
                writer.WriteAttributeString("Type", "AgeModifier");
                if (!m->getInterpolatorsProperty().empty())
                {
                    writer.WriteStartElement("Interpolators");
                    for (const std::unique_ptr<Interpolator>& interpolator : m->getInterpolatorsProperty())
                    {
                        writer.WriteStartElement("Interpolator");
                        WriteInterpolator(writer, *interpolator);
                        writer.WriteEndElement();
                    }
                    writer.WriteEndElement();
                }
            }
            else if (auto* m2 = dynamic_cast<CircleContainerModifier*>(&modifier))
            {
                writer.WriteAttributeString("Type", "CircleContainerModifier");
                WriteAttributeFloat(writer, "Radius", m2->Radius);
                WriteAttributeBool(writer, "Inside", m2->Inside);
                WriteAttributeFloat(writer, "RestitutionCoefficient", m2->RestitutionCoefficient);
            }
            else if (auto* m3 = dynamic_cast<DragModifier*>(&modifier))
            {
                writer.WriteAttributeString("Type", "DragModifier");
                WriteAttributeFloat(writer, "DragCoefficient", m3->DragCoefficient);
                WriteAttributeFloat(writer, "Density", m3->Density);
            }
            else if (auto* m4 = dynamic_cast<LinearGravityModifier*>(&modifier))
            {
                writer.WriteAttributeString("Type", "LinearGravityModifier");
                WriteAttributeVector2Local(writer, "Direction", m4->Direction);
                WriteAttributeFloat(writer, "Strength", m4->Strength);
            }
            else if (dynamic_cast<OpacityFastFadeModifier*>(&modifier))
            {
                writer.WriteAttributeString("Type", "OpacityFastFadeModifier");
            }
            else if (auto* m5 = dynamic_cast<RectangleContainerModifier*>(&modifier))
            {
                writer.WriteAttributeString("Type", "RectangleContainerModifier");
                WriteAttributeInt(writer, "Width", m5->Width);
                WriteAttributeInt(writer, "Height", m5->Height);
                WriteAttributeFloat(writer, "RestitutionCoefficient", m5->RestitutionCoefficient);
            }
            else if (auto* m6 = dynamic_cast<RectangleLoopContainerModifier*>(&modifier))
            {
                writer.WriteAttributeString("Type", "RectangleLoopContainerModifier");
                WriteAttributeInt(writer, "Width", m6->Width);
                WriteAttributeInt(writer, "Height", m6->Height);
            }
            else if (auto* m7 = dynamic_cast<RotationModifier*>(&modifier))
            {
                writer.WriteAttributeString("Type", "RotationModifier");
                WriteAttributeFloat(writer, "RotationRate", m7->RotationRate);
            }
            else if (auto* m8 = dynamic_cast<VelocityColorModifier*>(&modifier))
            {
                writer.WriteAttributeString("Type", "VelocityColorModifier");
                WriteAttributeVector3(
                    writer, "StationaryColor", Vector3(m8->StationaryColor.getHProperty(), m8->StationaryColor.getSProperty(), m8->StationaryColor.getLProperty()));
                WriteAttributeVector3(
                    writer, "VelocityColor", Vector3(m8->VelocityColor.getHProperty(), m8->VelocityColor.getSProperty(), m8->VelocityColor.getLProperty()));
                WriteAttributeFloat(writer, "VelocityThreshold", m8->VelocityThreshold);
            }
            else if (auto* m9 = dynamic_cast<VelocityModifier*>(&modifier))
            {
                writer.WriteAttributeString("Type", "VelocityModifier");
                WriteAttributeFloat(writer, "VelocityThreshold", m9->VelocityThreshold);
                if (!m9->getInterpolatorsProperty().empty())
                {
                    writer.WriteStartElement("Interpolators");
                    for (const std::unique_ptr<Interpolator>& interpolator : m9->getInterpolatorsProperty())
                    {
                        writer.WriteStartElement("Interpolator");
                        WriteInterpolator(writer, *interpolator);
                        writer.WriteEndElement();
                    }
                    writer.WriteEndElement();
                }
            }
            else if (auto* m10 = dynamic_cast<VortexModifier*>(&modifier))
            {
                writer.WriteAttributeString("Type", "VortexModifier");
                WriteAttributeVector2Local(writer, "Position", m10->Position);
                WriteAttributeFloat(writer, "Strength", m10->Strength);
                WriteAttributeFloat(writer, "OuterRadius", m10->OuterRadius);
                WriteAttributeFloat(writer, "InnerRadius", m10->InnerRadius);
                WriteAttributeFloat(writer, "MaxVelocity", m10->MaxVelocity);
                WriteAttributeFloat(writer, "RotationAngle", m10->getRotationAngleProperty());
            }
            else
            {
                writer.WriteAttributeString("Type", typeid(modifier).name());
            }
        }

        void WriteInterpolator(System::Xml::XmlWriter& writer, Interpolator& interpolator)
        {
            writer.WriteAttributeString("Name", interpolator.getNameProperty());
            WriteAttributeBool(writer, "Enabled", interpolator.getEnabledProperty());

            if (auto* i = dynamic_cast<ColorInterpolator*>(&interpolator))
            {
                writer.WriteAttributeString("Type", "ColorInterpolator");
                WriteAttributeVector3(
                    writer, "StartValue", Vector3(i->StartValue.getHProperty(), i->StartValue.getSProperty(), i->StartValue.getLProperty()));
                WriteAttributeVector3(
                    writer, "EndValue", Vector3(i->EndValue.getHProperty(), i->EndValue.getSProperty(), i->EndValue.getLProperty()));
            }
            else if (auto* i2 = dynamic_cast<HueInterpolator*>(&interpolator))
            {
                writer.WriteAttributeString("Type", "HueInterpolator");
                WriteAttributeFloat(writer, "StartValue", i2->StartValue);
                WriteAttributeFloat(writer, "EndValue", i2->EndValue);
            }
            else if (auto* i3 = dynamic_cast<OpacityInterpolator*>(&interpolator))
            {
                writer.WriteAttributeString("Type", "OpacityInterpolator");
                WriteAttributeFloat(writer, "StartValue", i3->StartValue);
                WriteAttributeFloat(writer, "EndValue", i3->EndValue);
            }
            else if (auto* i4 = dynamic_cast<RotationInterpolator*>(&interpolator))
            {
                writer.WriteAttributeString("Type", "RotationInterpolator");
                WriteAttributeFloat(writer, "StartValue", i4->StartValue);
                WriteAttributeFloat(writer, "EndValue", i4->EndValue);
            }
            else if (auto* i5 = dynamic_cast<ScaleInterpolator*>(&interpolator))
            {
                writer.WriteAttributeString("Type", "ScaleInterpolator");
                WriteAttributeVector2Local(writer, "StartValue", i5->StartValue);
                WriteAttributeVector2Local(writer, "EndValue", i5->EndValue);
            }
            else if (auto* i6 = dynamic_cast<VelocityInterpolator*>(&interpolator))
            {
                writer.WriteAttributeString("Type", "VelocityInterpolator");
                WriteAttributeVector2Local(writer, "StartValue", i6->StartValue);
                WriteAttributeVector2Local(writer, "EndValue", i6->EndValue);
            }
            else
            {
                writer.WriteAttributeString("Type", typeid(interpolator).name());
            }
        }
    }

    std::unique_ptr<ParticleEffect> Deserialize(const std::string& fileName, ContentManager& content)
    {
        System::ArgumentException::ThrowIfNullOrEmpty(fileName, "fileName");

        const std::string fullPath = System::IO::Path::GetFullPath(fileName);
        const std::string baseDirectory = System::IO::Path::GetDirectoryName(fullPath);
        const std::string xml = System::IO::File::ReadAllText(fileName);

        System::Xml::XmlDocument document;
        document.LoadXml(xml);
        return DeserializeDocument(document, content, baseDirectory);
    }

    std::unique_ptr<ParticleEffect> Deserialize(System::IO::Stream& stream, ContentManager& content, const std::string& baseDirectory)
    {
        std::string effectiveBaseDirectory = baseDirectory;
        if (effectiveBaseDirectory.empty())
        {
            effectiveBaseDirectory = content.getRootDirectoryProperty();
        }

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
        const std::string xml(buffer.begin(), buffer.begin() + totalRead);

        System::Xml::XmlDocument document;
        document.LoadXml(xml);
        return DeserializeDocument(document, content, effectiveBaseDirectory);
    }

    void Serialize(const std::string& fileName, ParticleEffect& effect)
    {
        System::ArgumentException::ThrowIfNullOrEmpty(fileName, "fileName");

        System::Xml::XmlWriterSettings settings;
        settings.Indent = true;
        settings.IndentChars = "  ";
        settings.CloseOutput = true;
        settings.NewLineChars = "\n";

        const std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::Create(fileName, settings));
        SerializeEffect(*writer, effect);
        writer->Flush();
    }

    void Serialize(System::IO::Stream& stream, ParticleEffect& effect)
    {
        System::Xml::XmlWriterSettings settings;
        settings.Indent = true;
        settings.IndentChars = "  ";
        settings.CloseOutput = false;
        settings.NewLineChars = "\n";

        const std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        SerializeEffect(*writer, effect);
        writer->Flush();

        const std::string xml = writer->ToString();
        stream.Write(reinterpret_cast<const SharpRuntime::bytecs*>(xml.data()), 0, static_cast<SharpRuntime::intcs>(xml.size()));
    }
}

