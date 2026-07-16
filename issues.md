# issues.md — known gaps in `World3DEXT` found while evaluating it as an `easy-3d` replacement

This file records issues found on 2026-07-16 while analyzing (for `galaxy-eggbert`) whether
`easy-3d` could be replaced by `cna-extended`'s `World3DEXT` module. **No code in this
repository was changed as a result of this analysis** — these are open items for the
project owner to fix or reconsider whenever there's time, not an active task list. Not
urgent; nothing here blocks any current phase of `plan.md`/`plan3d.md`.

**Decision made as a result of this analysis (2026-07-16, project owner)**: `galaxy-eggbert`
keeps using `easy-3d` directly for its terrain/HUD rendering — **no change there**.
`cna-extended` stays a general-purpose sibling library for other, non-`galaxy-eggbert`
projects. The two libraries are not meant to merge; this file exists so the gaps below
aren't lost, not to push a merge.

## 1. `World3DEXT` never actually became a full replacement for `easy-3d`

The original intent behind building `World3DEXT` (per `3d.md`, 2026-07-14) was to have
`cna-extended` fully absorb what `easy-3d` provides, as `EXT`-suffixed code, so `easy-3d`
could eventually be retired. That did not fully happen. Comparing `easy-3d`'s actual header
surface (`include/Easy3D/*.hpp`) against `World3DEXT`'s (`include/CNA/Extended/World3DEXT/*.hpp`)
turned up two concrete, unported pieces — see §2 and §3 below.

This isn't necessarily a mistake to reverse — §3 in particular is arguably *correct* scope
discipline (see its own reasoning) — but it does mean anyone who assumed "`World3DEXT`
covers everything `easy-3d` does" (as the project owner did, going into this analysis) will
be surprised. Recording that gap explicitly here so it's not silently assumed away again in
a future session.

## 2. No CPU-side static batch-building API — performance gap for large static scenes

**What's missing**: `easy-3d`'s `CubeMesh.hpp`/`BillboardMesh.hpp` expose a CPU-side
"append many items into shared `std::vector<Vertex>`/`std::vector<uint32_t>` arrays, upload
once" API (`AppendCubeMesh`/`BuildCubeMesh`, and the billboard equivalent). A caller with,
say, a few thousand static world blocks builds one combined mesh and issues a small, fixed
number of draw calls total (one per material/category), regardless of block count.

`World3DEXT`'s equivalents (`CubeMeshComponentEXT`/`CubeMeshRenderSystemEXT`,
`BillboardComponentEXT`/`BillboardRenderSystemEXT`) went a different, ECS-idiomatic route
instead: **one shared unit-cube (or unit-quad) mesh, drawn once per entity via
`DrawCubeEXT()`/`DrawBillboardEXT()`**. This is a clean, simple design for a moderate
number of *dynamic* entities, but it means **one `DrawIndexedPrimitives` call per instance,
every frame**, with no batching. For a large static scene (e.g. a whole tile-based world
with thousands of blocks), that's thousands of draw calls per frame instead of a handful.

This gap is already self-acknowledged in this repo, just not raised as a general concern:
`plan3d.md`'s Phase 8 entry for `TilemapRenderer3DEXT` says outright — *"Frustum-culls each
tile individually via `Camera3DEXT`'s `BoundingFrustum`... per-chunk geometry batching
remains a documented future optimization, not assumed needed without profiling."* That's
the same gap, scoped to `Tilemaps3DEXT` specifically; this entry generalizes it to
`CubeMeshRenderSystemEXT`/`BillboardRenderSystemEXT` as a whole, since any caller building a
large static scene from many `DrawCubeEXT`/`DrawBillboardEXT` calls (not just
`TilemapRenderer3DEXT`) hits the same ceiling.

**Suggested direction, not a decision**: a CPU-side batch-building helper analogous to
`easy-3d`'s `AppendCubeMesh`/`BuildCubeMesh` — e.g. a way to combine many
`CubeMeshComponentEXT`-shaped instances (position/size/texture/tint) into one shared
`VertexBuffer`/`IndexBuffer` built once and re-used across frames, falling back to
per-instance `DrawCubeEXT` only for entities that actually move/change. This is generic
(useful to any game with a large static or semi-static 3D scene), not `galaxy-eggbert`- or
Speedy-Blupi-specific, so it would fit `World3DEXT`'s stated scope as a general-purpose
toolkit — unlike §3 below.

## 3. Game-specific tile-render-mode geometry was (correctly, but silently) left out

`easy-3d`'s `CubeMesh.hpp` also defines `DirectionalCubeItem`, `PlateItem`,
`TripleCrossItem`, and `PyramidTipItem` — cube/plate/cross/pyramid variants with
per-face-visible/per-face-UV control. Their own doc comments cite
`mobile-eggbert-reference/questionnaire-all-remaining-tiles.md` and
`galaxy-eggbert plan.md E3D-MIG-147` directly — these are Speedy Blupi / `galaxy-eggbert`
tile-rendering concepts (the "DirectionalCube", "InnerPillarBox", "InnerFlatPlate",
"TripleCrossBillboard" render modes), not general MonoGame.Extended-family primitives.

**None of these were ported into `World3DEXT`.** That's very likely the *right* call —
baking one specific game's bespoke tile-render shapes into a general-purpose sibling library
that other, unrelated future projects might depend on would be scope pollution in the other
direction (the same kind of thing `easy-3d/CLAUDE.md` itself warns against: *"Eggbert-specific
logic does not go in Easy3D... put it in that project later"* — worth noting `easy-3d`
arguably already drifted on its own rule by carrying these types itself). But whichever
session did the `World3DEXT` port apparently made this scoping call implicitly, without
flagging it anywhere as a deliberate exclusion — it just quietly doesn't exist. Recording it
explicitly here so it reads as a documented decision instead of an oversight.

**No action suggested here** — this is working as intended, just previously undocumented.
If a future *other* project genuinely needs directional-cube/plate/cross/pyramid tile
geometry, that would be a new, explicitly-scoped ask, not a retroactive "finish the
`easy-3d` port."

## 4. Where this was found

Investigated 2026-07-16 while evaluating, for `galaxy-eggbert`
(`/rv/data/development/github.com/openeggbert/galaxy-eggbert`), whether its `Easy3D::*`
usage (`Camera3D`, `CubeMesh`/`CubeBatch`/`CubeMeshRenderer`, `BillboardMesh`/`BillboardBatch`/
`BillboardMeshRenderer`, `TextureAtlas`, mainly in `src/GalaxyEggbertCNA/Game/GETerrainRenderer.cpp`
and `Game/GEHud.cpp`) could be swapped for `cna-extended`'s `World3DEXT` equivalents. Answer:
`Camera3D`→`Camera3DEXT` and `TextureAtlas`→`Texture2DAtlas` would be a genuinely narrow,
low-risk swap; `CubeMesh`/`BillboardMesh` would not, for the reasons in §2 and §3. Project
owner decided not to pursue the swap at all for now — `galaxy-eggbert` stays on `easy-3d`
unchanged. This file exists only to preserve the technical findings for whenever
`cna-extended` itself (independent of `galaxy-eggbert`) is revisited.
