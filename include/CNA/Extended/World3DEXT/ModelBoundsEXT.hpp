// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ComputeModelBoundsEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Added to fix an independently re-verified audit finding (audit.md, A-08):
// ModelComponentEXT::BoundsEXT defaults to a zero-radius BoundingSphere and is easy for a
// caller to leave unset, which silently culls a real model every frame (a zero-radius
// sphere at the origin almost never intersects the view frustum once the model has moved).
// This gives callers a real alternative to hand-rolling local-space bounds: merge every
// ModelMesh::getBoundingSphereProperty() via BoundingSphere::CreateMerged, matching this
// project's "reuse existing CNA/sharp-runtime machinery, don't re-roll it" convention.
//
// IMPORTANT, discovered while implementing this fix (not caught by the audit itself):
// ModelMesh::boundingSphere_ has no public setter and is never written by any code path in
// `cna` today (verified by grepping `cna`'s entire src/include tree) -- Model's constructor
// doesn't compute it from part geometry, and there's no content-pipeline loader in scope
// (see root CLAUDE.md's exclusion list) that would populate it either. So
// ModelMesh::getBoundingSphereProperty() currently *always* returns a zero-radius sphere at
// the origin, for every Model constructible today, and this helper's merged result inherits
// that: it is correct and forward-compatible (the moment `cna` gains any way to populate
// real per-mesh bounds, this helper immediately starts returning real ones with zero
// cna-extended changes needed), but it is NOT yet a full fix for A-08's actual concern
// (silent culling from unset bounds) -- callers still cannot get a non-degenerate bound out
// of this helper today. A real fix would need to derive bounds from each ModelMeshPart's
// raw vertex position data instead, which `cna`'s VertexBuffer::GetData only exposes via
// fixed per-vertex-type overloads (VertexPositionColor, VertexPositionNormalTexture, etc.),
// not a generic/type-erased read -- doing that generically would mean adding to `cna`
// itself, which is out of scope without the user's explicit go-ahead (root CLAUDE.md: "Do
// NOT modify sibling repositories"). Left as an explicit, documented gap rather than
// silently declared "fixed".
#pragma once

#include "Microsoft/Xna/Framework/BoundingSphere.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class Model;
}

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Derives a local-space BoundingSphere for @p model by merging every one of its
     * meshes' own BoundingSphere (Model::getMeshesProperty()[i]->getBoundingSphereProperty()).
     * @param model The model to compute bounds for.
     * @return A zero-radius sphere at the origin if @p model has no meshes; otherwise the
     * merged bounds of every mesh, suitable for assigning directly to
     * ModelComponentEXT::BoundsEXT.
     */
    [[nodiscard]] Microsoft::Xna::Framework::BoundingSphere ComputeModelBoundsEXT(const Microsoft::Xna::Framework::Graphics::Model& model);
}
