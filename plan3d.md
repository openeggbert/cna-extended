# `cna-extended` — World3DEXT Porting/Design Plan

Status: **APPROVED (2026-07-14 by Robert Vokáč), including the Phase 5-8 scope expansion
(`Collisions3DEXT`/`Graphics3DEXT`/`Particles3DEXT`/`Tilemaps3DEXT`) recorded the same
day.** Implementation may proceed phase by phase, each verified (build + tests, both
CMake configs) before the next starts, matching this project's established discipline.

See [`3d.md`](3d.md) for the full analysis and rationale behind every decision recorded
here — this file is the checkbox-tracked task list derived from it, not a restatement of
the reasoning.

## 1. What this is

`World3DEXT` is a new, **non-upstream** addition to `cna-extended`: a small 3D scene
layer (camera, transform hierarchy, model rendering with frustum culling and skinned
animation) built on top of this project's already-ported ECS (`CNA::Extended::ECS`) and
`Transform3`, and on top of `cna`'s existing `Model`/`BasicEffect`/`SkinnedEffect`/
`BoundingFrustum` support. It exists because `cna-extended`'s upstream, MonoGame.Extended,
is 2D-only and has no equivalent — this work has no C# source to port faithfully against,
unlike every other module in this project. Design material for it is
`cna_scene_/` (a specification study, see `3d.md` §2-3) and `cna-scene`'s own existing
source (read as reference material only, not a dependency — see `3d.md` §5).

## 2. Recorded scope decisions (2026-07-14)

See `3d.md` §5 for full rationale. Summary:

- **No new build dependency.** Code adapted from `cna-scene` is re-implemented as this
  project's own `World3DEXT` code, not linked in via CMake.
- **Namespace**: `CNA::Extended::World3DEXT`.
- **Tracking**: this document, not a reopened `plan.md`.
- **Scope**: larger than the specification's own minimal core — frustum culling, a
  multi-effect (`BasicEffect`/`SkinnedEffect`/custom `ShaderEffect`) rendering pipeline,
  and skinned/skeletal animation are all in scope for the initial implementation, not
  deferred as later follow-ups.
- **Scope, second round (2026-07-14, same day)**: the owner approved this plan and asked
  for it to be implemented, then asked whether 2D-only `cna-extended` modules would get
  systematic 3D counterparts. Confirmed: **yes, for four specific modules**, added as new
  phases below: `Collisions3DEXT` (3D counterpart of `Collisions2D`),
  `Graphics3DEXT` (3D counterpart of the `Graphics` module — billboards, cube meshes,
  debug draw, 3D text/labels; absorbs and renames the original Phase 5 "cube/billboard/
  debug-draw helpers"), `Particles3DEXT` (3D counterpart of `Particles`), and
  `Tilemaps3DEXT` (3D/voxel counterpart of `Tilemaps`). Other 2D modules (`NinePatch`,
  `Tweening`, `ViewportAdapters`, `Timers`, `BitmapFonts`, `Input`) were not selected and
  are out of scope unless the owner asks for them later — do not add 3D variants of
  modules not listed here without asking first, matching the same explicit-approval
  discipline that governed every scope decision in this document so far.

## 3. Conventions (in addition to the root `CLAUDE.md`, which still applies in full)

- **Naming**: every new class/struct/enum/free-function name ends in `EXT`
  (`Camera3DEXT`, `Transform3ComponentEXT`). Every new method added to an
  *already-ported* class also ends in `EXT`. Members that override a real base-class
  virtual keep the base method's real name (see `3d.md` §7 for the full rule and why).
- **Namespace/file layout**: `CNA::Extended::World3DEXT`, files under
  `include/CNA/Extended/World3DEXT/<Type>.hpp` / `src/CNA/Extended/World3DEXT/<Type>.cpp`,
  matching this project's existing per-module convention (`Tilemaps/`, `Particles/`,
  `ECS/`).
