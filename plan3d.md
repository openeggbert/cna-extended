# `cna-extended` — World3DEXT Porting/Design Plan

Status: **All 9 original phases complete (2026-07-14); Phase 10 (audit follow-ups, added
2026-07-14) done except A-05, explicitly deferred to a future session (see its own entry
below for why).** Approved 2026-07-14 by Robert Vokáč, including the Phase 5-8
scope expansion (`Collisions3DEXT`/`Graphics3DEXT`/`Particles3DEXT`/`Tilemaps3DEXT`).
Every phase was verified (build + tests, both CMake configs, `rm -rf` clean rebuilds)
before the next started, matching this project's established discipline. An independent
audit (`audit.md`) reviewed the completed Phase 1-9 result and found real findings, now
tracked as Phase 10 below.

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

### Phase 1 — `Camera3DEXT` — **COMPLETE (2026-07-14)**

- [x] `CNA::Extended::World3DEXT::Camera3DEXT` — **standalone class, not
      `Camera<Vector3>`/`IMovable`/`IRotatable`** (corrected during design review before
      implementation — those interfaces hardcode 2D types regardless of their template
      parameter/are flatly 2D; see `3d.md` §6.1 for the full finding). Position/target/up,
      FOV/aspect/near/far, `GetViewMatrixEXT()`/`GetProjectionMatrixEXT()`/
      `GetBoundingFrustumEXT()`. Math adapted from `cna-scene::Camera3D` as reference
      (`Matrix::CreateLookAt`/`Matrix::CreatePerspectiveFieldOfView`, both real CNA math
      this file only forwards to). `include/CNA/Extended/World3DEXT/Camera3DEXT.hpp` +
      `src/.../Camera3DEXT.cpp`.
- [x] Tests: 7 tests covering defaults, property round-trip, view/projection matrix
      correctness (checked directly against `Matrix::CreateLookAt`/
      `CreatePerspectiveFieldOfView`), and 3 `BoundingFrustum` geometry tests (contains a
      point at the target, excludes a point behind the camera, excludes a point beyond
      the far plane) — `tests/CNA/Extended/World3DEXT/Camera3DEXTTests.cpp`.
      Both build configs clean, full suite **2086/2086** (was 2079).

### Phase 2 — Transform hierarchy bridge — **COMPLETE (2026-07-14)**

- [x] `Transform3ComponentEXT` (embeds a `Transform3` + `ParentEntityIdEXT`).
      `include/CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp`.
- [x] `TransformHierarchySystemEXT` (resolves `ParentEntityIdEXT` → real
      `Transform3::setParentProperty()` wiring each `Update()`; a thin adapter, no new
      hierarchy logic — `Transform3`'s own dirty-flag world-matrix recompute does the
      rest unchanged). `include/CNA/Extended/World3DEXT/TransformHierarchySystemEXT.hpp` +
      `src/.../TransformHierarchySystemEXT.cpp`.
      Derives from `EntityUpdateSystem`; overriding only the `Initialize(ComponentManager&)`
      overload hides `EntityUpdateSystem::Initialize(World&)` under
      `-Werror=overloaded-virtual` — fixed with a `using
      ECS::Systems::EntityUpdateSystem::Initialize;` declaration.
- [x] Real `ctest`-verified test proving multi-level parent/child world-matrix
      propagation through a live `ECS::World`: a 3-level grandparent/parent/child chain
      (confirms world-position summation through the hierarchy), live parent-move
      propagation without a second `Update()` call (proving `Transform3`'s existing
      dirty-flag propagation is doing the work, not this system), and parent detachment
      falling back to local position. Confirmed by direct reading of `World.cpp`
      (`ComponentManager`/`EntityManager` are auto-registered in `World`'s own
      constructor, before any `WorldBuilder`-added system, so `EntityManager::Update()`
      — which fires `EntityAdded` and populates this system's active-entities set —
      always runs before this system's own `Update()` within the same
      `world->Update()` call) that a **single** `world->Update()` call per frame is
      sufficient, not two. `tests/CNA/Extended/World3DEXT/TransformHierarchySystemEXTTests.cpp`,
      3 tests. Both build configs clean (genuine `rm -rf` + fresh configure + build),
      full suite **2089/2089** (was 2086; 2 pre-existing skips unrelated to this phase).

### Phase 3 — Model rendering, frustum culling, multi-effect pipeline — **COMPLETE (2026-07-14)**

- [x] `ModelComponentEXT` (non-owning `Model*` + `BoundingSphere`) —
      `include/CNA/Extended/World3DEXT/ModelComponentEXT.hpp`. **Design correction from
      this task's original wording, made before implementation**: no separate `Effect*`
      field. Reading `Model.cpp`/`ModelMesh.cpp` showed `Model::Draw(world, view,
      projection)` already forwards `World`/`View`/`Projection` to every mesh part's own
      `Effect*` via `IEffectMatrices` and issues the real
      `SetVertexBuffer`/`EffectPass::Apply()`/`DrawIndexedPrimitives` calls internally
      (`ModelMesh::Draw()`) — a component-level `Effect*` would only duplicate state
      `Model` already owns per mesh part. `RenderSystem3DEXT` therefore calls
      `Model::Draw()` directly rather than hand-rolling the draw call this task
      originally described.
- [x] `RenderSystem3DEXT` (`ECS::Systems::EntityDrawSystem`): per-frame frustum test via
      `Camera3DEXT::GetBoundingFrustumEXT()` + `BoundingFrustum::Intersects` against each
      entity's `ModelComponentEXT::BoundsEXT` transformed by its `Transform3ComponentEXT`
      world matrix (identity if the entity has none); draws via `Model::Draw(world, view,
      projection)` for entities that pass the test.
      `include/CNA/Extended/World3DEXT/RenderSystem3DEXT.hpp` + `src/.../RenderSystem3DEXT.cpp`.
      Same `EntityDrawSystem::Initialize(World&)`-hiding issue as Phase 2's
      `EntityUpdateSystem`, fixed the same way (`using
      ECS::Systems::EntityDrawSystem::Initialize;`).
- [x] Real headless render test (matching `TilemapIntegrationTests.cpp`'s established
      `GraphicsDevice` + render-target + pixel-readback idiom): a hand-built single-
      triangle `Model` (real `VertexBuffer`/`IndexBuffer`/`ModelBone`/`ModelMesh`/
      `ModelMeshPart`/`BasicEffect`, matching `cna`'s own `SkinnedModelEXTTests.cpp`
      GPU-backed-part-construction idiom — no content pipeline in scope, so tests build
      `Model`s directly) actually drawn and confirmed via a sampled red center pixel; a
      second test placing the same model 10,000 units behind the far plane confirms it is
      frustum-culled (no non-black pixels at all). Viewport fixed at `800x480` to match
      `GetBackBufferData`'s window-logical-size expectation (same reason
      `TilemapIntegrationTests.cpp` uses that size, not an arbitrary one — see that file's
      own header comment).
      `tests/CNA/Extended/World3DEXT/RenderSystem3DEXTTests.cpp`, 2 tests.
      Both build configs clean (genuine `rm -rf` + fresh configure + build), full suite
      **2089/2091 passing** (was 2087/2089; 2 pre-existing skips unrelated to this phase).

### Phase 4 — Skinned animation — **COMPLETE (2026-07-14)**

