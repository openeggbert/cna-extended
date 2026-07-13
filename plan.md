# cna-extended — Porting Plan

Status: **APPROVED (2026-07-12 by Robert Vokáč) — Phase 0 complete, Phase 1 underway.
Fidelity requirement: port 1:1 wherever C#/C++ differences allow — no simplification.
See `NEXT.md` for session history.**

## 1. What this project is

`cna-extended` is a C++23 port of [MonoGame.Extended](https://github.com/craftworkgames/MonoGame.Extended)
(the `craftworkgames/MonoGame.Extended` repository), adapted to plug into
[`cna`](../cna) (a C++23 port of XNA 4.0 / FNA) and built on top of
[`sharp-runtime`](../sharp-runtime) (a C++23 reimplementation of the relevant .NET BCL
surface). It is a sibling repository to `cna`, `sharp-runtime`, and `easy-3d`, following
the same conventions.

Reference source (full clone, kept up to date, **read-only, never edited**):
`/rv/data/library/github.com/craftworkgames/MonoGame.Extended`
(update with `/rv/data/library/github.com/github.sh` — no-arg mode refreshes all clones).

## 2. Scope decisions (confirmed with the user on 2026-07-12)

### Ported

- **Math, Shapes & Collections foundation** — root-level math/bounding/interface types,
  `Math/`, `Collections/` (minus `ObservableCollection`, see below).
- **Input, Timers, Tweening, ViewportAdapters, VectorDraw** — small independent utility
  modules.
- **Collisions 2D** — `Collision2D`/`CollisionShape2D` root types + `Collisions/`
  (broadphase: `QuadTree`, `SpatialHash`; narrow phase: `CollisionWorld2D`, `ICollisionActor`).
- **Screens** — screen/scene stack manager.
- **Graphics & BitmapFonts** — `SpriteBatch` extensions, sprites/sprite sheets, texture
  atlases (including the direct-JSON TexturePacker reader), nine-patch, custom effects,
  BMFont bitmap fonts, and the `Animations/` frame-animation-controller module (paired
  with `Graphics` since it exists to drive sprite-sheet animation).
- **Particles** — the Mercury-Particle-Engine-derived particle system. (License note: all
  files in this module carry MonoGame.Extended's own MIT header — see `NOTICE.md`.)
- **ECS** — the Artemis-style Entity-Component-System (`World`/`Entity`/`Aspect`/`Systems`).
- **Tilemaps** — Tiled (TMX/JSON), LDtk, and Ogmo map formats and rendering, plus the
  `Serialization/` helpers they depend on.

### Explicitly excluded (non-goals)

- **`MonoGame.Extended.Content.Pipeline`** (the whole design-time MGCB assembly) — the
  user decided up front this will not be ported; it only matters at asset-build time
  inside a .NET/MSBuild toolchain and has no C++ runtime counterpart.
- **Runtime `.xnb` content readers**: `Content/ContentReaders/*`
  (`BitmapFontContentReader`, `Texture2DAtlasReader`, `ParticleEffectContentReader`,
  `JsonContentTypeReader`), the `Tilemaps/Content/*Reader` classes, `Content/BitmapFonts/`
  (the xnb-side helper, as opposed to the runtime `BitmapFonts/` module which **is**
  ported), and `Content/ExtendedContentManager.cs` / `Content/ContentReaderExtensions.cs`
  (glue code that only exists to register those xnb readers). Rationale: without the
  excluded Content Pipeline there is no way to produce the `.xnb` files these classes
  read, so porting them would be dead code. Direct-format loading (Tiled TMX/JSON, LDtk
  JSON, Ogmo JSON, TexturePacker JSON, BMFont `.fnt`) covers the same ground.
  `Content/ExternalResourceResolver(s).cs` (path resolution for external tileset/image
  references) is **not** xnb-specific and **is** ported, as part of the Tilemaps phase.
- **`ObservableCollection`/`IObservableCollection`** from MonoGame.Extended's own
  `Collections/` — `sharp-runtime` already has the real BCL type at
  `System::Collections::ObjectModel::ObservableCollection`. Use that instead; only add a
  thin adapter if a later phase finds a call site that specifically needs
  `IObservableCollection`'s exact interface shape.

## 3. License & attribution

- MonoGame.Extended is MIT-licensed (Copyright 2015–2024 Dylan Wilson, Lucas
  Girouard-Stranks, Christopher Whitley; all individual source files carry a
  "Copyright (c) Craftwork Games... Licensed under the MIT license" header). MIT permits
  porting/relicensing a derivative work under a new MIT copyright as long as the
  original notice is preserved somewhere — this is done via `NOTICE.md`, mirroring the
  pattern `cna` uses in `THIRD_PARTY_NOTICES.md` for its FNA attribution.
- `cna-extended` itself will be **MIT licensed**, copyright Robert Vokáč, matching
  `easy-3d`'s `LICENSE`.
- `NOTICE.md` (Phase 0) will carry: the full MonoGame.Extended MIT notice, a link back to
  the upstream repo, and a courtesy mention of MonoGame.Extended's own "Special Thanks"
  chain (Mercury Particle Engine, 2D XNA Primitives, LibGDX, Nez) since the Particles
  module traces its lineage there even though the code itself ships under
  MonoGame.Extended's own MIT header.
- Every ported source file gets an SPDX header (`// SPDX-License-Identifier: MIT`) plus a
  short "ported from MonoGame.Extended" note, matching `sharp-runtime`'s convention for
  its "ported from .NET runtime" headers.

## 4. Architecture & conventions

- **C++ standard**: C++23, matching `cna`/`sharp-runtime`/`easy-3d`.
- **Namespace**: `CNA::Extended::` (decided 2026-07-12), sub-namespaced per module, e.g.
  `CNA::Extended::Tilemaps`, `CNA::Extended::Particles`, `CNA::Extended::ECS`,
  `CNA::Extended::Graphics`. Rationale: `cna-extended` is a non-official addition on top
  of the XNA-compatible core, consistent with `cna`'s own `NOXNA`/`CNA::` convention for
  non-XNA additions — it does not claim to be an official MonoGame namespace.
- **File layout**: header/source split mirroring the namespace path, matching `cna` and
  `sharp-runtime`: `include/CNA/Extended/<Module>/<Type>.hpp` ↔
  `src/CNA/Extended/<Module>/<Type>.cpp`. Simple types may be header-only.
- **Properties**: `getXProperty()`/`setXProperty()`, matching `sharp-runtime`/`cna`.
- **Types**: use `sharp-runtime` type aliases (`SharpRuntime::intcs`, etc.) and BCL types
  (`System::Collections::Generic::List`, `System::EventHandler`, `std::ranges` — **not**
  `sharp-runtime`'s legacy `Linq.hpp`, per `sharp-runtime`'s own `CLAUDE.md` rule 8) in
  public APIs, same as `cna`.
- **CMake integration pattern**: mirror `easy-3d`'s sibling-dependency pattern (the
  cleanest precedent in this ecosystem):
  - `add_library(CNA_EXTENDED STATIC ...)`, alias `CNA::Extended` (or similar — finalize
    exact target/alias names in Phase 0).
  - If a `CNA` target already exists (parent build did `add_subdirectory(../cna)` first),
    just link against it.
  - Else, if `CNA_EXTENDED_LINK_CNA` is set, build `cna` from `../cna` via
    `add_subdirectory`.
  - Else, fall back to a headers-only include path (`CNA_EXTENDED_CNA_INCLUDE_DIR`) for
    compile-checking without a full link, same as `easy-3d`'s test/example pattern.
  - Same three-tier resolution for `sharp-runtime`.
- **Tests**: GoogleTest, mirroring `sharp-runtime`'s and `cna`'s test setup (submodule at
  `vendor/googletest`). Port the upstream test suite
  (`tests/MonoGame.Extended.Tests/**`) alongside each phase's implementation, translated
  to GoogleTest — do not skip tests to "come back later."
- **Docs**: Doxygen comment blocks on public types/methods, matching `cna`/`sharp-runtime`.
- **Reference variant**: MonoGame.Extended ships three parallel build variants
  (`MonoGame.Extended`, `FNA.Extended`, `KNI.Extended`) gated by `#if FNA`/`#if KNI`
  conditionals. Since `cna` mirrors FNA's API surface, prefer the `FNA` branch of any
  conditional as the primary reference; fall back to the default (MonoGame) branch
  elsewhere. Ignore KNI-only branches unless a specific need arises.
- **No `plan.sqlite3` for now** — the user explicitly asked for tasks to live in this
  markdown file. If the task list grows unwieldy as porting progresses (`sharp-runtime`
  needed a SQLite DB for its ~450-file scope), revisit that decision with the user rather
  than silently switching.

## 5. Phases & tasks

Ordered by dependency — later phases build on earlier ones. Check off tasks as they
land; update `NEXT.md` alongside every session's progress.

### Phase 0 — Repository scaffolding — ✅ complete (2026-07-12)

- [x] `LICENSE` (MIT, Robert Vokáč, matching `easy-3d`'s `LICENSE`)
- [x] `NOTICE.md` (MonoGame.Extended MIT attribution, see §3)
- [x] `README.md` (purpose, relationship to `cna`/`sharp-runtime`, scope summary, link to
      this plan)
- [x] Root `CMakeLists.txt` (see §4 CMake pattern) — options, target, alias, sibling
      dependency resolution for `cna` and `sharp-runtime`
- [x] `include/CNA/Extended/` and `src/CNA/Extended/` skeleton directories
- [x] `vendor/googletest` submodule + `tests/CMakeLists.txt`
- [x] `examples/minimal/` smoke-test example (mirroring `easy-3d`'s `examples/minimal`)
- [x] `.gitignore`, `Doxyfile`
- [x] Verify the whole scaffold builds green (empty lib, empty test binary) before Phase 1
      starts — see `NEXT.md` for the exact command/output.

### Phase 1 — Foundation: Math, Shapes, Interfaces & Collections

No dependency on CNA graphics — pure math/data types. Blocks almost every later phase.

- [x] Marker interfaces: `IMovable`, `IRotatable`, `IScalable`, `ISizable`,
      `IRectangular`, `IColorable`, `IEquatableByRef` (2026-07-12)
- [x] `TransformFlags` + `BaseTransform<TMatrix>` + `Transform3` (2026-07-12) — see §6 for
      the `MulticastAction` extension this required.
- [ ] `Transform2` (`BaseTransform<Matrix3x2>`) — **deferred until `Matrix3x2` is ported**
      (a few items below in this same phase); its `RecalculateLocalMatrix`/
      `RecalculateWorldMatrix` bodies need `Matrix3x2::CreateScale`/`CreateRotationZ`/
      `CreateTranslation`/`Multiply`/`Decompose`. Do this as the immediate follow-up to
      the `Matrix3x2` task, not standalone.
- [x] Bounding volumes: `BoundingBox2D`, `BoundingCircle2D`, `BoundingCapsule2D`,
      `BoundingPolygon2D`, `OrientedBoundingBox2D` (2026-07-12) — data/geometry API fully
      ported (fields, properties, factory methods, `Transform`/`Translate`/`Deconstruct`,
      `Equals`/`GetHashCode`/`ToString`/operators). Two deferrals, both documented in the
      relevant header's top comment, not silently dropped:
      - All 16 `Contains(...)`/`Intersects(...)`/`TryGetCollision(...)` overloads across
        the 5 types — every one delegates to `Collision2D` (Phase 2), not this phase.
      - `BoundingCapsule2D::CreateFromSegment(LineSegment2D, float)` and `CreateMerged`
        need `LineSegment2D::DistanceToPoint` — add both once `LineSegment2D` lands (next
        task in this phase).
      Known gap vs. this phase's "port tests faithfully" bar: upstream has ~130 test
      methods across these 5 types' test files, but ~20-27% per file exercise the
      deferred `Collision2D`-dependent methods. Rather than port the ~100 portable
      upstream tests now and revisit them again once `Collision2D` lands, wrote fresh
      (not upstream-1:1) tests covering everything that's ported (92 total project tests
      passing). **Follow-up task, do at the start of Phase 2**: port the full upstream
      `tests/MonoGame.Extended.Tests/Primitives`/`Shapes` suites for these 5 types 1:1,
      in one pass, once `Collision2D` makes the currently-deferred methods portable too.
- [x] `Line2D`, `LineSegment2D`, `Ray2D` (2026-07-12) — construction/factory methods,
      geometry (`DistanceToPoint`/`ClosestPoint`/`GetPoint`/`GetBounds`/`Normalize`/etc.),
      `Equals`/`GetHashCode`/`ToString`/operators/`Deconstruct` fully ported for all 3
      types. All `Intersects(...)` overloads across all 3 types, plus
      `LineSegment2D::DistanceSquaredToPoint`/`DistanceToPoint`/`DistanceSquaredToSegment`/
      `DistanceToSegment`, deferred until `Collision2D` (Phase 2) — documented in each
      header. **Correction to the `BoundingCapsule2D.hpp` deferral note** (written before
      `LineSegment2D` existed): porting `LineSegment2D` does **not** unblock
      `BoundingCapsule2D::CreateFromSegment`/`CreateMerged` after all —
      `LineSegment2D::DistanceSquaredToPoint`'s own body needs `Collision2D`, so those two
      `BoundingCapsule2D` members are still blocked on Phase 2, same as its
      `Contains`/`Intersects`/`TryGetCollision` overloads. `BoundingCapsule2D.hpp`'s
      comment has been corrected accordingly.
- [x] `Camera<T>` (2026-07-12) — fully abstract (all members pure virtual), header-only,
      ported in full. `getBoundingRectangleProperty()` returns `RectangleF`, forward-declared
      (not yet ported — a pure virtual declaration doesn't need a complete type). No upstream
      tests exist for this abstract type; a compile-only smoke test was added, with real
      instantiation-based tests deferred until `RectangleF` lands (a concrete override
      returning `RectangleF` by value needs the complete type).
- [ ] `OrthographicCamera` — **deferred, not just its `RectangleF` bits**: unlike the
      `Transform2`/`Matrix3x2` case, this is a deep structural dependency, not a narrow one.
      `OrthographicCamera` stores a `ViewportAdapter` as a required field, takes one as a
      required constructor parameter, and calls into it from multiple methods (not a couple
      of peripheral helpers) — `ViewportAdapters` is a whole separate module scheduled for
      **Phase 3** ("Input, Timers, Tweening, ViewportAdapters, VectorDraw"), not this phase.
      Port `OrthographicCamera` immediately after `ViewportAdapters` lands in Phase 3, not
      standalone.
- [x] Color helpers: `ColorExtensions`, `ColorHelper`, `HslColor` (2026-07-13) — ported
      directly (no fork needed, ~620 lines total). `ColorHelper`'s name→`Color` lookup table
      is built via reflection upstream (`typeof(Color).GetRuntimeProperties()`); C++ has no
      runtime reflection, so it's a hand-written (script-generated from CNA's `Color.hpp`,
      cross-checked at 141 entries) static table instead, with lower-cased keys for
      case-insensitive lookup (matches upstream's `StringComparer.OrdinalIgnoreCase`).
      `HslColor::ToRgb`'s `MathExtended.MachineEpsilon` reference resolved directly via
      `std::numeric_limits<float>::epsilon()` (the exact same value,
      `1.19209290e-7f`) rather than deferring on the not-yet-ported `MathExtended` — a
      trivial constant, not an algorithm, matching the established "duplicate the trivial"
      pattern. All upstream tests ported 1:1 this time (no `Collision2D` dependency to work
      around): `ColorExtensionsTests.cs`/`ColorHelperTests.cs`/`HslColorTests.cs` in full,
      via GoogleTest `TEST_P`/`INSTANTIATE_TEST_SUITE_P` for the `[Theory]`/`[InlineData]`
      cases. One real bug caught by the build: `ColorHelper::FromAbgr` couldn't use CNA's
      `Color(UInt32)` constructor (private in CNA, unlike upstream's public one) — reworked
      to decompose into R/G/B/A components and use the public 4-int constructor instead,
      same resulting color.
- [x] `MathExtended`, `FloatHelper`, `Angle` (2026-07-13) — ported directly (~315 lines).
      `Angle.cs` carries its own upstream attribution to the **SlimMath** project
      (Copyright (c) 2007-2010 SlimDX Group, MIT) in its file header — not just Craftwork
      Games; added a proper "code directly derived from other MIT-licensed projects"
      section to `NOTICE.md` (with SlimMath's full license text) rather than folding it
      into the existing inspiration-only "courtesy attribution" list. Fidelity note:
      `Angle::Equals`/`CompareTo` are intentionally **non-const**, taking `other` **by
      value** — upstream's versions aren't `readonly` and call `WrapPositive()` on both
      `this` and a by-value copy of `other`, so calling `Equals`/`CompareTo`/`==`/`!=` has
      the side effect of wrapping the angle into `[0, tau)`. `GetHashCode()` deliberately
      does **not** wrap first, unlike `Equals` — an apparent equality/hashing contract
      inconsistency in upstream itself, preserved as-is rather than fixed. All upstream
      tests ported 1:1 (`MathExtendedTests.cs`, `AngleTest.cs`). Also updated `HslColor.cpp`
      (from the previous task) to use the now-real `MathExtended::MachineEpsilon` instead
      of the `std::numeric_limits<float>::epsilon()` placeholder — same value, now the
      actual named constant matching upstream exactly.
- [x] `RectangleF`, `Rectangle.Extensions`, `RectangleF.Extensions`, `BoundingRectangle`
      (2026-07-13, via forked sub-agent) — ported (1313 lines: 4 header/source pairs +
      4 test files). Deferred, all documented in the relevant header, all genuinely blocked
      on not-yet-ported types (`SizeF`, `Matrix3x2`, `PrimitivesHelper`), not skipped for
      convenience:
      - `RectangleF`: `Size` property + `RectangleF(Vector2, SizeF)` ctor (`SizeF`); both
        `Transform(...)` overloads + `CreateFrom(points,...)`×2/`UpdateFromPoints` +
        `SquaredDistanceTo`/`DistanceTo`/`ClosestPointTo` (`Matrix3x2`/`PrimitivesHelper`).
      - `BoundingRectangle`: the `(Vector2, SizeF)` ctor and both implicit
        `BoundingRectangle(Rectangle|RectangleF)` conversions (all three route through the
        deferred `SizeF` ctor upstream); `Transform`/`CreateFrom(points,...)`/
        `UpdateFromPoints`/`SquaredDistanceTo`/`ClosestPointTo` (same reasons as
        `RectangleF`). Its *inverse* conversions (`BoundingRectangle`→`Rectangle`/
        `RectangleF`) needed no `SizeF` and **are** ported. Kept as a C++ aggregate
        (no user-declared constructor) rather than adding one upstream doesn't have, since
        C++20 parenthesized-aggregate-init covers both the default and 2-arg cases.
      - `OrientedRectangle` — **entirely deferred, no file created.** Its `Orientation`
        field is `Matrix3x2` itself (not just used in a method) — a deep structural
        dependency like `OrthographicCamera`→`ViewportAdapter`, not a narrow one. Port it
        immediately after `Matrix3x2` lands (task below), not standalone.
      Follow-ups landed in the same pass (verified unblocked, not assumed — see §6):
      `IRectangularF` in `IRectangular.hpp` (test added to `InterfaceTests.cpp`); `Camera<T>`
      in `Camera.hpp` (forward-declare replaced with a real `#include`, `CameraTests.cpp`
      got a full concrete `Camera<Vector2>` test double replacing the compile-only
      placeholder). `ISizable` is still blocked (needs `SizeF`, unrelated to this task).
      Test coverage: upstream test suite for this area is unusually thin/broken — the main
      `Primitives/RectangleFTests.cs` is entirely blocked (every test needs the deferred
      `SizeF` ctor or `Transform`), and `Primitives/BoundingRectangleTests.cs` is *entirely
      commented out in upstream itself* (nothing active to port). Ported what upstream
      actually has active and portable 1:1 (7 tests: `Math/RectangleFTests.cs` +
      `RectangleExtensionsTests.cs`), wrote ~30 fresh tests for everything else that's
      ported but untested/disabled upstream.
- [ ] `CircleF`, `EllipseF`, `Segment2`
- [ ] `Size`, `SizeF`, `Interval`, `Thickness`
- [ ] `Matrix3x2`, `MatrixExtensions`, `Vector2Extensions`
- [ ] `FastRandom`, `RandomExtensions`
- [ ] `PrimitivesHelper`, `ShapeExtensions`
- [ ] `Math/Triangulation/*` (polygon triangulation helpers)
- [ ] `GameTimeExtensions`, `GameComponentCollectionExtensions`
- [ ] `FramesPerSecondCounter` + `FramesPerSecondCounterComponent`
- [ ] `SimpleGameComponent` + `SimpleDrawableGameComponent`
- [ ] Collections: `Bag<T>`, `Deque<T>`
- [ ] Collections: `ObjectPool<T>`, `Pool<T>`, `IPoolable`, `ItemEventArgs`
- [ ] Collections: `KeyedCollection`, `DictionaryExtensions`, `ListExtensions`
- [ ] Port `tests/MonoGame.Extended.Tests/{Math,Primitives,Shapes,Collections}` as
      GoogleTest suites

### Phase 2 — Collisions 2D

Depends on Phase 1 (math/shapes).

- [ ] Root types: `Collision2D`, `CollisionResult2D`, `CollisionShape2D`,
      `CollisionShapeKind2D`
- [ ] `CollisionWorld2D`, `ICollisionActor`, `ICollisionBroadphase2D`,
      `CollisionEvent2D`, `CollisionPair2D`, `ActorPairKey`
- [ ] Broadphase: `QuadTree/*`, `SpatialHash`
- [ ] `Layers/*`, `LayerPair`
- [ ] Port `tests/MonoGame.Extended.Tests/Collisions/**`

### Phase 3 — Input, Timers, Tweening, ViewportAdapters, VectorDraw

Independent of each other; depends only on Phase 1 and CNA's `Microsoft::Xna::Framework::Input`.

- [ ] `Input/` listeners (keyboard/mouse/gamepad/touch) and their event-args/settings types
- [ ] `Timers/*`
- [ ] `Tweening/*` (easing functions + `Tweener`)
- [ ] `ViewportAdapters/*` (`BoxingViewportAdapter`, `ScalingViewportAdapter`,
      `DefaultViewportAdapter`, etc.)
- [ ] `VectorDraw/*` (debug line/shape drawing)
- [ ] Port `tests/MonoGame.Extended.Tests/{ViewportAdapters,Tweening}`

### Phase 4 — Screens

Depends on Phase 1; needs CNA's `GameComponent`/`Game`.

- [ ] `Screens/*` (`Screen`, `ScreenManager`, transitions, etc.)
- [ ] Port `tests/MonoGame.Extended.Tests/Screens`

### Phase 5 — Graphics, BitmapFonts & Animations

Depends on Phase 1 and CNA's `GraphicsDevice`/`SpriteBatch`/`Effect`/`Texture2D`.

- [ ] `Graphics/Effects/*` (custom `Effect` wrapper — investigate how the embedded
      `DefaultEffect.fx`/`.mgfxo`/`.dx11.mgfxo`/`.ogl.mgfxo` resources map onto CNA's
      graphics backend(s); this may need re-authoring rather than a literal port —
      flag as a design question if CNA's shader pipeline doesn't have an equivalent)
- [ ] `Sprite`, `AnimatedSprite`, `SpriteSheet`, `SpriteSheetAnimation`,
      `SpriteSheetAnimationBuilder`, `SpriteSheetAnimationFrame`
- [ ] `Texture2DAtlas`, `Texture2DRegion`, `Texture2DRegion.Extensions`
- [ ] `NinePatch`
- [ ] `SpriteBatch.Extensions`, `GraphicsDevice.Extensions`, `RenderTarget2DExtensions`,
      `PrimitiveTypeExtensions`, `FlipFlags`, `IMatrixChainEffect`
- [ ] `Content/TexturePacker/*` (direct-JSON TexturePacker atlas format — not xnb-based)
- [ ] `Content/ExternalResourceResolver(s)` (not xnb-specific)
- [ ] `BitmapFonts/*` (runtime `BitmapFont`/`BitmapFontRegion` types — **not** the xnb
      `Content/BitmapFonts/` helper)
- [ ] `Animations/*` (`AnimationController`, `AnimationEvent`, `AnimationEventTrigger`,
      `IAnimationController`, `IAnimation`, `IAnimationFrame`) + root `AnimationComponent`
- [ ] Port `tests/MonoGame.Extended.Tests/{Graphics,BitmapFonts,Animations}`

### Phase 6 — Serialization

Depends on Phase 1. Feeds Phase 7 (Tilemaps).

- [ ] `Serialization/*` (JSON converters — check `sharp-runtime`'s
      `System::Text::Json` coverage first and reuse it rather than hand-rolling a
      parallel JSON layer)
- [ ] `Serialization/Xml/*`
- [ ] Port `tests/MonoGame.Extended.Tests/Serialization/**`

### Phase 7 — Tilemaps (Tiled / LDtk / Ogmo)

The largest module (121 + 22 files). Depends on Phases 1, 5 (rendering), 6 (serialization).

- [ ] Core: `Tilemap`, `TilemapData`, `TilemapFactory`, `TilemapLayerCollection`,
      `TilemapOrientation`, `TilemapTile*`, `TilemapTileset*`, `TilemapWorld`
- [ ] `TilemapLayers/*`, `TilemapObjects/*`, `Rendering/*`, `Properties/*`
- [ ] `Tiled/*` (TMX/JSON parser — `TiledTmxParser` and friends) — priority given the
      user's existing `tiled-blupi` project
- [ ] `LDtk/*` (LDtk JSON document model + integration)
- [ ] `Ogmo/*` (Ogmo Editor JSON document model + integration)
- [ ] `Parsers/*`
- [ ] Explicitly **skip**: `Tilemaps/Content/*Reader` (xnb-based, see §2 exclusions)
- [ ] Port `tests/MonoGame.Extended.Tests/Tilemaps/**`

### Phase 8 — Particles

Depends on Phases 1 and 5 (rendering).

- [ ] Core: `ParticleEffect`, `ParticleEmitter`, `ParticleBuffer`, `ParticleIterator`,
      `ParticleRenderingOrder`
- [ ] Remaining `Particles/**` (profiles, modifiers, primitives — enumerate exact file
      list at implementation time; 47 files total)
- [ ] Explicitly **skip**: `Content/ContentReaders/ParticleEffectContentReader.cs`
      (xnb-based)
- [ ] Port `tests/MonoGame.Extended.Tests/Particles/**`

### Phase 9 — ECS

Depends on Phase 1 (`Bag<T>` from Collections is used internally by Artemis-style ECS
implementations — confirm and reuse rather than re-rolling).

- [ ] `World`, `WorldBuilder`, `Entity`, `EntityManager`
- [ ] `Aspect`, `AspectBuilder`, `ComponentType`, `ComponentBits`, `ComponentManager`,
      `ComponentMapper`, `BitArrayExtensions`
- [ ] `EntitySubscription`
- [ ] `Systems/*` (`ISystem`, `EntitySystem`, `UpdateSystem`, `DrawSystem`,
      `EntityUpdateSystem`, `EntityDrawSystem`, `EntityProcessingSystem`)
- [ ] Port `tests/MonoGame.Extended.Tests/ECS/**`

### Phase 10 — Integration, polish, documentation

- [ ] End-to-end example combining several modules (e.g. a small Tiled map + sprite
      animation + input demo) under `examples/`
- [ ] Full Doxygen pass; generate `docs/generated/html` (git-ignored, matching
      `sharp-runtime`)
- [ ] Zero-warning pass (`-Wall -Wextra -Werror` / `/W4 /WX`, matching house convention)
- [ ] Update `README.md` with final module list and usage snippet
- [ ] Re-audit `NOTICE.md` against the final set of ported files

## 6. Decisions log

- 2026-07-12 — Confirmed MonoGame.Extended license (MIT) permits this port; `cna-extended`
  will be MIT with `NOTICE.md` attribution.
- 2026-07-12 — Reference clone relocated from the user's suggested `/rv/tmp` to the
  established `/rv/data/library/github.com/craftworkgames/MonoGame.Extended` convention
  (mirrors how `cna`'s `CLAUDE.md` references `/rv/data/library/github.com/FNA-XNA/FNA`).
- 2026-07-12 — Full module scope confirmed via user Q&A (see §2). Everything proposed was
  approved except the already-excluded Content Pipeline / xnb readers.
- 2026-07-12 — Namespace: `CNA::Extended::` (not `MonoGame::Extended::` or flat
  `CnaExtended::`).
- 2026-07-12 — XNB-based `ContentTypeReader` classes excluded (dead code without the
  excluded Content Pipeline).
- 2026-07-12 — User approved `plan.md`; confirmed strict fidelity (1:1 port as far as
  C#/C++ differences allow, no simplification) and per-task git commits during the
  unattended porting session.
- 2026-07-12 — Phase 0 CMake target name: `CNA_EXTENDED` (matches `CNA`/`SHARP_RUNTIME`
  uppercase convention), alias `CNA::Extended` (matches the C++ namespace, gives a clean
  namespaced import target). Root `CMakeLists.txt` written using easy-3d's three-tier
  `TARGET CNA` / opt-in build / headers-only pattern for both `cna` and `sharp-runtime`.
- 2026-07-12 — Standard local build/verify command for this project is
  `cmake -S . -B build -DCNA_EXTENDED_LINK_CNA=ON` (then build + `ctest`). Discovered while
  testing the marker interfaces: CNA's XNA types (`Vector2`, `Color`, `Rectangle`, ...) are
  declared in headers but only *defined* in CNA's compiled `.cpp` files, so a test that
  constructs one needs CNA actually linked to run, not just compiled. `tests/CMakeLists.txt`
  now follows easy-3d's `CNA_EXTENDED_CNA_LINKED`-gated pattern (real gtest executable when
  linked, `OBJECT`-library compile-check otherwise) for the whole suite.
- 2026-07-12 — Extended `sharp-runtime`'s `System::MulticastAction<Args...>` with a
  token-based `Add()`/`Remove()` (identity-based unsubscription), approved by the user
  (this is normally disallowed by `CLAUDE.md`'s "don't modify sibling repos" rule without
  explicit permission). Needed because `Transform.cs`'s `BaseTransform<TMatrix>`
  resubscribes to every ancestor's `TransformBecameDirty` event on reparenting, unsubscribing
  the old chain by identity — something C# delegate equality supports natively but
  `std::function` does not. `operator+=` is unchanged (purely additive change); all 11562
  sharp-runtime tests plus 12 new `MulticastAction` tests pass. Expect this same need to
  recur elsewhere in the port (any C# `event Action` with a `-=`) — reach for
  `Add`/`Remove` first before inventing another local workaround.
- 2026-07-12 — Bounding volumes ported via a forked sub-agent (to keep ~3200 lines of C#
  source-reading out of the orchestrating session's context), then reviewed and corrected
  by the orchestrating session before commit: the fork's port had silently dropped
  `GetHashCode()` from all 5 types (a real fidelity gap — C#'s equality-override
  convention pairs `Equals`/`GetHashCode`). Fixed by adding it to all 5 types, following
  `sharp-runtime`'s `ArraySegment<T>::GetHashCode()` convention (`int GetHashCode() const`,
  XOR-combining member hashes) for 4 of them, and `sharp-runtime`'s `System::HashCode` for
  `BoundingPolygon2D` (matches upstream's `HashCode.Add`/`ToHashCode()` loop over
  vertices) — note `System::HashCode::Add<T>` needs `std::hash<T>`, which doesn't exist
  for `Vector2`, so it's fed `vertex.GetHashCode()` (an `int`) rather than the `Vector2`
  itself. Lesson for future forked porting tasks: explicitly review for dropped
  `Equals`/`GetHashCode`/operator-overload triads, not just "does it build and pass its
  own tests" — a fork's self-reported build/test success does not by itself guarantee
  nothing was silently omitted.
- 2026-07-12 — `Line2D`/`LineSegment2D`/`Ray2D` ported via a forked sub-agent, this time
  explicitly instructed to self-check the `Equals`/`GetHashCode`/`ToString`/operators/
  `Deconstruct` checklist per type before reporting back (the lesson from the bounding
  volumes task) — independently re-verified via `grep -n "public "` against the upstream
  `.cs` files regardless, and the self-check held up. The fork also caught and reported a
  real mistake in `BoundingCapsule2D.hpp`'s existing deferral comment (see the Phase 1
  checklist entry above) rather than silently working around it or ignoring it — a good
  outcome from asking forks to state deferral *reasons*, not just deferral *lists*.
- 2026-07-13 — `RectangleF`/`Rectangle.Extensions`/`RectangleF.Extensions`/
  `BoundingRectangle` ported via a forked sub-agent (`OrientedRectangle` fully deferred —
  needs `Matrix3x2`, see the Phase 1 checklist). Explicitly asked the fork to check
  whether landing `RectangleF` actually unblocked `IRectangularF` (`IRectangular.hpp`) and
  `Camera<T>`'s test — it verified both were genuinely unblocked (not assumed) and landed
  both follow-ups in the same pass, updating `Camera.hpp`'s forward-declare to a real
  `#include`. Independent re-verification via `grep -n "public "` against `RectangleF.cs`
  confirmed the `Equals`/`GetHashCode`/`ToString`/operators checklist held up again.

## 7. Open items to resolve during implementation (not blocking plan approval)

- `Graphics/Effects` shader resources: whether CNA's graphics backend(s)
  (`SDL_RENDERER`/`EASYGL`/`BGFX`/`VULKAN`) can consume a ported `DefaultEffect` as-is or
  need a backend-specific re-authoring — investigate at the start of Phase 5 and raise
  with the user if it's a real design fork, not a mechanical port.
- Whether `Serialization/*` should wrap `sharp-runtime`'s `System::Text::Json` directly or
  needs its own converter layer — decide at the start of Phase 6.
