// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/World3DScreenEXT.hpp"

#include "CNA/Extended/ECS/World.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::GameTime;

    World3DScreenEXT::World3DScreenEXT() = default;
    World3DScreenEXT::~World3DScreenEXT() = default;

    void World3DScreenEXT::Initialize()
    {
        ECS::WorldBuilder builder;
        ConfigureWorldEXT(builder);
        worldEXT_ = builder.Build();
        worldEXT_->Initialize();
    }

    void World3DScreenEXT::Update(GameTime& gameTime)
    {
        worldEXT_->Update(gameTime);
    }

    void World3DScreenEXT::Draw(const GameTime& gameTime)
    {
        worldEXT_->Draw(gameTime);
    }
}