- [x] `SkinnedModelComponentEXT` (`SkinnedModelEXT*` + `SkinnedEffect*` + clip
      name/position/loop + bone transform array + `BoundsEXT`) —
      `include/CNA/Extended/World3DEXT/SkinnedModelComponentEXT.hpp`. **Reuses `cna`'s own
      `Microsoft::Xna::Framework::Graphics::SkinnedModelEXT` directly rather than building
      a new skinned-model type** — discovered during this phase that `cna` already has a
      real, tested bone-hierarchy + keyframe-interpolation sampler
      (`SkinnedModelEXT::ComputeBoneTransformsEXT`), originally built for
      `AvatarRenderer::EnableRealRenderingEXT` but avatar-agnostic in its actual logic (see
      `SkinnedModelEXT.hpp`'s own header comment). **Resolves `3d.md` §9.1's ownership
      question**: `EffectEXT` is a non-owning `SkinnedEffect*`, one instance per animated
      entity (bone transforms differ per entity, so it can't be shared the way a stateless
      effect could be) — matches every other component in this ECS being purely
      non-owning, and mirrors `AvatarRenderer::DrawRealEXT`'s real, already-working usage:
      a *single* `SkinnedEffect` shared across a model's `Parts` (not a per-`ModelMeshPart`
      effect the way `ModelComponentEXT`'s plain `Model` uses).
- [x] `AnimationSystem3DEXT` (`ECS::Systems::EntityUpdateSystem`): advances
      `PositionEXT` by the frame's elapsed time and recomputes `BoneTransformsEXT` via
      `SkinnedModelEXT::ComputeBoneTransformsEXT`. **Resolves `3d.md` §9.3**: a dedicated
      system, not a reuse of `Animations::AnimationController` — that class's discrete
      fixed-duration-per-frame sprite-sheet timing model doesn't fit continuous keyframe
      interpolation across arbitrarily-spaced `TimeSpan`-keyed keyframes, which
      `ComputeBoneTransformsEXT` already implements correctly; this system is a thin
      per-entity driver over it, not a new timing engine.
      `include/CNA/Extended/World3DEXT/AnimationSystem3DEXT.hpp` + `src/.../AnimationSystem3DEXT.cpp`.
- [x] `RenderSystem3DEXT` extended: constructor's `Aspect` changed from `All({ModelComponentEXT})`
      to `One({ModelComponentEXT, SkinnedModelComponentEXT})`; `Draw()` now handles each
      component independently per entity. The skinned path needs a `GraphicsDevice&`
      reference (re-added to the constructor) since, unlike `Model::Draw()`,
      `SkinnedModelEXT` has no built-in draw method — the skinned path hand-draws each
      part following `AvatarRenderer::DrawRealEXT`'s exact sequence (set
      World/View/Projection/BoneTransforms once, then per part: texture + `Apply()` +
      `SetVertexBuffer`/`SetIndexBuffer`/`DrawIndexedPrimitives`).
- [x] Real headless render test proving a skinned model's pose changes between two
      different animation times produce different pixel output: a 2-bone rig (root +
      moving child bone, translating 0→5 units over a 1-second clip) with a triangle
      100%-weighted to the moving bone; sampling the screen center at clip position 0s
      (triangle visible, non-black red pixel) vs. position 1s (triangle translated away,
      different pixel) confirms the pose change is visible. A second, non-rendering test
      confirms `AnimationSystem3DEXT::Update()` alone advances `PositionEXT` and produces
      the expected bone-1 world translation. Lighting/rasterizer setup (ambient white +
      one enabled directional light + `RasterizerState::CullNone`) copied from `cna`'s own
      proven-working recipe (`cna/examples/avatar_tint_routing_integration_test.cpp`),
      not re-derived. `tests/CNA/Extended/World3DEXT/AnimationSystem3DEXTTests.cpp`, 2 tests.
      Both build configs clean (genuine `rm -rf` + fresh configure + build), full suite
      **2091/2093 passing** (was 2089/2091; 2 pre-existing skips unrelated to this phase).

### Phase 5 — `Collisions3DEXT` (3D counterpart of `Collisions`) — **COMPLETE (2026-07-14)**

Mirrors `CNA::Extended::Collisions`'s shape (`CollisionWorld2D`, a broadphase
(`QuadTree`/`SpatialHash`), narrow-phase `ICollisionActor`/`CollisionShape2D`) in 3D,
reusing `cna`'s real `BoundingBox`/`BoundingSphere` (already used for frustum culling in
Phase 3) instead of `cna-extended`'s existing 2D shape types. **Corrections made during
implementation to this task's original wording** (the module is `CNA::Extended::Collisions`,
not `Collisions2D`; `ICollisionActor` has no `OnCollisionEXT` callback — it is exactly
`getIdProperty()`/`getShapeProperty()`; real `CollisionWorld2D` is a query-oriented API
(`QueryCandidates`/`QueryCollisions`/`QueryCollisionPairs`), not an `Update`-driven
event-raising one — `CollisionWorld3DEXT` mirrors the real, query-oriented shape instead
of the imagined design):

- [x] `CollisionShape3DEXT` (3D counterpart of `CollisionShape2D`, scoped to Box/Sphere —
      not the full Box/Circle/OrientedBox/Capsule/Polygon 5-kind set, which Phase 5's own
      bullet list never called for) + `CollisionResult3DEXT`/`CollisionShapeKind3DEXT`
      (3D counterparts of `CollisionResult2D`/`CollisionShapeKind2D`). `Intersects()`
      delegates to `BoundingBox`/`BoundingSphere`'s own real methods; `TryGetCollision()`'s
      minimum-translation-vector math (Box/Box per-axis overlap, Sphere/Sphere
      center-distance-minus-radii, Box/Sphere closest-point) is standard collision-
      resolution math, not adapted from any specific source — same category as
      `CollisionShape2D`'s own hand-written legacy-penetration helpers.
      `include/CNA/Extended/World3DEXT/CollisionShape3DEXT.hpp` +
      `CollisionResult3DEXT.hpp`/`CollisionShapeKind3DEXT.hpp` + matching `.cpp` files.
- [x] `ICollisionActor3DEXT` (3D counterpart of `Collisions::ICollisionActor`):
      `getIdProperty()`/`getShapeProperty() -> CollisionShape3DEXT`, exactly mirroring the
      real upstream-ported shape. `CollisionEvent3DEXT`/`CollisionPair3DEXT`/
      `ActorPairKey3DEXT` (3D counterparts of `CollisionEvent2D`/`CollisionPair2D`/
      `ActorPairKey`) round out the query-result types.
- [x] `OctreeEXT` (3D counterpart of `QuadTree`) — **design correction, documented
      transparently in its own header comment**: implemented as a fixed-cell-size 3D
      spatial hash (`Collisions::SpatialHash`'s own real algorithm extended with a Z
      axis), not a true recursive octree with node splitting — a uniform grid hash
      generalizes far more directly from 2D to 3D than `QuadTree`'s recursive-subdivision
      logic does, while still being a real spatially-accelerated broadphase, matching this
      task's own "start with the simplest correct version" instruction. True recursive
      subdivision remains a documented future option if profiling ever shows a need.
      `include/CNA/Extended/World3DEXT/OctreeEXT.hpp` + `src/.../OctreeEXT.cpp`.
- [x] `CollisionWorld3DEXT` (3D counterpart of `CollisionWorld2D`): `Insert`/`Remove`/
      `Contains`/`QueryCandidates`/`QueryCollisions`/`QueryCollisionPairs`/`Rebuild`, owning
      a single `ICollisionBroadphase3DEXT` (`OctreeEXT` by default). **Deliberately scoped
      down**: no named-`Layer`/`LayerPair`/cross-layer-filtering system — not called for by
      this task's own bullet list, and a real multi-layer 3D use case can be added later
      without touching `ICollisionBroadphase3DEXT` or `CollisionShape3DEXT`.
      `include/CNA/Extended/World3DEXT/CollisionWorld3DEXT.hpp` + `src/.../CollisionWorld3DEXT.cpp`.
- [x] Tests mirroring `Collisions`'s existing test coverage/structure
      (`CollisionShape2DTests.cpp`'s "spot-check pair per delegation branch" philosophy,
      `SpatialHashTests.cpp`, `CollisionWorld2DTests.cpp`) adapted to 3D scenarios:
      `CollisionShape3DEXTTests.cpp` (11 tests: None/Box-Box/Sphere-Sphere/Box-Sphere/
      Sphere-Box `Intersects`+`TryGetCollision`, `CollisionResult3DEXT::Invert`),
      `OctreeEXTTests.cpp` (9 tests: multi-cell/negative-coordinate/insert-dedup/remove/
      reset/sphere-bounds scenarios), `CollisionWorld3DEXTTests.cpp` (8 tests: Contains/
      Remove/QueryCandidates/QueryCollisions/QueryCollisionPairs/Rebuild). 28 tests total.
      Both build configs clean (genuine `rm -rf` + fresh configure + build), full suite
      **2119/2121 passing** (was 2091/2093; 2 pre-existing skips unrelated to this phase).
      **`OctreeEXT` renamed to `SpatialHash3DEXT` in Phase 10 (2026-07-14, A-05)** — see that
      entry below; this Phase 5 entry is left describing the original name/files as they
      actually were at the time, not rewritten to match the later rename.

### Phase 6 — `Graphics3DEXT` (3D counterpart of `Graphics`) — **COMPLETE (2026-07-14)**

