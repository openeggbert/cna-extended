# `cna-extended` — World3DEXT Porting/Design Plan

Status: **All 9 original phases complete (2026-07-14); Phase 10 (audit follow-ups, added
2026-07-14) in progress.** Approved 2026-07-14 by Robert Vokáč, including the Phase 5-8
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
version" precedent (`OctreeEXT`, `CollisionWorld3DEXT`).

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
      shortcut (e.g. querying the grid directly instead of via `OctreeEXT`) was not called
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
- [ ] **A-06 (Medium)** — confirmed by re-reading the code: (1)
      `ParticleEmitter3DEXT::SampleConeDirectionEXT` calls `Vector3::Normalize
      (ConeDirectionEXT)` with no zero-vector guard (undefined/NaN result if a caller
      leaves or sets it to `Vector3::Zero`); (2) `ParticleRenderSystem3DEXT` casts
      `particle.OpacityEXT * 255.0f` straight to `std::uint8_t` with no clamping --
      out-of-[0,1] opacity is real UB in C++, not just wraparound. Fix: default/guard a
      zero `ConeDirectionEXT` in `EmitEXT`/`SampleConeDirectionEXT`; clamp opacity to
      [0,1] before the byte conversion (in `ParticleEmitter3DEXT::UpdateEXT`, where
      `OpacityEXT` is actually computed, not at the render call site). Add tests for both.
- [ ] **A-08 (Medium)** — `ModelComponentEXT`/`SkinnedModelComponentEXT::BoundsEXT`
      default to a zero-radius `BoundingSphere` and are easy to leave unset, silently
      culling a real model every frame with no warning. Add a helper (e.g.
      `ComputeModelBoundsEXT(const Model&)`) that derives a real bounding sphere from a
      `Model`'s mesh bounds (`ModelMesh::getBoundingSphereProperty()`, merged via
      `BoundingSphere::CreateMerged`), so callers have a real alternative to hand-rolling
      one. Documenting the "must set this yourself" contract more prominently in the
      component's own header comment is the minimum bar if the helper turns out to need a
      Phase 11 of its own.
- [ ] **A-09 (Medium)** — status docs are stale/contradictory, confirmed directly:
      `3d.md`'s own top status line still says "still needs explicit phase-by-phase
      approval before any code is written" despite `plan3d.md` recording that approval and
      all 9 phases as complete; `README.md` says "2042/2042 tests passing" (predates even
      `plan.md`'s own later phases, let alone `World3DEXT` — current count is 2157/2159).
      Fix: update `3d.md`'s status line to match `plan3d.md`'s actual "DONE" status;
      update `README.md`'s test count and re-scan `NEXT.md` for any other stale figures
      left over from mid-session edits this same day.
- [ ] **A-07 (Medium)** — most `*EXT` render tests draw exactly one isolated object, so
      the missing-depth-buffer class of bug this session already found and fixed in
      `world3d_demo` (see `NEXT.md` section 5) has no regression test protecting it in the
      actual test suite. Add a depth-enabled multi-object test (two overlapping cubes
      drawn in both submission orders, asserting the *nearer* one's color wins either way)
      to lock in that fix.
- [ ] **A-05 (Medium, deferred)** — `OctreeEXT` is honestly documented as a fixed-cell
      spatial hash, not a hierarchical octree (`OctreeEXT.hpp`'s own header comment
      already says so), but the audit is right that the class *name* still invites the
      opposite assumption, and `Query`'s candidate dedup is `std::find`-in-a-loop (linear).
      Left for a future session: renaming is a real breaking API change (touches
      `CollisionWorld3DEXT`'s default construction and every test/example referencing
      `OctreeEXT` by name) and the dedup performance concern needs a real large-scene
      benchmark before "optimize" is well-defined, not a guess — recorded here rather than
      done reflexively.
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
- [x] **A-10 (Low, already tracked)** — voxel tilemap rendering's lack of chunk
      meshing/hidden-face culling/batching is already recorded as a deliberate,
      documented scope decision (`plan3d.md`'s own Phase 8 entry, `NEXT.md` section 8's
      deferred-scope list) — the audit independently arriving at the same conclusion is a
      useful confirmation, not a new finding needing its own fix task.

## 5. After meaningful changes

- Check off completed tasks above; add newly discovered tasks under the right phase.
- Update `NEXT.md` (or a dedicated `NEXT3D.md`) with what changed and what's next.
- Keep both build configs green, exactly as required for the rest of this project.
