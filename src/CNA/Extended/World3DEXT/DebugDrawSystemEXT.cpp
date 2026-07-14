// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/DebugDrawSystemEXT.hpp"

#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/DebugDrawComponentEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"

#include <typeindex>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using ECS::AspectBuilder;
    using ECS::ComponentManager;
    using ECS::Entity;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;
    using Microsoft::Xna::Framework::Graphics::VertexBuffer;
    using Microsoft::Xna::Framework::Graphics::VertexPositionColor;

    DebugDrawSystemEXT::DebugDrawSystemEXT(GraphicsDevice& graphicsDevice, Camera3DEXT& camera)
        : EntityDrawSystem(AspectBuilder().All({std::type_index(typeid(DebugDrawComponentEXT))})), graphicsDevice_(&graphicsDevice),
          camera_(&camera), effect_(graphicsDevice)
    {
        effect_.VertexColorEnabled = true;
    }

    DebugDrawSystemEXT::~DebugDrawSystemEXT() = default;

    void DebugDrawSystemEXT::Initialize(ComponentManager& componentManager)
    {
        (void)componentManager; // no cached ComponentMapper needed: Entity::Get<T>() below is cheap
    }

    void DebugDrawSystemEXT::Draw(const GameTime& gameTime)
    {
        (void)gameTime;

        std::vector<VertexPositionColor> vertices;
        for (const int entityId : getActiveEntitiesProperty())
        {
            Entity* entity = GetEntity(entityId);
            if (entity == nullptr)
            {
                continue;
            }

            DebugDrawComponentEXT* debugComponent = entity->Get<DebugDrawComponentEXT>();
            if (debugComponent == nullptr)
            {
                continue;
            }

            for (const DebugLineEXT& line : debugComponent->LinesEXT)
            {
                vertices.emplace_back(line.Start, line.LineColor);
                vertices.emplace_back(line.End, line.LineColor);
            }
        }

        if (vertices.empty())
        {
            return;
        }

        if (static_cast<int>(vertices.size()) > lineVertexCapacity_)
        {
            lineVertexCapacity_ = static_cast<int>(vertices.size());
            lineVertexBuffer_ = std::make_unique<VertexBuffer>(*graphicsDevice_, lineVertexCapacity_);
        }

        lineVertexBuffer_->SetData(vertices.data(), 0, static_cast<int>(vertices.size()));

        effect_.World = Matrix::getIdentityProperty();
        effect_.View = camera_->GetViewMatrixEXT();
        effect_.Projection = camera_->GetProjectionMatrixEXT();
        effect_.Apply();

        graphicsDevice_->SetVertexBuffer(lineVertexBuffer_.get());
        graphicsDevice_->DrawPrimitives(PrimitiveType::LineList, 0, static_cast<int>(vertices.size() / 2));
    }
}