- **No upstream C# to port against.** Every file gets an SPDX header
  (`// SPDX-License-Identifier: MIT`), but the usual "ported from MonoGame.Extended's
  X.cs" header-comment convention does not apply here — instead, note what real
  `cna`/`cna-extended` infrastructure a given `World3DEXT` type builds on (e.g. "builds on
  `CNA::Extended::Transform3`, `Microsoft::Xna::Framework::Graphics::BasicEffect`"), and,
  for anything adapted from `cna-scene`, a note identifying which `cna-scene` file/class it
  was adapted from (matching how this project already cites SlimMath/Triangulator/
  artemis_CSharp provenance for code adapted from other MIT-licensed sources — see
  `NOTICE.md`).
- **Tests**: GoogleTest, added alongside each phase, same `tests/CNA/Extended/World3DEXT/`
  layout. Every phase must land with real test coverage before being marked complete — no
  "add tests later," matching the root `CLAUDE.md` rule.
- **Zero-warning policy** (`-Wall -Wextra -Werror`) and the two-config build/test
  discipline (`-DCNA_EXTENDED_LINK_CNA=ON` and `=OFF`) apply exactly as they do to the
  rest of this project.
- **Do NOT modify `../cna`, `../sharp-runtime`, `../cna-scene`, or any other sibling
  repository.** `cna-scene` may be *read* for reference (per the recorded decision in §2),
  never edited, never linked.

## 4. Phases & tasks

Ordered by dependency, matching `3d.md` §8. Check off tasks as they land; update `NEXT.md`
alongside every session's progress (or a dedicated `NEXT3D.md`, if this work is large
enough to warrant one by the time Phase 1 starts — decide then, not now).

### Phase 1 — `Camera3DEXT`

- [ ] `CNA::Extended::World3DEXT::Camera3DEXT` (`Camera<Vector3>` + `IMovable` +
      `IRotatable`): position/target-or-forward/up, FOV/aspect/near/far,
      `GetViewMatrixEXT()`/`GetProjectionMatrixEXT()`/`GetBoundingFrustumEXT()`. Adapt
      `cna-scene::Camera3D`'s math as reference (see `3d.md` §6.1).
- [ ] Tests mirroring `OrthographicCameraTests.cpp`'s structure/coverage bar.

### Phase 2 — Transform hierarchy bridge

- [ ] `Transform3ComponentEXT` (embeds a `Transform3` + `ParentEntityIdEXT`).
- [ ] `TransformHierarchySystemEXT` (resolves `ParentEntityIdEXT` → real
      `Transform3::setParentProperty()` wiring each frame/on change).
- [ ] Real `ctest`-verified test proving multi-level parent/child world-matrix
      propagation through a live `ECS::World`.

### Phase 3 — Model rendering, frustum culling, multi-effect pipeline

- [ ] `ModelComponentEXT` (non-owning `Model*` + `Effect*` + `BoundingSphere`).
- [ ] `RenderSystem3DEXT` (`ECS::Systems::EntityDrawSystem`): per-frame frustum test via
      `Camera3DEXT::GetBoundingFrustumEXT()` + `BoundingFrustum::Contains`/`Intersects`
      against each entity's transformed `BoundingSphere`; `IEffectMatrices`-based
      `World`/`View`/`Projection` set (works for any `Effect` subclass); draw via
      `VertexBuffer`/`IndexBuffer` + `Effect::Apply()` + `DrawIndexedPrimitives`, matching
      `TilemapRenderer.cpp`'s established pattern.
- [ ] Real headless render test (matching `TilemapIntegrationTests.cpp`'s established
      `GraphicsDevice` + render-target + pixel-readback idiom): a 3D model actually drawn,
      confirmed by sampled pixel(s); a second test confirming an out-of-frustum entity is
      correctly skipped (e.g. via a draw-call counter or an untouched clear-color pixel).

### Phase 4 — Skinned animation

- [ ] `SkinnedModelComponentEXT` (`Model*` + `SkinnedEffect*` + bone transform array —
      resolve the ownership question noted in `3d.md` §9.1 before landing this).
