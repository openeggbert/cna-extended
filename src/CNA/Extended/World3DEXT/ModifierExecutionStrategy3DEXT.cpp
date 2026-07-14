// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ModifierExecutionStrategy3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    namespace
    {
        class SerialModifierExecutionStrategy3DEXT final : public ModifierExecutionStrategy3DEXT
        {
        public:
            void ExecuteModifiers(const std::vector<Modifier3DEXT*>& modifiers, float elapsedSeconds, std::vector<Particle3DEXT>& particles) override
            {
                for (Modifier3DEXT* modifier : modifiers)
                {
                    modifier->InternalUpdate(elapsedSeconds, particles);
                }
            }
        };
    }

    ModifierExecutionStrategy3DEXT& ModifierExecutionStrategy3DEXT::getSerialProperty()
    {
        static SerialModifierExecutionStrategy3DEXT instance;
        return instance;
    }
}
