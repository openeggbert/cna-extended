// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ModelComponentEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Builds on Microsoft::Xna::Framework::Graphics::Model (real, already-implemented CNA
// model support: a rigid per-mesh bone hierarchy, each ModelMeshPart carrying its own
// Effect*, VertexBuffer*, IndexBuffer*) and BoundingSphere (also real CNA math). A plain
// data component -- entirely non-owning, matching this ECS's established
// ComponentMapperOf<T> convention (see Transform3ComponentEXT.hpp) -- pairing a Model*
// with a local-space BoundingSphere used for per-frame frustum culling by
// RenderSystem3DEXT. No separate Effect* field is needed here: unlike a single
// hand-rolled draw call, Model::Draw(world, view, projection) already forwards to each
// mesh's own per-part Effect (see Model.cpp) -- ModelComponentEXT would only be
// duplicating state Model already owns.
#pragma once

#include "Microsoft/Xna/Framework/BoundingSphere.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class Model;
}

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief ECS component pairing a non-owning Model* with a local-space BoundingSphere,
     * so RenderSystem3DEXT can frustum-cull and draw it each frame.
     * @see RenderSystem3DEXT, which reads this component.
     */
    struct ModelComponentEXT
    {
        /** @brief The model to draw. Not owned by this component -- caller/asset system manages lifetime. */
        Microsoft::Xna::Framework::Graphics::Model* ModelEXT = nullptr;

        /** @brief The model's bounds in its own local (untransformed) space, used for frustum culling. */
        Microsoft::Xna::Framework::BoundingSphere BoundsEXT;
    };
}