- [ ] `AnimationSystem3DEXT` (advances bone transforms per frame; decide during this
      phase whether to reuse `Animations::AnimationController`'s timing logic or need a
      new one — `3d.md` §9.3).
- [ ] `RenderSystem3DEXT` extended to forward `SkinnedModelComponentEXT`'s bone array to
      `SkinnedEffect` alongside the existing `ModelComponentEXT` path.
- [ ] Real headless render test proving a skinned model's pose changes between two
      different animation times produce different pixel output.

### Phase 5 — `Collisions3DEXT` (3D counterpart of `Collisions2D`)

Mirrors `CNA::Extended::Collisions2D`'s shape (`CollisionWorld2D`, a broadphase
(`QuadTree`/`SpatialHash`), narrow-phase `ICollisionActor`) in 3D, reusing `cna`'s real
`BoundingBox`/`BoundingSphere`/`BoundingFrustum` (already used for frustum culling in
Phase 3) instead of `cna-extended`'s existing 2D shape types.

- [ ] `ICollisionActor3DEXT` (3D counterpart of `Collisions2D::ICollisionActor`):
      exposes a `BoundingBox`/`BoundingSphere` bounds, an `OnCollisionEXT` callback.
- [ ] A broadphase: `OctreeEXT` (3D counterpart of `QuadTree`) — start with the simplest
      correct version (matching `QuadTree`'s own real structure/API shape where it
      translates cleanly to 3D), optimize later only if a real need appears.
- [ ] `CollisionWorld3DEXT` (3D counterpart of `CollisionWorld2D`): registers actors,
      runs broadphase + narrow-phase collision detection per `Update`, raises collision
      events.
- [ ] Tests mirroring `Collisions2D`'s existing test coverage/structure
      (`CollisionWorld2DTests.cpp`, `QuadTreeTests.cpp`) adapted to 3D scenarios.

### Phase 6 — `Graphics3DEXT` (3D counterpart of `Graphics`)

Absorbs and renames the original Phase 5 draft ("cube/billboard/debug-draw helpers") —
this is now the single phase covering every `Graphics3DEXT` rendering helper, mirroring
`CNA::Extended::Graphics`'s `Sprite`/`AnimatedSprite`/`SpriteBatchExtensions` shape in 3D.

- [ ] `CubeMeshComponentEXT` / `CubeMeshRenderSystemEXT` (adapted from
      `cna-scene::CubeMesh`/`CubeMeshRenderer`, re-authored as this project's own code —
      resolve the `NOTICE.md` attribution question, `3d.md` §9.2, before landing).
- [ ] `BillboardComponentEXT` / `BillboardRenderSystemEXT` (adapted from
      `cna-scene::BillboardMesh`/`BillboardMeshRenderer`, always-face-camera behavior
      driven by `Camera3DEXT` — this is the 3D counterpart of `Graphics::Sprite`).