Absorbs and renames the original Phase 5 draft ("cube/billboard/debug-draw helpers") —
this is now the single phase covering every `Graphics3DEXT` rendering helper, mirroring
`CNA::Extended::Graphics`'s `Sprite`/`AnimatedSprite`/`SpriteBatchExtensions` shape in 3D.
**Resolves `3d.md` §9.2's deferred `NOTICE.md` question**: no entry needed — `cna-scene`
and `cna-extended` share the same copyright holder (Robert Vokáč), unlike the genuinely
third-party MIT code this project's `NOTICE.md` documents (SlimMath, nickgravelyn's
Triangulator, artemis_CSharp); each file below documents its `cna-scene` inspiration in
its own header comment instead, and none of them are literal copies (see each file's own
note on how its design actually differs from `cna-scene`'s immediate-mode API).

- [x] `CubeMeshComponentEXT` / `CubeMeshRenderSystemEXT` — one shared unit-cube mesh (24
      vertices, 4 per face for independent UVs) + one shared `BasicEffect` owned by the
      system (texture/tint swapped per draw call, matching `TilemapRenderer`'s/
      `AvatarRenderer`'s established pattern); the component only carries per-instance
      `TextureEXT`/`SizeEXT`/`TintEXT`.
- [x] `BillboardComponentEXT` / `BillboardRenderSystemEXT` — `Matrix::CreateBillboard`
      (real CNA/XNA math) recomputes orientation every frame from the current camera
      position; each entity owns a small 4-vertex `VertexBufferEXT` (its own baked-in UV
      rect, via `BillboardMeshEXT.hpp`'s `BuildBillboardQuadVertexBufferEXT`/
      `ConvertPixelRectToUvRectEXT` helpers), the system owns one shared `IndexBuffer` +
      `BasicEffect`.
- [x] `AnimatedBillboardComponentEXT` / `AnimatedBillboardSystemEXT` — reuses
      `Graphics::SpriteSheet`/`Texture2DAtlas`/`Animations::AnimationController` exactly
      as `AnimatedSprite::Update()` does (not reinvented); on each detected frame change,
      re-derives the sibling `BillboardComponentEXT`'s UV from the new frame's
      `Texture2DRegion` and re-uploads its `VertexBufferEXT`. **Bug caught and fixed
      before landing**: syncing only on frame *change* left a newly-attached entity with
      no initial texture/UV until the animation's first frame boundary elapsed; fixed by
      also syncing in `OnEntityAdded` (matching `AnimatedSprite::SetAnimation`'s own
      immediate-assignment behavior) — caught by the render test itself, not by
      inspection.
- [x] `Text3DEXT` / `TextBillboardRenderSystemEXT` / `BuildText3DMeshEXT` — reuses
      `BitmapFont::GetGlyphs` (real kerning/line-layout, not reimplemented) to build one
      combined multi-glyph quad mesh per string (Y-flipped from BMFont's Y-down layout
      into this project's Y-up local space), billboarded as a whole via the same
      `CreateBillboard` math. **Known simplifications, documented in the header
      comments**: assumes a single-page font (glyphs from a second page are skipped); no
      frustum culling (`Text3DEXT` has no cached bounds, and text labels are rarely
      numerous enough for it to matter) — both can be added later if a real need appears.
- [x] `DebugDrawComponentEXT` / `DebugDrawSystemEXT` — a per-entity world-space line list,
      batched into one shared, growth-only `VertexBuffer` + `PrimitiveType::LineList` draw
      call per frame. `AddDebugBoxLinesEXT`/`AddDebugFrustumLinesEXT` cover exactly what
      this task calls for (`Collisions3DEXT` bounds via `CollisionShape3DEXT::
      getBoundingBoxProperty()`, `Camera3DEXT` frustums) via one shared
      corners-to-12-edges helper, since `BoundingBox::GetCorners()`/`BoundingFrustum::
      GetCorners()` share the same real CNA 8-corner ordering. Sphere wireframes are not
      included in this first pass (box wireframes already cover every collision shape
      kind Phase 5 supports) — documented as a deferred, not forgotten, scope decision.
- [x] Tests for each, matching the real-render-test bar established in Phase 3:
      `CubeMeshRenderSystemEXTTests.cpp` (2), `BillboardRenderSystemEXTTests.cpp` (2,
      including a camera-orbits-the-billboard test proving `CreateBillboard` really keeps
      it face-on), `AnimatedBillboardSystemEXTTests.cpp` (1, real two-frame render showing
      different colors at different animation times), `Text3DEXTTests.cpp` (3: mesh
      vertex/index-count checks + a real render showing both glyph colors),
      `DebugDrawSystemEXTTests.cpp` (4: edge-count checks + real box-wireframe render).
      12 new tests total, all passing on first real run except the AnimatedBillboard
      initial-sync bug caught above. Both build configs clean (genuine `rm -rf` + fresh
      configure + build), full suite **2131/2133 passing** (was 2119/2121; 2 pre-existing
      skips unrelated to this phase).

      **Revisited after Phase 9, same day**: `CubeMeshRenderSystemEXT`'s original fully
      unlit rendering (`VertexPositionTexture`, no normals) was visually flat when the
      owner reviewed `world3d_demo`'s screenshot. Switched to `VertexPositionNormalTexture`
      with real per-face flat normals, enabled `BasicEffect` lighting (ambient +
      `EnableDefaultLighting()`, `SpecularColor` forced to zero — its default specular
      highlight produced its own bright pseudo-"seam" artifact across many adjacent
      flat-shaded cubes at a shallow angle, easy to mistake for a real bug). All existing
      tests re-verified passing unchanged (their assertions tolerate real shading; see
      `NEXT.md` section 5 for the full account, including a second, unrelated, genuinely
      serious bug found in the same pass — a missing depth buffer in this project's
      established headless-render-test `RenderTarget2D` idiom, invisible until a real
      multi-object 3D scene existed to expose it).

### Phase 7 — `Particles3DEXT` (3D counterpart of `Particles`) — **COMPLETE (2026-07-14)**

Mirrors `CNA::Extended::Particles`'s `ParticleEffect`/`ParticleEmitter`/`ParticleBuffer`
shape, changing 2D position/velocity fields to `Vector3` and rendering through
`Graphics3DEXT`'s billboard pipeline (Phase 6) instead of `SpriteBatch`. **Deliberately
scoped down**, discovered/decided during implementation once the 2D module's actual size
was read (~13 `Modifier` subclasses, 5 `Profile` subclasses, 6 `Interpolator` subclasses):
rather than port that full plugin architecture, `ParticleEmitter3DEXT` has ONE built-in
emission shape (a cone; `ConeHalfAngleEXT == Pi` degenerates to full-sphere emission) and
THREE built-in per-frame behaviors baked directly into `UpdateEXT` (linear gravity,
age-based expiry, start/end color+opacity interpolation) — covering `AgeModifier`'s/
`LinearGravityModifier`'s/`ColorInterpolator`'s/`OpacityInterpolator`'s conceptual roles
without their extensibility framework. A real, working, tested particle system, just not a
plugin architecture; the full `Profile`/`Modifier`/`Interpolator` port can be added later
if a real need appears — matching this plan's repeated "start with the simplest correct
version" precedent (`SpatialHash3DEXT`, `CollisionWorld3DEXT`).

- [x] `Particle3DEXT` (3D counterpart of `Particles::Data::Particle` — position/velocity/
      color/scale as `Vector3`/`Color`/`float` fields instead of the 2D packed-float-array,
      `#pragma pack(1)` layout, held in a plain `std::vector<Particle3DEXT>` rather than
      upstream's raw-pointer-walked `ParticleBuffer` — that packing existed to match a C#
      `[StructLayout(Pack=1)] unsafe struct`'s exact layout, which has no counterpart for
      new, non-upstream code). `include/CNA/Extended/World3DEXT/Particle3DEXT.hpp` + `.cpp`.
- [x] `ParticleEmitter3DEXT` / `ParticleEffect3DEXT` (3D counterparts of `ParticleEmitter`/
      `ParticleEffect` — see the scope note above for what's actually included).
      `ParticleEffect3DEXT` owns 1+ `ParticleEmitter3DEXT`s, driving them together from one
      world position (e.g. a torch = "flame" + "smoke" emitters). `System::Random` (real
      sharp-runtime type, not a hand-rolled RNG) drives cone-direction/speed/lifetime/scale
      sampling. `include/CNA/Extended/World3DEXT/ParticleEmitter3DEXT.hpp` +
      `ParticleEffect3DEXT.hpp` + matching `.cpp` files.
- [x] `ParticleEffectComponentEXT` (ECS component pairing a `ParticleEffect3DEXT*` with the
      texture its particles draw with — one texture per effect, not per-particle, the
      simplest correct scope) + `ParticleUpdateSystem3DEXT` (`EntityUpdateSystem`, advances
      each active entity's effect from its `Transform3ComponentEXT` world position, or
      `Vector3::Zero` if it has none).
- [x] `ParticleRenderSystem3DEXT` (draws active 3D particles as billboards via
      `Graphics3DEXT::BillboardRenderSystemEXT`'s existing draw path, not a new one) —
      **required a small `BillboardRenderSystemEXT` refactor to actually satisfy this**:
      extracted its per-entity draw step into a new public `DrawBillboardEXT(VertexBuffer&,
      Texture2D*, Vector3 worldPosition, Vector2 size, Color tint)` method, called both by
      `BillboardRenderSystemEXT::Draw()`'s own entity loop and by
      `ParticleRenderSystem3DEXT` for each active particle — one genuinely shared draw
      path, not a duplicated one. `ParticleRenderSystem3DEXT` owns one shared, full-texture
      ([0,1] UV) quad `VertexBuffer` (particles don't need atlas sub-regions in this scope),
      reused for every particle every frame.
      `include/CNA/Extended/World3DEXT/ParticleRenderSystem3DEXT.hpp` + `.cpp`.
- [x] Tests mirroring `Particles`'s existing coverage, adapted to 3D scenarios:
      `ParticleEmitter3DEXTTests.cpp` (9: emit count/cap/origin, age/velocity/gravity
      integration, expiry, color/opacity interpolation, emission-rate-over-time, and
      `ParticleEffect3DEXT` multi-emitter forwarding), `ParticleRenderSystem3DEXTTests.cpp`
      (2: a real headless render showing an emitted particle's tint color on screen, and a
      no-particles-drawn-nothing case). One real test bug caught and fixed during this
      phase (not a product bug): a gravity-integration test advanced time by exactly the
      particle's default 1-second lifetime, so the particle expired and was removed before
      the assertion could read its velocity — fixed by giving that test a longer lifetime.
      11 new tests total. Both build configs clean (genuine `rm -rf` + fresh configure +
      build), full suite **2142/2144 passing** (was 2131/2133; 2 pre-existing skips
      unrelated to this phase).

### Phase 8 — `Tilemaps3DEXT` (3D/voxel counterpart of `Tilemaps`) — **COMPLETE (2026-07-14)**

Mirrors `CNA::Extended::Tilemaps`'s data-model/rendering split (`Tilemap`/
`TilemapTileLayer`/`TilemapTileset` + `Rendering::TilemapRenderer`) for a 3D/voxel grid,
building on `Graphics3DEXT::CubeMeshRenderSystemEXT` (Phase 6, extended this phase — see
below) for the actual per-tile rendering. **Scope decisions made during this phase** (per
this task's own "decide during this phase" wording):

- [x] `Tilemap3DEXT` (3D counterpart of `Tilemap`/`TilemapTileLayer`, absorbed into one
      type rather than kept separate — see below): a single sparse `(X, Y, Z)` → tile ID
      grid (`std::unordered_map<TileCoordinate3DEXT, int>`), not a stack of 2D-style
      layers — a real voxel/block world (Minecraft-style) is one 3D grid, not layers; a
      layer stack isn't the natural 3D analog of `TilemapTileLayer`. Sparse, not a dense
      3D array: real voxel worlds are overwhelmingly empty space, so a dense array would
      waste memory proportional to the full bounding volume. Tile ID `0` means empty
      ("air"), the common voxel-engine convention. `TilemapTileLayer3DEXT` was judged
      unnecessary once `Tilemap3DEXT` itself became the single grid — no separate layer
      type exists to wrap. `include/CNA/Extended/World3DEXT/Tilemap3DEXT.hpp` + `.cpp`.
- [x] `TilemapTileset3DEXT` (3D counterpart of `TilemapTileset`, also scoped down): maps a
      tile ID directly to a whole `Texture2D*` (applied to all 6 faces via
      `CubeMeshRenderSystemEXT::DrawCubeEXT`), not a per-tile source rectangle within a
      shared atlas — a voxel/block world conventionally gives each block type its own
      whole texture rather than packing many block types into one shared 2D-style atlas.
      `include/CNA/Extended/World3DEXT/TilemapTileset3DEXT.hpp` + `.cpp`.
- [x] `Tilemap3DFactoryEXT` (3D counterpart of `TilemapFactory`): **hand-built data only**
      — `BuildFromArrayEXT(tileIds, width, height, depth, tileSize)` from a flat row-major
      array. No 3D-extended Tiled/Ogmo JSON reader: those formats are all 2D-only with no
      voxel/3D grid concept, and inventing a new file format to also parse was judged out
      of scope for this phase (explicitly permitted by this task's own wording).
      `include/CNA/Extended/World3DEXT/Tilemap3DFactoryEXT.hpp` + `.cpp`.
- [x] `TilemapRenderer3DEXT` (3D counterpart of `Tilemaps::Rendering::TilemapRenderer`):
      **a standalone renderer object with its own `Draw(camera, tilemap, tileset)`
      method**, matching the real 2D `TilemapRenderer`'s actual shape (driven directly by
      game code, not an ECS system — unlike every other `*System3DEXT` type in
      `World3DEXT`, deliberately not `TilemapRenderSystem3DEXT`, matching this task's own
      naming). Required extending `CubeMeshRenderSystemEXT` the same way Phase 7 extended
      `BillboardRenderSystemEXT`: extracted its per-cube draw step into a new public
      `DrawCubeEXT(Texture2D*, const Matrix& world, const Color& tint)` method (taking a
      full world matrix, not just a position, unlike `DrawBillboardEXT` — cube entities
      support rotation via `Transform3ComponentEXT`, so a position-only signature would
      have silently dropped that for the shared path), called both by
      `CubeMeshRenderSystemEXT::Draw()`'s own entity loop and by `TilemapRenderer3DEXT`
      per visible tile — one genuinely shared draw path. Frustum-culls each tile
      individually via `Camera3DEXT`'s `BoundingFrustum` (the simplest correct version of
      "per-chunk/per-tile culling"; per-chunk geometry batching remains a documented
      future optimization, not assumed needed without profiling).
      `include/CNA/Extended/World3DEXT/TilemapRenderer3DEXT.hpp` + `.cpp`.
      **`Collisions3DEXT` tile-collision integration was judged out of scope for this
      phase**: `Tilemap3DEXT`/`CollisionWorld3DEXT` are independently usable today (a
      caller can insert one `ICollisionActor3DEXT` per populated tile via
      `Tilemap3DEXT::getTilesProperty()`), but a purpose-built "tilemap-aware" broadphase
      shortcut (e.g. querying the grid directly instead of via `SpatialHash3DEXT`) was not called
      for by this task's own bullet list beyond "building on Collisions3DEXT for tile
      collision" — no code needed writing to satisfy that; can be added later as a
      dedicated integration if a real need appears.
- [x] Tests mirroring `Tilemaps`'s existing coverage, adapted to a 3D grid:
      `Tilemap3DEXTTests.cpp` (10: `Tilemap3DEXT` get/set/remove/has/world-position/
      populated-tiles-only, `Tilemap3DFactoryEXT` array-build + mismatched-size exception,
      `TilemapTileset3DEXT` get/set), `TilemapRenderer3DEXTTests.cpp` (3: a real render
      showing a visible tile's pixel color, a far-outside-frustum tile correctly culled,
      and a no-texture-assigned tile not throwing). 13 new tests total. Both build configs
      clean (genuine `rm -rf` + fresh configure + build), full suite **2155/2157 passing**
      (was 2142/2144; 2 pre-existing skips unrelated to this phase).

### Phase 9 — `World3DScreenEXT` — **COMPLETE (2026-07-14)**

- [x] `World3DScreenEXT : Screens::Screen` owning an `ECS::World` + `Camera3DEXT`,
      `GetWorld3DEXT()`/`GetCamera3DEXT()` accessors — ties Phases 1-8 together into one
      convenience base class for real game screens, reusing `Screen`'s real
      `Initialize`/`Update`/`Draw` lifecycle and `ScreenManager`'s existing stack/
      transition support unchanged (no new Scene/SceneManager type — see `3d.md` §6.8).
      A derived screen overrides `ConfigureWorldEXT(WorldBuilder&)` (called once, from
      `Initialize()`, before the `World` is built) to add its own systems; a derived
      class overriding `Initialize()` itself must call the base implementation first so
      `GetWorld3DEXT()` is valid before its own setup runs — documented in the header,
      exercised directly by the example below.
      `include/CNA/Extended/World3DEXT/World3DScreenEXT.hpp` + `src/.../World3DScreenEXT.cpp`.
- [x] An end-to-end example under `examples/world3d_demo/` (matching Phase 10's
      `examples/tiled_demo/` precedent — real headless `GraphicsDevice` + off-screen
      `RenderTarget2D`, `GetBackBufferData` readback, a saved PNG, pixel-sampled
      assertions, a real 0/1 exit code): a `World3DScreenEXT` subclass wiring up
      `TransformHierarchySystemEXT`/`AnimationSystem3DEXT`/`ParticleUpdateSystem3DEXT`/
      `RenderSystem3DEXT`/`CubeMeshRenderSystemEXT`/`BillboardRenderSystemEXT`/
      `ParticleRenderSystem3DEXT` via `ConfigureWorldEXT`, then creating: a parent/child
      cube hierarchy (a pillar + a cap cube riding on it via `Transform3ComponentEXT`/
      `TransformHierarchySystemEXT` — no per-frame code needed to keep the child
      attached), a cube deliberately placed 10,000 units behind the far plane (proving
      per-entity frustum culling), a two-bone skinned "character" (the same hand-built
      rig shape `AnimationSystem3DEXTTests.cpp` uses — no content pipeline in scope, see
      the root `CLAUDE.md`), a rising spark `ParticleEffect3DEXT` burst, and a
      `Tilemap3DEXT` voxel floor drawn each frame via `TilemapRenderer3DEXT` (a
      standalone renderer, called directly from the screen's own `Draw()` override,
      matching that type's own non-ECS shape from Phase 8). Verified by actually running
      the built executable, not just compiling it: real center-pixel sampling across 10
      simulated frames consistently shows the pillar's tint color, spark-colored pixels
      are detected, and the final frame's saved PNG was inspected and visually confirms
      a coherent 3D scene (pillar, cap, floor, particles, character all present).
      `examples/world3d_demo/CMakeLists.txt` + `main.cpp`, wired into the root
      `CMakeLists.txt`'s `add_subdirectory` list next to `tiled_demo`.
- [x] `World3DScreenEXTTests.cpp` (2 tests: a real headless render proving
      `Initialize`/`Update`/`Draw` reach a configured `CubeMeshRenderSystemEXT` and
      actually draw, and that `GetCamera3DEXT()` is independently usable before
      `Initialize()` builds the `World`). Both build configs clean (genuine `rm -rf` +
      fresh configure + build), full suite **2157/2159 passing** (was 2155/2157; 2
      pre-existing skips unrelated to this phase).

### Phase 10 — Audit follow-ups (2026-07-14)

An independent audit (Codex, `bf71945`) reviewed the full port and `World3DEXT`, saved to
[`audit.md`](audit.md). Overall verdict: 2D port 8/10, `World3DEXT` 6/10
production-readiness ("a capable first version... treat as experimental/early-production
until the high-priority findings are fixed"). Each finding below was independently
re-verified against the actual code before being accepted (per this project's standing
"verify before trusting" discipline) — none were fixed on the audit's word alone. Findings
keep the audit's own IDs (A-01 etc.) for traceability back to `audit.md`.

- [x] **A-01 (High)** — `CollisionShape3DEXT::TryGetBoxSphereCollision`'s fallback (sphere
      center inside the box) returns `depth == sphere.Radius`, not the true separating
      distance, and picks a box-center-to-sphere-center direction that isn't necessarily
      an outward face normal — confirmed by re-reading the code: doesn't actually move the
      box out of the sphere in general. Fix: find the nearest of the box's 6 faces, use its
      outward normal, and set `depth = distanceToThatFace + sphere.Radius`. Add tests: dead
      center, all 6 near-face cases, a non-uniform (non-cube) box, and the inverse
      (`sphere.TryGetCollision(box)`) call.
      **Round 2 (caught by a follow-up Codex re-review, 2026-07-14, independently
      re-verified before fixing — same discipline extended to a re-review of a re-review):
      round 1's fix moved the box *through* its nearest face, not away from it.**
      Counter-example: box `[-2,2]` on X, sphere center `x=1.5`, radius `1` — round 1
      returned `+X`/depth `1.5`, translating the box to `[-0.5, 3.5]`, which still overlaps
      the sphere's `[0.5, 2.5]` interval. Proven via direct SAT-style derivation: the
      minimal depth (nearest-face distance + radius) was already correct, but the escape
      direction must be the *opposite* of the nearest face's own outward normal (matches
      the already-correct sphere-outside-the-box branch's convention: move away from the
      sphere's approach side, not through it). Fixed by flipping the sign; round 1's test
      values (which asserted specific field values matching the buggy direction, not actual
      separation) were corrected, and a new `ExpectMtvActuallySeparates` helper was added
      that translates the box by the MTV and checks real non-overlap against the sphere's
      exact per-axis interval — this is what should have caught round 1's bug and is now in
      place so a future regression would be caught the same way.
- [x] **A-02 (High)** — `TransformHierarchySystemEXT::Update` wires any non-negative
      `ParentEntityIdEXT` without checking for self-parenting or indirect cycles —
      confirmed by re-reading the code. `Transform3`'s world-matrix recomputation
      recurses through the parent chain, so `A -> B -> A` (or direct self-parenting) risks
      unbounded recursion / stack overflow. Fix: walk the *ECS* `ParentEntityIdEXT` chain
      (not yet-wired `Transform3` pointers) before calling `setParentProperty`; reject
      self-parenting and any candidate parent whose own ancestor chain already contains
      this entity, falling back to no-parent (detached) for the rejected edge. Add tests:
      self-parent, 2-node cycle, 3+-node cycle, parent entity with no `Transform3ComponentEXT`.
- [x] **A-04 (Medium)** — `BillboardRenderSystemEXT`'s frustum-culling radius is
      `max(width, height) / 2`, not `Vector2(width, height).Length() / 2` (half the
      diagonal) — confirmed by re-reading the code. A billboard's corners can be
      incorrectly culled while still on-screen. Fix: use the diagonal-based radius. Add a
      boundary test: a wide/tall billboard whose center sits just outside a frustum plane
      while a corner should still be visible under the corrected radius.
- [x] **A-06 (Medium)** — confirmed by re-reading the code: (1)
      `ParticleEmitter3DEXT::SampleConeDirectionEXT` calls `Vector3::Normalize
      (ConeDirectionEXT)` with no zero-vector guard (undefined/NaN result if a caller
      leaves or sets it to `Vector3::Zero`); (2) `ParticleRenderSystem3DEXT` casts
      `particle.OpacityEXT * 255.0f` straight to `std::uint8_t` with no clamping --
      out-of-[0,1] opacity is real UB in C++, not just wraparound. Fix: default/guard a
      zero `ConeDirectionEXT` in `EmitEXT`/`SampleConeDirectionEXT`; clamp opacity to
      [0,1] before the byte conversion (in `ParticleEmitter3DEXT::UpdateEXT`, where
      `OpacityEXT` is actually computed, not at the render call site). Add tests for both.
      **Follow-up robustness pass (2026-07-14, not a new audit finding — a follow-up review
      confirming NaN/reversed-range handling wasn't fully nailed down yet)**: "reversed"
      Min/Max ranges (Min > Max) turned out not to be a bug at all — `min + t*(max-min)`
      for `t` in `[0,1)` always lands in `[min,max]` regardless of which is larger, proven
      and locked in by a new test. The real gap was non-finite (NaN/Infinity) inputs: a
      non-finite `LifetimeEXT` makes `IsExpiredEXT()`'s `AgeEXT >= LifetimeEXT` false
      forever (NaN compares false against everything in IEEE 754), leaking the particle
      permanently instead of ever removing it. `EmitEXT` now guards `LifetimeEXT`,
      speed-derived `VelocityEXT`, and `ScaleEXT` against non-finite results, falling back
      to values that keep the particle either already-expired (lifetime) or harmlessly
      invisible (velocity/scale) rather than propagating NaN/Infinity. 5 new tests. Both
      configs verified; full suite green (2181/2181).
- [x] **A-08 (Medium)** — `ModelComponentEXT`/`SkinnedModelComponentEXT::BoundsEXT`
      default to a zero-radius `BoundingSphere` and are easy to leave unset, silently
      culling a real model every frame with no warning. Added `ComputeModelBoundsEXT(const
      Model&)` (`ModelBoundsEXT.hpp`/`.cpp`) that merges `ModelMesh::getBoundingSphereProperty()`
      across every mesh via `BoundingSphere::CreateMerged`, per the audit's own suggestion.
      **Important limitation found while implementing, not caught by the audit itself**:
      `cna`'s `ModelMesh::boundingSphere_` has no public setter and is never written by any
      code path in `cna` today (confirmed by grepping `cna`'s entire src/include tree) --
      `getBoundingSphereProperty()` always returns a zero-radius sphere at the origin for
      every `Model` constructible today. The helper is correct and forward-compatible (the
      moment `cna` gains any way to populate real per-mesh bounds, this starts working with
      zero `cna-extended` changes), but is NOT yet a full fix for the silent-culling concern
      -- a real fix needs per-vertex position data, and `cna`'s `VertexBuffer::GetData` only
      exposes fixed per-vertex-type overloads, not a generic/type-erased read, so doing that
      generically means changing `cna` itself (out of scope without the user's explicit
      go-ahead — root `CLAUDE.md`: "Do NOT modify sibling repositories"). Documented
      prominently in `ModelBoundsEXT.hpp`'s header comment and both components' `BoundsEXT`
      doc comments rather than silently declared "fixed". `SkinnedModelComponentEXT::BoundsEXT`
      has no equivalent helper at all (`SkinnedModelEXT` parts have no per-part
      `BoundingSphere`, same vertex-readback blocker) — its doc comment was strengthened
      instead, matching this finding's own documented fallback bar.
- [x] **A-09 (Medium)** — status docs are stale/contradictory, confirmed directly:
      `3d.md`'s own top status line still said "still needs explicit phase-by-phase
      approval before any code is written" despite `plan3d.md` recording that approval and
      all 9 phases as complete; `README.md` said "2042/2042 tests passing" (predates even
      `plan.md`'s own later phases, let alone `World3DEXT`). Fixed: `3d.md`'s status line
      now matches `plan3d.md`'s actual status; `README.md`'s Status section now describes
      `World3DEXT`'s existence and current test count (2175/2175, matching `NEXT.md`).
      Re-scanned `NEXT.md` for other stale figures: found and fixed two more "current
      status" claims presented as live facts (section 1's "2157/2159 passing" and section
      2's "2079/2079", both now 2175/2175 or 2175/2177 as appropriate) plus section 1's
      "Current phase: DONE, both plans" line (now reflects Phase 10 in progress). Left
      alone: figures inside historical narration describing a *specific past fix*'s
      point-in-time verification (e.g. "full ctest (2079/2079, up from 2063 — 16 new
      regression tests)") — those are truthful accounts of what happened at that commit,
      not claims about current state, and rewriting them would corrupt the historical
      record rather than fix a staleness bug.
- [x] **A-07 (Medium)** — most `*EXT` render tests draw exactly one isolated object, so
      the missing-depth-buffer class of bug this session already found and fixed in
      `world3d_demo` (see `NEXT.md` section 5) has no regression test protecting it in the
      actual test suite. Add a depth-enabled multi-object test (two overlapping cubes
      drawn in both submission orders, asserting the *nearer* one's color wins either way)
      to lock in that fix.
- [x] **A-05 (Medium, addressed)** — `OctreeEXT` was honestly documented as a fixed-cell
      spatial hash, not a hierarchical octree (its own header comment already said so), but
      the audit was right that the class *name* still invited the opposite assumption.
      **Renamed to `SpatialHash3DEXT` (2026-07-14, user-requested)** — pure rename, no
      behavior change, all 8 referencing files updated (`SpatialHash3DEXT.hpp`/`.cpp`,
      `SpatialHash3DEXTTests.cpp` — itself renamed from `OctreeEXTTests.cpp` — plus
      `CollisionWorld3DEXT.hpp`/`.cpp`, `CollisionWorld3DEXTTests.cpp`, and two unrelated
      files with only a comment cross-reference: `ParticleEmitter3DEXT.hpp`,
      `Text3DEXT.hpp`), both configs rebuilt clean, full suite unchanged at 2176/2176
      (rename only, no test count change).
      **Dedup fix landed (2026-07-14)**: `Query`'s candidate dedup was `std::find`-in-a-loop
      (linear, O(K) per candidate, O(K^2) worst case) — replaced with a local
      `std::unordered_set<ICollisionActor3DEXT*>` for O(1) average membership checks. Pure
      internal change, no API/behavior change (existing
      `QueryWhenActorOverlapsMultipleCellsReturnsUniqueActor` test already exercises the
      dedup path and still passes). Not persisted across calls, so no invalidation concern
      on `Insert`/`Remove`/`Reset`.
      **Still explicitly not pursued**: a true recursive octree (matching the class's
      *algorithm* to what its original name implied, as an alternative to the rename)
      remains a documented future option (`SpatialHash3DEXT.hpp`'s own header comment) if
      profiling ever shows the uniform-grid approach's O(N³) cell iteration for
      large/sparse actors is insufficient — not pursued without real large-scene benchmark
      data justifying it, matching this project's own stated "don't optimize on a guess"
      principle.
- [x] **A-03 (High, addressed as documentation, not a code fix)** — the audit's own 22
      graphics-test failures were caused by its sandbox lacking any SDL video device
      (`SDL_InitSubSystem(SDL_INIT_VIDEO) failed: No available video device`), not a
      renderer defect — confirmed not reproducible in *this* environment: every `*EXT`
      render test and `world3d_demo` itself have run successfully via real EasyGL-over-Mesa
      software rendering throughout every phase of this whole session (see `NEXT.md`
      section 1's "genuine `rm -rf` rebuild, full ctest" verification after every phase).
      The audit's underlying point — that headless rendering isn't automatically portable
      to every environment, and this project doesn't document what it actually needs — is
      fair and worth recording explicitly rather than assumed. See `NEXT.md` section 7 for
      the added platform-requirements note.
      **CI actually wired up (2026-07-14)**: `.github/workflows/ci.yml` turns that
      documented recipe into running CI — two jobs, `headers-only` (configure+build with
      `CNA_EXTENDED_LINK_CNA=OFF`, no sibling checkouts needed) and `linked` (checks out
      `cna`/`sharp-runtime`/`easy-gl`/`meta-gl` as siblings, matching `cna`'s own
      `devices-tests.yml` checkout pattern, installs the same system package list plus
      `xvfb`+`libgl1-mesa-dri` for headless software rendering, then runs the full suite
      via `xvfb-run -a ctest`). **Caveat, stated plainly rather than overclaimed**: this
      was validated locally via YAML syntax checking and by carefully mirroring `cna`'s own
      proven-working CI structure + this session's own locally-verified `xvfb-run`
      recipe — it was NOT end-to-end verified by an actual GitHub Actions run before this
      commit (no `gh` CLI auth available in this session to check). The push that lands
      this file will trigger the workflow for real on GitHub's infrastructure; if it fails,
      that's real, actionable signal, not a regression to hide.
- [x] **A-10 (Low, already tracked)** — voxel tilemap rendering's lack of chunk
      meshing/hidden-face culling/batching is already recorded as a deliberate,
      documented scope decision (`plan3d.md`'s own Phase 8 entry, `NEXT.md` section 8's
      deferred-scope list) — the audit independently arriving at the same conclusion is a
      useful confirmation, not a new finding needing its own fix task.

### Phase 11 — 2D/3D parity: collision layers, recursive octree, particles (2026-07-14)

User-requested follow-up after discussing why `World3DEXT` is smaller in scope than the 2D
port in several places: three deliberately-scoped-down areas (all previously documented as
"extend later if wanted" candidates in `NEXT.md`) get brought to parity with their 2D
counterparts, phased smallest/most-mechanical first per the approved plan at
`/home/robertvokac/.claude/plans/rippling-hugging-stardust.md`. Each sub-phase: implement →
port tests alongside → verify both CMake configs via genuine `rm -rf` clean rebuild → commit
→ push → update this file/`NEXT.md` → next phase.

- [x] **Phase A — Collision layers** — mirrors `CollisionWorld2D`'s named-Layer/LayerPair
      cross-layer-filtering system onto `CollisionWorld3DEXT`, method-for-method against
      `CollisionWorld2D.hpp`/`.cpp`. New: `Layer3DEXT.hpp`/`.cpp`, `LayerPair3DEXT.hpp`/`.cpp`
      (mirror `Layer`/`LayerPair`). Reused as-is (no duplicate): `CNA::Extended::Collisions::
      UndefinedLayerException` — a generic `std::string`-message type with no 2D-specific
      logic, so a small `World3DEXT` → `Collisions` include was judged an acceptable trade
      against duplicating an otherwise-identical file (user-confirmed decision).
      `CollisionWorld3DEXT` restructured from a single implicit `broadphase_` member to
      `layers_`/`actorLayerNames_`/`layerCollision_`, gaining `DefaultLayerName`,
      `getLayersProperty`, `SetDefaultLayer`/`AddLayer`/`RemoveLayer`, layer-aware `Insert`,
      `Contains`/`TryGetLayerName`/`GetLayerName`/`MoveToLayer`/`Remove`,
      `RebuildDynamicLayers()` (renamed from `Rebuild()`), layer-scoped
      `QueryCandidates`/`QueryCollisions`/`QueryCollisionPairs`,
      `EnableCollisionBetweenLayers`/`DisableCollisionBetweenLayers`/
      `IsCollisionEnabledBetweenLayers`. Two deliberate 3D-specific deviations from the 2D
      1:1 mirror, both documented in `CollisionWorld3DEXT.hpp`'s own header comment: (1)
      layer-name parameters default to `""` (2D leaves several required) so existing
      zero-arg call sites keep compiling; (2) the no-arg constructor auto-registers a
      `"default"` layer wrapping `SpatialHash3DEXT(16.0f)`, which `CollisionWorld2D()`
      itself deliberately does *not* do (verified directly against `CollisionWorld2D.cpp` —
      its own no-arg constructor leaves `layers_` empty, a real upstream-port API trap this
      port does not reproduce). A third constructor, `CollisionWorld3DEXT(unique_ptr
      <ICollisionBroadphase3DEXT>)`, was kept alongside the new `CollisionWorld3DEXT(
      unique_ptr<Layer3DEXT>)` (2D's own shape) purely for the existing test call site that
      constructs a world directly from a broadphase.
      Tests: `Layer3DEXTTests.cpp` (5, mirrors `LayerTests.cpp`), `LayerPair3DEXTTests.cpp`
      (4, mirrors `LayerPairTests.cpp`), plus 15 new layer-focused cases added to
      `CollisionWorld3DEXTTests.cpp` (named-layer insert, undefined-layer-name throws,
      already-present throws, `TryGetLayerName`/`GetLayerName`/`MoveToLayer`,
      `RebuildDynamicLayers`, cross-layer enable/disable including the
      `CountingShapeActor3DEXT` shape-access-count proof that a disabled pair short-circuits
      before touching actor shapes, `RemoveLayer`) — mirroring the layer-filtering subset of
      `CollisionWorld2DTests.cpp`. 32 new tests total (5+4+23, the last file's original 8
      pre-layer tests kept and 1 renamed for the `Rebuild()`→`RebuildDynamicLayers()` rename).
      Both configs verified; full suite green.
- [x] **Phase B — True recursive octree broadphase** — `OctreeNode3DEXT`/`OctreeNodeData3DEXT`
      (pure recursive tree, mirrors `QuadTree`/`QuadtreeData` method-for-method, 8 octants —
      `Split()` divides `NodeBounds` around `(Min+Max)*0.5` into all 8 min/center × center/max
      combinations per axis — not 4 quadrants; same `DefaultMaxDepth = 7`/
      `DefaultMaxObjectsPerNode = 25` thresholds; same non-exclusive "insert into every
      overlapping child" recursion, dirty-flag traversal dedup, and BFS `Shake()`
      merge-back-down) + `Octree3DEXT` (the public `ICollisionBroadphase3DEXT` adapter,
      mirrors `QuadTreeSpace`: owns every `OctreeNodeData3DEXT` it creates, the tree/its nodes
      hold only non-owning references). Not the default broadphase — usable via the existing
      `CollisionWorld3DEXT(unique_ptr<ICollisionBroadphase3DEXT>)` constructor, exactly as
      `SpatialHash3DEXT.hpp`'s header comment already promised.
      Tests: `OctreeNode3DEXTTests.cpp` (28 tests, full 1:1 port of `QuadTreeTests.cpp`'s
      entire coverage — constructor/`NumTargets`/`Insert`/`Remove`/`RemoveFromAllParents`/
      `Shake`/`Query`, including the 1000-actor incremental-insert and
      2×`DefaultMaxObjectsPerNode` real-multi-level-split-then-query scenarios) +
      `Octree3DEXTTests.cpp` (7 tests, mirrors `QuadTreeSpaceTests.cpp`'s coverage adapted to
      3D per `SpatialHash3DEXTTests.cpp`'s own established pattern — sphere bounds in place
      of 2D's circle/oriented-rectangle cases, `CollisionShape3DEXT` has no oriented-box
      shape). 35 new tests total. Both configs verified; full suite green (2240/2240).
- [x] **Phase C-1 — Particles core architecture + proving slice** — `Profile3DEXT`/
      `Modifier3DEXT`/`ModifierExecutionStrategy3DEXT` (Serial only)/`Interpolator3DEXT`/
      `InterpolatorOfT3DEXT<T>` base types; `Particle3DEXT` gains `RotationEXT`/`MassEXT`/
      `TriggeredPositionEXT`, `ScaleEXT` changes `float`→`Vector2` (user-confirmed); a
      representative slice (`PointProfile3DEXT` + `ConeProfile3DEXT`, `AgeModifier3DEXT` +
      `LinearGravityModifier3DEXT`, `ColorInterpolator3DEXT` + `OpacityInterpolator3DEXT`)
      proves the new architecture reproduces today's exact baked-in
      `ParticleEmitter3DEXT::UpdateEXT` behavior before expanding further.
      `ConeDirectionEXT`/`ConeHalfAngleEXT` moved off `ParticleEmitter3DEXT` onto
      `ConeProfile3DEXT` (user-confirmed decision — a real, intentional field-ownership move,
      not a pure refactor; existing tests updated to configure the profile via
      `getProfileEXTProperty()`/`Profile3DEXT::Cone()` instead). `StartColorEXT`/
      `EndColorEXT`/`StartOpacityEXT`/`EndOpacityEXT`/`GravityEXT` deliberately stayed on
      `ParticleEmitter3DEXT` itself (zero pre-existing test changes needed for these) —
      `UpdateEXT` syncs them into the default `AgeModifier3DEXT`'s owned interpolators and the
      default `LinearGravityModifier3DEXT` every call via a private `SyncBuiltInModifiersEXT()`
      helper, so the new architecture genuinely drives the default per-frame pipeline
      end-to-end rather than being bypassed. `Modifier3DEXT::InternalUpdate` has one
      deliberate behavioral deviation from 2D's own `Modifier::InternalUpdate` (found and
      resolved during implementation, documented in full in `Modifier3DEXT.hpp`'s header
      comment): it passes the real `elapsedSeconds` to `Update()`, not a fixed `cycleTime_`,
      so a single large-timestep `UpdateEXT(dt, ...)` call still applies the full `dt`
      wherever it touches particles — 2D's own fixed-`cycleTime_` behavior would have silently
      broken `ParticleEmitter3DEXTTests.cpp`'s existing single-large-step test convention.
      `ParticleEmitter3DEXT`'s own built-in default modifiers additionally override
      `Frequency` to an effectively-unthrottled value (`1.0e6f`) so every particle is still
      touched every call, preserving the emitter's pre-existing behavior exactly.
      `InterpolatorOfT3DEXT<T>`'s protected constructor takes explicit `startValue`/
      `endValue` arguments (rather than 2D's in-class `T StartValue{};` default-member
      initializer) because `Microsoft::Xna::Framework::Color` (used by
      `ColorInterpolator3DEXT`) has no zero-argument constructor.
      `ColorInterpolator3DEXT` is not an HSL port — `Particle3DEXT::ColorEXT` is a plain RGB
      `Color`, and the pre-refactor baked logic already did a direct per-channel RGB lerp
      (alpha hardcoded to 255), reproduced exactly rather than introducing HSL interpolation.
      Regression check: pre-existing `ParticleEmitter3DEXTTests.cpp`/
      `ParticleRenderSystem3DEXTTests.cpp` (19 tests) kept passing with only the two
      `ConeDirectionEXT`/`ConeHalfAngleEXT`→profile call sites and one `ScaleEXT`
      `float`→`Vector2` assertion updated — proof the refactor is behavior-preserving.
      New tests: `Profile3DEXTTests.cpp` (12: `PointProfile3DEXT`, `ConeProfile3DEXT`
      including the A-06 zero-direction NaN-guard regression and a half-angle-containment
      check, `Profile3DEXT::Point()`/`Cone()` factories), `Modifier3DEXTTests.cpp` (16: base
      `Modifier3DEXT` frequency/name/enabled semantics and `InternalUpdate` pacing via a
      `RecordingModifier3DEXT` test double, `ModifierExecutionStrategy3DEXT` Serial strategy,
      `AgeModifier3DEXT`, `LinearGravityModifier3DEXT`), `Interpolator3DEXTTests.cpp` (11:
      base `Interpolator3DEXT` name/enabled semantics, `ColorInterpolator3DEXT`,
      `OpacityInterpolator3DEXT` including its A-06 clamp regression). 35 new tests total.
      Both configs verified via genuine `rm -rf` clean rebuild; full suite green
      (2275/2275).
- [x] **Phase C-2 — Remaining Profiles (6)**: `LineProfile3DEXT`, `RingProfile3DEXT`,
      `CircleProfile3DEXT`, `BoxProfile3DEXT`, `BoxFillProfile3DEXT`, `BoxUniformProfile3DEXT`
      (+ shared `LineRadiation3DEXT`/`CircleRadiation3DEXT` enums), plus a new shared
      `NextUnitVector3EXT(System::Random&, Vector3&)` free-function helper (3D counterpart of
      `RandomExtensions::NextUnitVector`) factored out once six call sites needed the same
      uniform-sphere-heading sampling; `PointProfile3DEXT`'s own pre-existing inline copy was
      refactored to call it too (same formula, no behavior change). Per the approved plan,
      `RingProfile3DEXT`/`CircleProfile3DEXT` are the sphere-surface/sphere-fill analogs of
      2D's circle-perimeter/filled-disk emission (not a literal ring/torus), and
      `BoxProfile3DEXT`/`BoxFillProfile3DEXT`/`BoxUniformProfile3DEXT` use a genuinely-3D
      6-face box (not a 2D rectangle embedded in 3D) — `BoxUniformProfile3DEXT` weights face
      selection by *surface area* (2D's `BoxUniformProfile` weights by perimeter *length*),
      using a continuous float draw rather than 2D's `Next(static_cast<int>(perimeter))`
      integer-truncation quirk, since this is brand-new code with no prior 3D behavior to
      preserve. `LineRadiation3DEXT` collapses 2D `LineRadiation`'s `PerpendicularUp`/
      `PerpendicularDown` pair into a single `Perpendicular` mode (a uniformly random
      direction within the plane perpendicular to `AxisEXT`) since a 3D line's perpendicular
      directions form an entire plane, not two fixed 2D screen-space choices. New code applies
      the A-06 zero-vector NaN guard proactively (`AxisEXT`/`DirectionEXT` in
      `LineProfile3DEXT`) even though upstream 2D `LineProfile` has no such guard, since this
      is fresh code, not a preservation of prior behavior. `Profile3DEXT` gained matching
      `Line`/`Ring`/`Box`/`BoxFill`/`BoxUniform`/`Circle` static factories.
      Tests: one new file per profile (`LineProfile3DEXT` 7, `RingProfile3DEXT` 4,
      `CircleProfile3DEXT` 3, `BoxProfile3DEXT` 3, `BoxFillProfile3DEXT` 2,
      `BoxUniformProfile3DEXT` 3 — including a larger-face-sampled-more-often proof
      distinguishing it from `BoxProfile3DEXT`'s equal-probability selection) plus 8 new
      factory-method cases added to `Profile3DEXTTests.cpp`. 30 new tests total. Both configs
      verified via genuine `rm -rf` clean rebuild; full suite green (2305/2305).
- [x] **Phase C-3 — Remaining Modifiers (8)**: `DragModifier3DEXT`, `OpacityFastFadeModifier3DEXT`,
      `RotationModifier3DEXT` (applies to `Particle3DEXT::RotationEXT`, the billboard roll
      angle — not a full 3D angular-velocity/orientation model), `VelocityColorModifier3DEXT`
      (RGB, not HSL — same `ColorInterpolator3DEXT` rationale), `VelocityModifier3DEXT` (owns
      `Interpolator3DEXT` instances, driven by speed instead of age), `VortexModifier3DEXT`,
      `SphereContainerModifier3DEXT`/`BoxContainerModifier3DEXT`/`BoxLoopContainerModifier3DEXT`
      (3D analogs of 2D's Circle/Rectangle/RectangleLoop containers — no shared
      `ContainerModifier` base exists in 2D either, so none was added here).
      `VortexModifier3DEXT` is NOT a literal port: 2D's vortex rotates a force around a single
      point (the 2D plane's one implicit rotation axis); the 3D version rotates around an
      explicit `AxisEXT` line via `PositionEXT` + `AxisEXT`, using Rodrigues' rotation formula
      restricted to a vector already perpendicular to the axis
      (`v*cos(theta) + (axis x v)*sin(theta)`) — a true generalization of 2D's 2x2 rotation
      matrix, not an unrelated new algorithm (see `VortexModifier3DEXT.hpp`'s header comment).
      `BoxContainerModifier3DEXT`/`BoxLoopContainerModifier3DEXT` preserve 2D
      `RectangleContainerModifier`'s exact documented inconsistent int-truncation quirk on
      X/Y (no attempt to "fix" it); the new Z axis has no 2D precedent, so it uses consistent
      (non-truncating for the reflecting variant, truncating-both-sides for the looping
      variant, matching each variant's own X/Y pattern) checks rather than inventing a new
      asymmetry. New code applies the A-06 zero-vector/near-zero-distance NaN guard
      proactively in `VortexModifier3DEXT` (zero `AxisEXT`, near-zero radial distance) and
      `SphereContainerModifier3DEXT` (a freshly-emitted zero-offset particle sitting exactly
      at its container's center) even though the 2D originals have the same latent
      div-by-zero unguarded, since this is fresh code with no prior 3D behavior to preserve.
      Tests: `DragModifier3DEXTTests.cpp` (2), `OpacityFastFadeModifier3DEXTTests.cpp` (2),
      `RotationModifier3DEXTTests.cpp` (2), `VelocityColorModifier3DEXTTests.cpp` (3),
      `VelocityModifier3DEXTTests.cpp` (2), `VortexModifier3DEXTTests.cpp` (5, including the
      along-axis-no-force and outside-outer-radius and max-velocity-clamp cases),
      `ContainerModifier3DEXTTests.cpp` (6, bundling all three containers, matching 2D's own
      single-file `ContainerModifierTests.cpp`). 22 new tests total. Both configs verified via
      genuine `rm -rf` clean rebuild; full suite green (2327/2327).
- [x] **Phase C-4 — Remaining Interpolators (4)**: `HueInterpolator3DEXT`,
      `RotationInterpolator3DEXT`, `ScaleInterpolator3DEXT`, `VelocityInterpolator3DEXT` —
      completing all 6 Interpolators, all 11 Modifiers, and all 8 Profiles from the approved
      plan; Phase 11's Particles plugin architecture is now at full parity with 2D's own
      shape. `HueInterpolator3DEXT` is not a literal port: 2D writes directly to
      `Data::Particle::Color[0]` since 2D's particle color is natively HSL; 3D converts the
      particle's *current* RGB `ColorEXT` to HSL via `CNA::Extended::HslColor::FromRgb`
      (reusing that existing utility rather than re-deriving conversion math — a small
      `World3DEXT` → `CNA::Extended` dependency, matching Phase 11 A's
      `UndefinedLayerException`-reuse precedent), overwrites only the H channel, and converts
      back with `HslColor::ToRgb` — preserving Saturation/Lightness exactly as 2D's own
      "hue-only" semantic intends. `ScaleInterpolator3DEXT` uses `Vector2` (meaningful now
      that `Particle3DEXT::ScaleEXT` is `Vector2` since Phase 11 C-1).
      `VelocityInterpolator3DEXT` deliberately deviates from a literal port: 2D's
      `VelocityInterpolator` uses `Vector2` and only ever touches Velocity's X/Y; per the
      "genuinely 3D, not a 2D vector embedded in 3D" principle established throughout Phase
      11 C-2/C-3, this interpolates the full `Vector3 VelocityEXT` (X/Y/Z).
      Tests: one new file per interpolator (`HueInterpolator3DEXT` 3 — including a
      hue-changes/saturation-lightness-preserved pair, `RotationInterpolator3DEXT` 2,
      `ScaleInterpolator3DEXT` 2, `VelocityInterpolator3DEXT` 2). 9 new tests total. Both
      configs verified via genuine `rm -rf` clean rebuild; full suite green (2336/2336).
      **Phase 11 (2D/3D parity: collision layers, recursive octree, particles plugin
      architecture) is now COMPLETE** — see this file's Phase 11 A/B/C-1/C-2/C-3/C-4 entries
      above for the full record.

## 5. After meaningful changes

- Check off completed tasks above; add newly discovered tasks under the right phase.
- Update `NEXT.md` (or a dedicated `NEXT3D.md`) with what changed and what's next.
- Keep both build configs green, exactly as required for the rest of this project.
