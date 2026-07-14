// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::World3DScreenEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Ties Phases 1-8 together into one convenience base class for real 3D game screens:
// owns an ECS::World + Camera3DEXT, reusing Screens::Screen's real Initialize/LoadContent/
// Update/Draw lifecycle and ScreenManager's existing stack/transition support as-is -- no
// new Scene/SceneManager type (see 3d.md section 6.8 for why: Screen/ScreenManager already
// fill that role for this whole project, a second parallel concept would just be
// duplication).
//
// A derived screen class configures which systems its World owns by overriding
// ConfigureWorldEXT (called once, from Initialize(), before the World is actually built)
// -- e.g. adding RenderSystem3DEXT/CubeMeshRenderSystemEXT/BillboardRenderSystemEXT/
// ParticleUpdateSystem3DEXT with a GraphicsDevice& and this screen's own GetCamera3DEXT().
// A derived class overriding Initialize() itself must call World3DScreenEXT::Initialize()
// first so GetWorld3DEXT() is valid before any of its own setup runs.
#pragma once

#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/Screens/Screen.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"

#include <memory>

namespace CNA::Extended::ECS
{
    class World;
}

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Convenience Screen base class owning an ECS::World + Camera3DEXT, tying
     * together every World3DEXT system from Phases 1-8.
     * @see Screens::Screen, whose Load/Update/Draw lifecycle this class reuses unchanged.
     * @see ConfigureWorldEXT, the hook derived classes override to add their own systems.
     */
    class World3DScreenEXT : public Screens::Screen
    {
    public:
        World3DScreenEXT();
        ~World3DScreenEXT() override;

        /** @brief Builds GetWorld3DEXT() via ConfigureWorldEXT(). Derived overrides must call this base implementation first. */
        void Initialize() override;

        /** @brief Calls GetWorld3DEXT().Update(gameTime). */
        void Update(Microsoft::Xna::Framework::GameTime& gameTime) override;

        /** @brief Calls GetWorld3DEXT().Draw(gameTime). */
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

        /** @brief Gets the ECS::World this screen owns. Only valid after Initialize() has run. */
        [[nodiscard]] ECS::World& GetWorld3DEXT() { return *worldEXT_; }

        /** @brief Gets the camera this screen owns. */
        [[nodiscard]] Camera3DEXT& GetCamera3DEXT() { return cameraEXT_; }

    protected:
        /**
         * @brief Override to add systems (via @p builder.AddSystem) before the World is
         * built. Called exactly once, from Initialize(). The base implementation adds
         * nothing -- an empty World is a valid (if useless) result.
         */
        virtual void ConfigureWorldEXT(ECS::WorldBuilder& builder) { (void)builder; }

    private:
        Camera3DEXT cameraEXT_;
        std::unique_ptr<ECS::World> worldEXT_;
    };
}
