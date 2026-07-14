// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/DebugDrawComponentEXT.hpp"

#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"

#include <array>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;
    using Microsoft::Xna::Framework::BoundingFrustum;
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Vector3;

    namespace
    {
        // Shared by AddDebugBoxLinesEXT/AddDebugFrustumLinesEXT: BoundingBox::GetCorners()
        // and BoundingFrustum::GetCorners() both return 8 corners in the same real CNA
        // ordering (near/front face 0-3, far/back face 4-7) -- see this file's header
        // comment.
        void AppendCornersWireframeEXT(std::vector<DebugLineEXT>& lines, const std::vector<Vector3>& corners, const Color& color)
        {
            static constexpr std::array<std::array<int, 2>, 12> kEdges = {{
                {0, 1}, {1, 2}, {2, 3}, {3, 0}, // near/front face
                {4, 5}, {5, 6}, {6, 7}, {7, 4}, // far/back face
                {0, 4}, {1, 5}, {2, 6}, {3, 7}, // connecting edges
            }};

            for (const std::array<int, 2>& edge : kEdges)
            {
                lines.push_back(DebugLineEXT{corners[static_cast<std::size_t>(edge[0])], corners[static_cast<std::size_t>(edge[1])], color});
            }
        }
    }

    void AddDebugBoxLinesEXT(std::vector<DebugLineEXT>& lines, const BoundingBox& box, const Color& color)
    {
        AppendCornersWireframeEXT(lines, box.GetCorners(), color);
    }

    void AddDebugFrustumLinesEXT(std::vector<DebugLineEXT>& lines, const BoundingFrustum& frustum, const Color& color)
    {
        AppendCornersWireframeEXT(lines, frustum.GetCorners(), color);
    }
}
