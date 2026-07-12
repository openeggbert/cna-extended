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
- [ ] Bounding volumes: `BoundingBox2D`, `BoundingCircle2D`, `BoundingCapsule2D`,
      `BoundingPolygon2D`, `OrientedBoundingBox2D`
- [ ] `Line2D`, `LineSegment2D`, `Ray2D`
- [ ] `Camera` + `OrthographicCamera`
- [ ] Color helpers: `ColorExtensions`, `ColorHelper`, `HslColor`
- [ ] `MathExtended`, `FloatHelper`, `Angle`
- [ ] `RectangleF`, `Rectangle.Extensions`, `RectangleF.Extensions`, `BoundingRectangle`,
      `OrientedRectangle`
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

## 7. Open items to resolve during implementation (not blocking plan approval)

- `Graphics/Effects` shader resources: whether CNA's graphics backend(s)
  (`SDL_RENDERER`/`EASYGL`/`BGFX`/`VULKAN`) can consume a ported `DefaultEffect` as-is or
  need a backend-specific re-authoring — investigate at the start of Phase 5 and raise
  with the user if it's a real design fork, not a mechanical port.
- Whether `Serialization/*` should wrap `sharp-runtime`'s `System::Text::Json` directly or
  needs its own converter layer — decide at the start of Phase 6.
