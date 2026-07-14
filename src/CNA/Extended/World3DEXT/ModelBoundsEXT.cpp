// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ModelBoundsEXT.hpp"

#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshCollection.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingSphere;
    using Microsoft::Xna::Framework::Graphics::Model;
    using Microsoft::Xna::Framework::Graphics::ModelMeshCollection;

    BoundingSphere ComputeModelBoundsEXT(const Model& model)
    {
        const ModelMeshCollection& meshes = model.getMeshesProperty();
        const int count = meshes.getCountProperty();
        if (count <= 0)
        {
            return BoundingSphere();
        }

        BoundingSphere merged = meshes[0]->getBoundingSphereProperty();
        for (int i = 1; i < count; ++i)
        {
            merged = BoundingSphere::CreateMerged(merged, meshes[i]->getBoundingSphereProperty());
        }
        return merged;
    }
}