- [ ] `AnimatedBillboardComponentEXT` (3D counterpart of `Graphics::AnimatedSprite`,
      reusing `Animations::AnimationController`'s frame-timing logic against a billboard's
      texture region instead of a 2D sprite's).
- [ ] `Text3DEXT` / `TextBillboardRenderSystemEXT` (3D counterpart of `BitmapFonts`
      rendering — floating/world-space text labels, rendered as camera-facing billboards
      using the already-ported `BitmapFont` glyph layout logic against a billboard quad
      instead of a 2D `SpriteBatch.DrawString` call).
- [ ] `DebugDrawComponentEXT` / `DebugDrawSystemEXT` (adapted from `cna-scene::DebugDraw`
      — lines/boxes for development visualization, including drawing `Collisions3DEXT`
      bounds and `Camera3DEXT` frustums for debugging).
- [ ] Tests for each, matching the real-render-test bar established in Phase 3.

### Phase 7 — `Particles3DEXT` (3D counterpart of `Particles`)

Mirrors `CNA::Extended::Particles`'s `ParticleEffect`/`ParticleEmitter`/`ParticleBuffer`
shape, changing 2D position/velocity fields to `Vector3` and rendering through
`Graphics3DEXT`'s billboard pipeline (Phase 6) instead of `SpriteBatch`.

- [ ] `Particle3DEXT` (3D counterpart of `Particles::Data::Particle` — position/velocity/
      etc. as `Vector3` instead of the 2D packed-float-array layout).
- [ ] `ParticleEmitter3DEXT` / `ParticleEffect3DEXT` (3D counterparts of `ParticleEmitter`/
      `ParticleEffect`; reuse `Particles::Modifiers`/`Particles::Profiles`' existing
      shape/logic wherever it's already dimension-agnostic, extend only what's genuinely
      2D-specific).
- [ ] `ParticleRenderSystem3DEXT` (draws active 3D particles as billboards via
      `Graphics3DEXT::BillboardRenderSystemEXT`'s existing draw path, not a new one).
- [ ] Tests mirroring `Particles`'s existing coverage, adapted to 3D scenarios.

### Phase 8 — `Tilemaps3DEXT` (3D/voxel counterpart of `Tilemaps`)

Mirrors `CNA::Extended::Tilemaps`'s data-model/rendering split (`Tilemap`/
`TilemapTileLayer`/`TilemapTileset` + `Rendering::TilemapRenderer`) for a 3D/voxel grid,
building on `Graphics3DEXT::CubeMeshComponentEXT`/`CubeMeshRenderSystemEXT` (Phase 6) for
the actual per-tile rendering and `Collisions3DEXT` (Phase 5) for tile collision.

- [ ] `Tilemap3DEXT` / `TilemapTileLayer3DEXT` (3D counterpart of `Tilemap`/
      `TilemapTileLayer` — a 3D grid of tile IDs, likely `(X, Y, Z)` or `(X, Z)` + a
      height/layer value, decide the exact shape during this phase based on what a real
      voxel/block world actually needs).
- [ ] `Tilemap3DFactoryEXT` (3D counterpart of `TilemapFactory` — builds a
      `Tilemap3DEXT` from data; whether it reads a real format (e.g. a 3D-extended Tiled/
      Ogmo JSON) or only supports hand-built data is a decision for this phase, not
      assumed here).
- [ ] `TilemapRenderer3DEXT` (3D counterpart of `Tilemaps::Rendering::TilemapRenderer` —
      batches visible tiles into `CubeMeshComponentEXT` draw calls, using
      `Camera3DEXT`'s frustum for per-chunk/per-tile culling, matching Phase 3's
      established culling pattern).
- [ ] Tests mirroring `Tilemaps`'s existing coverage, adapted to a 3D grid.

### Phase 9 — `World3DScreenEXT`

- [ ] `World3DScreenEXT : Screens::Screen` owning an `ECS::World` + `Camera3DEXT`,
      `GetWorld3DEXT()`/`GetCamera3DEXT()` accessors — ties Phases 1-8 together into one
      convenience base class for real game screens, reusing `Screen`'s real
      `Load`/`Update`/`Draw`/`Unload` lifecycle and `ScreenManager`'s existing stack/
      transition support (no new Scene/SceneManager type — see `3d.md` §6.8).
- [ ] An end-to-end example under `examples/` (matching Phase 10's
      `examples/tiled_demo/` precedent) demonstrating a small 3D scene: a camera, a
      hierarchy of a few transformed entities, at least one culled-out-of-frame entity,
      a `Tilemaps3DEXT` voxel area, some `Particles3DEXT` effect, and (if Phase 4 has
      landed) a skinned model.

## 5. After meaningful changes

- Check off completed tasks above; add newly discovered tasks under the right phase.
- Update `NEXT.md` (or a dedicated `NEXT3D.md`) with what changed and what's next.
- Keep both build configs green, exactly as required for the rest of this project.
