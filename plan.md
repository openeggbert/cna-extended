# cna-extended — Porting Plan

Status: **APPROVED (2026-07-12 by Robert Vokáč) — Phase 0 and Phase 1 complete (2026-07-13),
Phase 2 ("Collisions 2D") next. Fidelity requirement: port 1:1 wherever C#/C++ differences
allow — no simplification. See `NEXT.md` for session history.**

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
      - `OrientedRectangle` — **entirely deferred, no file created** at the time this task
        landed. Its `Orientation` field is `Matrix3x2` itself (not just used in a method) —
        a deep structural dependency like `OrthographicCamera`→`ViewportAdapter`, not a
        narrow one. **Update (2026-07-13, follow-up task after `Matrix3x2` landed): now
        ported**, minus what's still transitively blocked — see the dedicated entry below.
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
- [x] `CircleF`, `EllipseF`, `Segment2` (2026-07-13, via forked sub-agent) — ~860 lines
      across 9 files. `EllipseF` fully self-contained, zero deferrals. `CircleF`: 4
      `Intersects(CircleF, BoundingRectangle)` overloads deferred (call
      `BoundingRectangle::SquaredDistanceTo`, itself already blocked on `PrimitivesHelper`
      — inherited blocker, not new). `Segment2`: 2 `Intersects(RectangleF|
      BoundingRectangle, out Vector2)` overloads deferred (`PrimitivesHelper.
      IntersectsSlab`, not ported). No whole-type deferral needed this time (unlike
      `OrientedRectangle`).
      **Genuine upstream bug found and faithfully preserved**: `Segment2.SquaredDistanceTo`
      (`Segment2.cs:96`) has a bare `endToPoint.Dot(endToPoint);` expression statement
      missing its `return` — for a point that projects beyond the segment's `End`, this
      silently falls through to the perpendicular-distance formula instead of returning the
      correct end-point distance, producing a wrong result (confirmed: e.g. 400 instead of
      the correct 800 for one straightforward case). The C++ port reproduces this exactly,
      per the standing no-simplification mandate; documented prominently in `Segment2.hpp`'s
      header comment and covered by a regression test asserting the actual (bug-preserving)
      value with a comment showing what the "correct" value would be. **Do not silently fix
      this if it's ever noticed again** — it's a deliberate fidelity choice, not an oversight.
      `CircleF.cs`/`Segment2.cs` cite "Real-Time Collision Detection, Christer Ericson,
      2005" as an algorithm source in comments — an academic citation, not a third-party
      code/license dependency (unlike `Angle.cs`'s SlimMath case); no `NOTICE.md` change
      needed. Test coverage: `EllipseFTest.cs` fully active, ported 1:1 (13 cases);
      `CircleFTests.cs` has exactly 1 active upstream test (rest commented out) — ported
      that one plus ~15 fresh; `Segment2DTests.cs` is entirely commented out upstream (0
      portable) — wrote 11 fresh tests, including the bug-regression test above.
- [x] `Size`, `SizeF`, `Interval`, `Thickness` (2026-07-13, via forked sub-agent) — ~1450
      lines across 16 files, fully ported with **no deferrals** — nothing in these 4 files
      depends on unported types. `Interval<T>` is header-only (a template); upstream's
      `where T : IComparable<T>` constraint has no C++ analogue for primitives, translated
      to ordinary `<`/`==` comparisons (the natural C++ equivalent). `Thickness` uses
      getX/setXProperty() accessors (upstream `{get;set;}`) while `Size`/`SizeF` use raw
      public fields (matching upstream's own field-vs-property split per type).
      **All three SizeF-blocked follow-ups verified unblocked and landed in the same
      pass**: `ISizable.hpp` (forward-declare → real include + round-trip test),
      `RectangleF` (added `Size` property + `RectangleF(Vector2, SizeF)` ctor),
      `BoundingRectangle` (added `(Vector2, SizeF)` ctor + both implicit conversions from
      `Rectangle`/`RectangleF`). `RectangleF`'s and `BoundingRectangle`'s *other*
      deferrals (`Transform`, `CreateFrom(points)`, `SquaredDistanceTo`) remain blocked on
      `Matrix3x2`/`PrimitivesHelper` — unrelated to this task, not touched.
      No upstream bugs found this time (unlike the previous task's `Segment2` case).
      Test coverage: `Math/IntervalTests.cs` fully active, ported 1:1 (34 tests, no
      `Collision2D` dependency this time); `Primitives/Size2Tests.cs` entirely commented
      out upstream (0 portable, wrote fresh); no upstream `ThicknessTests.cs` exists at
      all (wrote fresh).
- [x] `Matrix3x2`, `MatrixExtensions`, `Vector2Extensions` (2026-07-13, via forked
      sub-agent) — ~1300 lines across 9 files. `Matrix3x2` fully self-contained, no
      deferrals. **Bonus follow-up landed in the same pass: `Transform2` fully
      implemented** in `Transform.hpp`/`.cpp` (deferred since task 2 of this phase,
      pending exactly this dependency) — both constructors, `IMovable`/`IRotatable`/
      `IScalable` implementation (Transform2 implements these upstream, Transform3 does
      not — asymmetry preserved faithfully), `RecalculateLocalMatrix`/
      `RecalculateWorldMatrix`, `ToString`; 8 new tests in `TransformTests.cpp`.
      Other follow-up spots checked precisely (not guessed):
      - `RectangleF`/`BoundingRectangle`'s `Transform(...)` overloads: confirmed **still
        blocked** on `PrimitivesHelper.TransformRectangle`, Matrix3x2 alone wasn't enough.
      - `CircleF`/`Segment2`'s existing deferrals: confirmed `PrimitivesHelper`-only,
        unaffected by Matrix3x2 landing.
      - `OrientedRectangle`: at the time this task landed, still not ported, but its
        blocker had changed — its constructor and `Orientation` field only needed
        `SizeF`+`Matrix3x2`, both now available; only its `Transform` method needed the
        still-missing `PrimitivesHelper.TransformOrientedRectangle`. **Update
        (2026-07-13, picked up immediately as its own small follow-up task): now ported**
        — see the dedicated checklist entry below for exactly what landed vs. what's
        still deferred.
      **Bug found — this session's own, not upstream's**: `Matrix3x2::Identity` was
      initially built from `Vector2::UnitX`/`UnitY`/`Zero` (CNA statics in a different
      translation unit) — a static-initialization-order fiasco that silently zeroed
      `Identity` at runtime, caught by 2 failing tests. Fixed with literal float values
      instead. Checked the other 4 similar `static const X::Empty`-style members already
      in the tree (RectangleF, BoundingRectangle, SizeF, Size) — all safe, none has this
      cross-TU pattern. **Watch for this specific pattern in any future
      `static const Type X = Type(other statics...)` declaration.**
      Test coverage: `Math/Matrix3x2.cs`'s 2 tests + `Vector2ExtensionsTests.cs`'s 10
      tests ported 1:1; `MatrixExtensions` has no upstream test file; wrote ~19 fresh
      tests for the rest of `Matrix3x2`'s large, mostly-untested-upstream surface.
- [x] `OrientedRectangle` (2026-07-13, follow-up task picked up immediately after
      `Matrix3x2` landed, ported directly without a fork) — fields, constructor, `Points`,
      `Position` (get; set throws `std::logic_error`, matching upstream's
      `NotImplementedException`), `Equals`/`GetHashCode`/`ToString`/operators, the
      `OrientedRectangle(RectangleF)` converting constructor, and the self-contained SAT
      `Intersects(OrientedRectangle, OrientedRectangle)` (returns a named
      `OrientedRectangleIntersection{bool Intersects, Vector2
      MinimumTranslationVector}` struct — C++'s closest faithful equivalent to upstream's
      named-tuple return type). **Correction found while scoping this task**: the
      previous task's fork report said only `Transform` was blocked, but `BoundingRectangle`
      (the property) and `explicit operator RectangleF(OrientedRectangle)` are **also**
      transitively blocked — both call `RectangleF::Transform` internally, which is
      itself still blocked on `PrimitivesHelper`. All three (the static `Transform`
      method, the `BoundingRectangle` property, and the `RectangleF` conversion operator)
      are deferred together, documented in the header. Test coverage: upstream's
      `Initializes_oriented_rectangle`/`Equals_comparison` tests ported 1:1; the entire
      nested `Transform` test class (11 tests) is inapplicable (all test the deferred
      method); wrote fresh tests for `Position`/the `RectangleF` conversion/`Intersects`
      (no upstream coverage for that SAT algorithm specifically).
- [x] `FastRandom`, `RandomExtensions` (2026-07-13, ported directly, no fork) — ~410 lines
      across 6 files, fully ported, no deferrals. `FastRandom`'s private nested
      `IFastRandomImpl`/`LinearCongruentialGeneratorImpl`/`ThreadSafeFastRandomImpl`
      (a bridge/strategy pattern) ported 1:1 via `std::unique_ptr`-owned nested types.
      C#'s `[ThreadStatic]` → `thread_local`. `Shared`'s lazy-initialized static property
      → a function-local static (deliberately, to avoid a repeat of this session's
      `Matrix3x2::Identity` cross-TU static-init-order bug). `RandomExtensions`'s free
      functions port each algorithm exactly as upstream wrote it (e.g. `NextSingle` as a
      literal `(float)NextDouble()` cast) **rather than delegating to `sharp-runtime`'s
      own, more modern `System::Random::NextSingle()`** — that method already exists and
      is used elsewhere in the codebase, but implements a different algorithm (matching
      real .NET 6+ `Random.NextSingle()`); using it here would silently change behavior
      from what this specific upstream file actually does. No upstream tests exist for
      either file — wrote fresh tests covering deterministic seeding, range bounds for
      every overload, and the `Shared` singleton property.
- [x] `PrimitivesHelper` (2026-07-13, ported directly, no fork) — fully ported, no
      deferrals (~145 lines). Fully self-contained (only needs `Vector2`, `Matrix3x2`,
      `MathExtended`, all already ported) — this was the single most-referenced blocker
      across Phase 1's deferrals; see the follow-up sweep below.
      **`ShapeExtensions` deferred in full, scope reassessed**: despite living in the
      `Math/` folder upstream, `ShapeExtensions.cs` is entirely `SpriteBatch` debug-drawing
      code (`DrawPolygon`/`DrawLine`/`DrawCircle`/etc., using a live `GraphicsDevice` and a
      1x1 white-pixel `Texture2D`) — a Graphics-phase concern (Phase 5, "Graphics &
      BitmapFonts"), not a Phase 1 math-utility concern, regardless of its upstream folder
      location. Needs a real graphics context to test meaningfully (matches this project's
      `CNA_EXTENDED_CNA_LINKED` test-gating pattern for anything touching
      `GraphicsDevice`/`SpriteBatch`). Port it as part of Phase 5, not standalone here.
      **Newly discovered while scoping this task**: `ShapeExtensions.cs` depends on
      `MonoGame.Extended.Shapes.Polygon`/`Polyline` (`source/MonoGame.Extended/Shapes/`),
      an entire upstream folder that was **never tracked anywhere in this plan** — a real
      gap, not an intentional Phase-1 exclusion. Checked: both types turned out small
      (178+41 lines) and fully self-contained (only `Vector2`/`RectangleF`, both already
      ported), so **ported them here** rather than leaving the gap open — `Polygon`,
      `Polyline` (`include/src/tests/CNA/Extended/Shapes/`). First use of a C++
      sub-namespace in this phase: `CNA::Extended::Shapes`, mirroring upstream's own
      `MonoGame.Extended.Shapes` C# sub-namespace (unlike the `Math/` folder types ported
      so far, which all stayed in the C# root namespace despite their subfolder). 4 of
      upstream's `PolygonTests.cs` tests ported 1:1; no upstream tests exist for
      `Polyline` or `PrimitivesHelper` — wrote fresh tests for both.
      **Follow-up sweep after `PrimitivesHelper` landed** (verified by reading each header's
      deferral comment and the actual upstream `.cs` source, not assumed — see §6 for the
      full account): `Line2D`'s deferred `Intersects(Ray2D/LineSegment2D/...)` overloads and
      `LineSegment2D`'s `DistanceSquaredToPoint`/`DistanceSquaredToSegment` remain blocked on
      `Collision2D` specifically (a different, larger Phase-2 type, not `PrimitivesHelper`)
      — **not** unblocked by this task despite both citing similar "Real-Time Collision
      Detection" algorithm sources; confirmed still blocked, left deferred. The following
      **were** genuinely unblocked and landed in this same task/commit:
      - `RectangleF`: `Transform` (2 overloads), `CreateFrom(points, ...)` (2 overloads),
        `UpdateFromPoints`, `SquaredDistanceTo`, `DistanceTo`, `ClosestPointTo`. Tests added:
        the 2 constructor tests + 5 `Transform` tests from upstream's
        `tests/MonoGame.Extended.Tests/Primitives/RectangleFTests.cs` (ported 1:1; that
        file's `Rectangle_Intersects_Test` was intentionally **not** ported — it exercises
        base XNA/FNA `Rectangle.Intersects`, not anything MonoGame.Extended adds), plus fresh
        tests for `CreateFrom(points)`/`UpdateFromPoints`/`SquaredDistanceTo`/`DistanceTo`/
        `ClosestPointTo` (no upstream coverage for those specifically).
      - `BoundingRectangle`: `CreateFrom(points, ...)` (2 overloads), `Transform` (2
        overloads, matching upstream's ref-mutates-input-in-place semantics faithfully),
        `UpdateFromPoints`, `SquaredDistanceTo`, `ClosestPointTo`. Fresh tests added (the
        upstream `Primitives/BoundingRectangleTests.cs` is entirely commented out, as noted
        by the pre-existing test file's header comment).
      - `OrientedRectangle`: `getBoundingRectangleProperty()` (upstream: `(RectangleF)this`),
        `static Transform(OrientedRectangle, ref Matrix3x2)`, and
        `explicit operator RectangleF(OrientedRectangle)`. Upstream's `private static`
        ref-taking `Transform` overload isn't part of its public API contract, so its logic
        was inlined directly into the public static `Transform`. Ported upstream's full
        nested `Transform` test class (9 tests) 1:1. **Testing nuance found and preserved
        faithfully**: 2 of those upstream tests (`Points_are_translated`,
        `Applies_rotation_and_translation`) use upstream's own `CollectionAssert.Equal` test
        helper (`tests/MonoGame.Extended.Tests/CollectionAssert.cs`), which is an
        order-*insensitive* containment check (same count + every actual element present in
        expected), not an ordered sequence comparison — unlike gtest's `EXPECT_EQ` on a
        `std::vector`. A literal `EXPECT_EQ` port of those two tests' expected-point lists
        failed even though the underlying `OrientedRectangle::Transform`/`getPointsProperty()`
        math was correct (upstream's own hand-written expected order doesn't match its own
        `Points` getter's algorithmic order either — it only passes upstream because
        `CollectionAssert.Equal` doesn't check order). Added a small local
        `ExpectUnorderedPointsEqual` helper replicating that exact upstream semantics for
        those 2 tests, rather than reordering the expected values to hide the discrepancy —
        this is a faithful port of what upstream's test actually asserts, not merely of its
        C# syntax.
      - Widened the sweep past the 3 explicitly-flagged types by grepping the tree for
        "PrimitivesHelper" in deferral comments: found and landed `CircleF`'s
        `Intersects(CircleF, BoundingRectangle)` (collapsed from upstream's 4 ref/value
        overloads to 1 static + 1 instance, matching the existing CircleF-vs-CircleF
        convention) and `Segment2`'s `Intersects(RectangleF|BoundingRectangle, out
        Vector2)` (2 overloads, using `PrimitivesHelper.IntersectsSlab` directly). Fresh
        tests for both (upstream's own coverage is entirely commented out for both).
- [x] `Math/Triangulation/*` (2026-07-13, ported via a forked sub-agent) — all 6
      upstream files (`Vertex`, `LineSegment`, `Triangle`, `CyclicalList`,
      `IndexableCyclicalLinkedList`, `Triangulator` + the `WindingOrder` enum) fully
      ported into a new `CNA::Extended::Triangulation` sub-namespace (matching the
      `Shapes` sub-namespace precedent from task 22), no deferrals. **Different license
      provenance than every other file so far**: all 6 upstream files cite
      `MIT Licensed: https://github.com/nickgravelyn/Triangulator` (an ear-clipping
      triangulation implementation), the same "code directly derived from a different
      MIT project" situation as `Angle.cs`/SlimMath — handled directly (not delegated to
      the fork) since it required license research: the original repo now 404s on GitHub
      (web UI and API both checked), so `NOTICE.md`'s new entry uses a standard MIT
      template crediting nickgravelyn with an explicit provenance caveat, rather than a
      verbatim-unavailable original copyright line. All 6 ported files carry a matching
      SPDX header crediting nickgravelyn, see `NOTICE.md`.
      **`CyclicalList<T>`/`IndexableCyclicalLinkedList<T>` design**: upstream implements
      these as `List<T>`/`LinkedList<T>` subclasses using C#'s `new`-keyword method
      hiding for their cyclical indexers — both are purely internal to `Triangulator`
      (upstream's own doc comment: "the sole public class in the entire library"), so
      rather than replicate that hiding mechanism, they compose (not inherit)
      `sharp-runtime`'s `System::Collections::Generic::List<T>`/`LinkedList<T>`, per this
      project's "reuse sharp-runtime, don't re-roll" convention, adding only the
      cyclical `operator[]`/`RemoveAt`/`IndexOf` on top. Header-only templates, matching
      `Interval<T>`'s precedent.
      **Critical fidelity point**: upstream's `Triangulator` has 5 `static readonly`
      mutable buffer fields shared across every call — a deliberate (if leaky) upstream
      design tradeoff for reduced per-call GC pressure, explicitly explained in its own
      doc comment, making `Triangulate`/`CutHoleInShape` non-reentrant and not
      thread-safe by design. Ported as genuine `static inline` C++ class members (not
      locals, not `thread_local`) to preserve that exact tradeoff, documented
      prominently in `Triangulator.hpp`'s header comment.
      `IsReflex` is dead code upstream (never called in `Triangulator.cs`) — ported
      anyway for completeness as a private static method (no compiler warning, unlike an
      unused free function). Independently re-verified (not just trusting the fork's
      self-report): grepped all 6 upstream `.cs` files for `public |internal ` members
      and confirmed every one has a mapped fate; hand-traced the trickiest method
      (`CutHoleInShape`'s nullable-comparison/index-injection logic) line-by-line against
      the C# source; did a clean `rm -rf build && cmake … && cmake --build …` from
      scratch to confirm zero new compiler warnings (not just trusting a possibly-cached
      prior build). Test suite grew from 464 to 503 (39 net new tests): upstream's one
      active test file (`TriangulatorTests.cs`, 4 `DetermineWindingOrder` tests,
      including a shoelace-formula regression test for issue #791) ported 1:1; fresh
      tests for everything else (no upstream coverage exists for `Triangulate`,
      `CutHoleInShape`, `EnsureWindingOrder`, `ReverseWindingOrder`, or any of `Vertex`/
      `LineSegment`/`Triangle`/the two cyclical collection types). Both build
      configurations verified clean, zero warnings, independently confirmed
      (503/503 `ctest`).
- [x] `GameTimeExtensions`, `GameComponentCollectionExtensions` (2026-07-13, ported
      directly, no fork) — both fully ported, no deferrals; small (two files, ~36 lines
      of C# total). `GetElapsedSeconds` -> a free function taking `const GameTime&`.
      `GameComponentCollectionExtensions`'s two `Add<T>` overloads -> free function
      templates taking `GameComponentCollection&`, returning `T*` (matches
      `GameComponentCollection`'s own non-owning `IGameComponent*`-based design in CNA;
      confirmed by reading `GameComponentCollection.cpp` that it never `delete`s its
      items — caller owns component lifetime, same as C#'s GC ownership model
      translated to explicit pointers). C#'s `Func<T> createGameComponent` ->
      `std::function<T*()>`, matching this project's established `Func<T>` convention
      (see `HslColor.hpp`'s `Match`/`Map`). No upstream tests for either file — wrote
      fresh tests. Both build modes clean, `ctest` → 507/507 (was 503).
- [x] `FramesPerSecondCounter` + `FramesPerSecondCounterComponent` (2026-07-13, ported
      directly, no fork) — both fully ported, no deferrals. `FramesPerSecondCounter`
      implements CNA's `IUpdateable` (`System::Object` base +
      `EnabledChanged`/`UpdateOrderChanged` as public `System::EventHandler` members +
      `getEnabledChangedEvent()`/`getUpdateOrderChangedEvent()`), following the exact
      pattern already established by CNA's own `GameComponent.hpp`/`.cpp` (first
      `System::Object`-deriving type in `cna-extended`). **Likely upstream bug, preserved
      for fidelity**: the `UpdateOrder` setter raises `EnabledChanged`, not
      `UpdateOrderChanged` -- almost certainly a copy-paste error from the `Enabled`
      setter immediately above it in `FramesPerSecondCounter.cs`. Reproduced exactly,
      documented prominently in the header, covered by a regression test that names the
      discrepancy explicitly (matches the `Segment2.SquaredDistanceTo` precedent from
      task 18). `_oneSecondTimeSpan` (`static readonly` upstream) ported as a local value
      constructed where needed rather than a `static const` class member, avoiding any
      static-initialization-order risk (the `Matrix3x2::Identity` lesson from task 20) --
      a storage-duration simplification only, not a logic change, since the value never
      varies. `FramesPerSecondCounterComponent` (`DrawableGameComponent` subclass,
      forwards `Update`/`Draw` to an internal `FramesPerSecondCounter`) requires a live
      `Game&` to construct; ported without a dedicated test file, matching CNA's own
      established precedent for this exact situation (`cna`'s own
      `DrawableGameComponentTests.cpp`: "No tests: DrawableGameComponent requires a live
      Game and GraphicsDevice (SDL/GPU)."). All the independently-testable logic lives in
      `FramesPerSecondCounter`, which is fully unit tested. No upstream tests exist for
      either file. Both build modes clean, `ctest` → 516/516 (was 507).
- [x] `SimpleGameComponent` + `SimpleDrawableGameComponent` (2026-07-13, ported directly,
      no fork) — both fully ported, no deferrals. Lighter-weight abstract bases (don't
      need a `Game&`, unlike CNA's own `GameComponent`/`DrawableGameComponent`), each
      implementing multiple CNA interfaces directly (`IGameComponent`, `IUpdateable`,
      `System::IDisposable`, `System::IComparable<GameComponent>`,
      `System::IComparable<SimpleGameComponent>` for the first; `+IDrawable` for the
      second). **Explicit-interface-implementation translation**: upstream uses C#'s
      explicit interface implementation twice (`bool IUpdateable.Enabled => _isEnabled;`
      alongside public `IsEnabled`; `bool IDrawable.Visible => _isVisible;` alongside
      public `Visible`) — C++ has no direct equivalent, approximated per-case:
      `IsEnabled`/`Enabled` genuinely differ in name upstream, so ported as two distinct
      C++ members (a public `getIsEnabledProperty()` plus a *private* override of
      `IUpdateable::getEnabledProperty()`, callable only through an `IUpdateable&`
      reference via virtual dispatch — the closest C++ analog to "interface-only
      access"); `Visible`/`IDrawable.Visible` share the identical name upstream (nothing
      distinct to preserve), so collapsed to one public `getVisibleProperty()` override
      satisfying `IDrawable` directly — both getters return the same field in both
      cases regardless, so neither translation changes observable behavior. No upstream
      tests exist for either file — wrote fresh tests (via minimal concrete test
      subclasses, since both are abstract) covering enabled/visible/update-order/draw-
      order change events (raised only when the value actually changes),
      `Initialize()`/`Dispose()` idempotency, and `CompareTo` ordering. Both build modes
      clean, `ctest` → 529/529 (was 516).
- [x] Collections: `Bag<T>`, `Deque<T>` (2026-07-13, ported via a forked sub-agent) —
      both fully ported, no deferrals. First use of a new `CNA::Extended::Collections`
      sub-namespace. Both are header-only templates (matching `Interval<T>`/
      `Triangulation`'s established convention for generic C# types).
      **`Bag<T>` has yet another different license than everything ported so far**:
      unlike every other file (MonoGame.Extended MIT, or the `Angle`/`Triangulation`
      MIT-from-a-different-project cases), `Bag.cs`'s own header credits a
      **2-clause-BSD-style** license — GAMADU.COM's C# port (2013) of thelinuxlich's
      [artemis_CSharp](https://github.com/thelinuxlich/artemis_CSharp) `Bag<T>`. Handled
      the license research myself (not delegated): unlike the `nickgravelyn/Triangulator`
      case, `artemis_CSharp` is still live — fetched its actual `Bag.cs` directly from
      the source repository and confirmed the license text matches MonoGame.Extended's
      embedded copy verbatim, word for word. Added a NEW `NOTICE.md` section ("Code
      directly derived from other permissively-licensed (non-MIT) projects"), distinct
      from the existing MIT-project section, since BSD-2-clause is a genuinely different
      license family from MIT, not just a different copyright holder.
      **`Deque<T>` implements `sharp-runtime`'s own `System::Collections::Generic::
      IList<T>`**, matching how `sharp-runtime`'s own `List<T>` implements the same
      interface. Two mechanical, no-behavioral-consequence simplifications documented in
      `Bag.hpp`'s header (upstream's `_isPrimitive` GC-hygiene-only array-clearing skip;
      upstream's `BagEnumerator`/boxing-avoidance enumerator triad, replaced by plain
      `begin()/end()`, since C++ range-based `for` never has the C#-specific
      boxing-allocation problem that machinery exists to solve).
      **Confirmed and preserved upstream bugs, the most significant found in this port so
      far** (all independently re-verified by hand-tracing concrete examples against the
      actual upstream algorithm before trusting the fork's claims — see §6 for the full
      account and specific traced examples):
      1. `IndexOf`'s final index-computation formula doesn't check for "not found" (`-1`)
         before applying modulo arithmetic, so a genuinely-absent item can make `IndexOf`
         return a bogus non-negative index instead of `-1` — which then makes `Remove`
         either throw unexpectedly or **silently remove an unrelated, real element while
         reporting success**, depending on the buffer's current wraparound offset. A
         freshly-constructed, never-grown `Deque` additionally hits a literal
         divide-by-zero in this same formula (C#: catchable `DivideByZeroException`;
         C++: UB, so explicitly guarded and thrown as `std::domain_error` instead, since
         UB itself cannot be "faithfully preserved" as an observable behavior).
      2. & 3. `RemoveAt`'s middle-index removal (choosing "shift the shorter half") is
         only reliable in one of its three practical cases: **front-half-shift on a
         non-wrapped buffer works correctly; back-half-shift is broken even on a
         non-wrapped buffer** (its shift source, `arrayCenterIndex`, is a fixed physical
         midpoint with no relationship to the actual removal point); **front-half-shift
         is ALSO broken once the buffer has wrapped** (an element is silently lost/
         orphaned, replaced by a stale default value). Not caught by upstream's own test
         suite because its one relevant test only asserts `Count` decrements correctly,
         never that the resulting values are correct. All three preserved exactly (not
         fixed), each with a dedicated, hand-verified regression test.
      Smaller preserved quirks: `CopyTo` validates the destination-index bound
      unconditionally, even before the `Count == 0` no-op check (so copying into an
      empty-but-valid destination can throw rather than succeed as a no-op);
      `IList<T>::Insert` always throws (`Deque<T>` only supports front/back insertion),
      matching the `OrientedRectangle::setPositionProperty` "matches upstream, always
      throws" precedent from task 17.
      **Iteration required real care**: upstream's `GetEnumerator()` re-reads
      `_frontArrayIndex`/`Count` *live* on every loop-condition check (not just once at
      iteration start) — deliberate, and covered by an upstream test
      (`Deque_ForEach_Iteration_Modified`) that removes from the front *during*
      iteration and expects every remaining element to still be visited exactly once. A
      naive C++ translation that snapshots state once at `begin()` would not reproduce
      this. Solved by capturing the physical starting offset/capacity once, tracking a
      visited-count, and re-deriving both the current element and the loop-termination
      condition from live state on every step — verified to collapse upstream's two
      differently-shaped loops (wrapped vs. non-wrapped) into one identical physical-index
      formula. Ported all 17 active upstream `DequeTests.cs` tests 1:1, plus 24 fresh
      tests including explicit regressions for all three bugs above and the live-iteration
      behavior. `Bag<T>`'s one upstream test is a C#-GC-allocation-boxing benchmark with
      no C++ equivalent concept — not ported; 19 fresh tests added instead covering actual
      correctness. Test suite grew from 529 to 589 (60 net new tests). Both build
      configurations verified clean from a fully clean rebuild (not a cached one), zero
      new compiler warnings, independently re-confirmed (not just trusting the fork's
      self-report).
- [x] Collections: `ObjectPool<T>`, `Pool<T>`, `IPoolable`, `ItemEventArgs` (2026-07-13,
      ported directly, no fork; small, 244 lines of C# total). All four fully ported, no
      deferrals; header-only templates. `IPoolable::NextNode`/`PreviousNode` -> `IPoolable*`
      (`ObjectPool<T>` `static_cast`s to `T*` at every point upstream does an unchecked
      `(T)node.NextNode` cast, trusting the same invariant upstream does — only
      T-implementing instances are ever linked into a given pool's list). `event Action<T>
      ItemUsed`/`ItemReturned` -> `System::MulticastAction<T*>` (a true multicast delegate,
      unlike `EventHandler<TEventArgs>`'s (sender, args) shape — `MulticastAction<Args...>`
      was added to `sharp-runtime` earlier in this project specifically for this kind of
      need). `Pool<T>`/`ObjectPool<T>`'s `where T : class` reference-type constraints map
      to working in `T*` throughout, consistent with `GameComponentCollectionExtensions.hpp`.
      **Found and preserved a severe, genuine upstream bug in `ObjectPool<T>` — an actual
      infinite loop, not just a wrong value, confirmed by hand-tracing the C# source
      byte-for-byte myself (not delegated, not assumed)**: `CreateObject()` unconditionally
      sets `_tailNode = item` as its last step, immediately before `New()` calls `Use(item)`
      on that exact same item; `Use()`'s `if (_tailNode is null)` check — meant to detect
      "is this the pool's first-ever node" — is therefore always false for every
      freshly-created item, so `Use()` always takes the branch that sets
      `item.PreviousNode = _tailNode` and `_tailNode.NextNode = item` — but since
      `_tailNode` IS `item` at that point, the node ends up pointing to **itself**.
      For a pool's very first item, this means `GetEnumerator()`'s `while (node != null) {
      yield return node; node = node.NextNode; }` never terminates, because
      `node.NextNode == node`. The self-reference is only retroactively fixed by a later
      `CreateObject()` call (for `NextNode`) or a `Return()` call (which unconditionally
      resets the current tail's `NextNode` to null) — so enumerating a pool whose items
      were obtained purely via `New()`, with none ever `Return()`ed, hangs on the current
      tail. Reproduced exactly (not fixed), documented prominently in `ObjectPool.hpp`'s
      header comment, and covered by regression tests that deliberately assert on
      `getNextNodeProperty()`/`getPreviousNodeProperty()` directly rather than by
      iterating the pool — iterating in the exact scenario being tested is the bug itself
      and would hang the test process. Smaller preserved quirk: `Pool<T>::Free()` calls
      `resetItem` unconditionally, even when the item wasn't actually kept because the
      pool was already at its `maximum`. Ported upstream's one active `ObjectPoolTests.cs`
      test 1:1 (its own name is stale/misleading — it says
      "ThrowsNullReferenceException" but actually asserts no exception is thrown); fresh
      tests for everything else including both `Pool<T>` (no upstream coverage at all)
      and the `ObjectPool<T>` full/policy paths (`ReturnNull`/`IncreaseSize`/
      `KillExisting`) and the infinite-loop-bug regressions. One of my own test-authoring
      mistakes caught before landing: an initial test assumed `Capacity`'s full-pool
      policy applies once `TotalCount > Capacity` naively at the second `New()` call, but
      upstream's actual guard is `TotalCount <= Capacity` (note `<=`), which allows
      `Capacity + 1` total items to be freely created before the policy switch ever
      triggers — traced and corrected before committing, not just assumed correct. Test
      suite grew from 589 to 608 (19 net new tests). Both build modes verified clean, and
      the full suite explicitly re-run under a 60-second `timeout` wrapper as an extra
      safety net given the infinite-loop bug under test, confirming no hang.
- [x] Collections: `KeyedCollection`, `DictionaryExtensions`, `ListExtensions`
      (2026-07-13, ported directly, no fork; small, 106 lines of C# total across the 3
      files). `KeyedCollection<TKey, TValue>` and `ListExtensions.Shuffle` fully ported
      as header-only templates. **`DictionaryExtensions.GetValueOrDefault` was NOT
      ported** — verified (not assumed) that `sharp-runtime`'s own
      `System::Collections::Generic::Dictionary<TKey, TValue>::GetValueOrDefault(key,
      defaultValue = TValue{})` already implements byte-for-byte identical semantics to
      upstream's extension method (both: return the value if present, else a supplied or
      default-constructed fallback), unlike the `RandomExtensions::NextSingle` case from
      task 21 where a same-named `sharp-runtime` method turned out to implement a
      genuinely different algorithm and had to be re-ported instead of reused. Since the
      semantics here are a true match, this is a case where "reuse `sharp-runtime`, don't
      re-roll" applies with no exception — adding a redundant free-function wrapper
      around an already-identical member method would be pure duplication with zero
      fidelity benefit, so this file is intentionally not ported; see the decisions log
      for the full account. `KeyedCollection<TKey, TValue>` is backed by that same
      `Dictionary<TKey, TValue>` (whose `operator[](key) const` already throws
      `KeyNotFoundException` for a missing key, exactly matching C#'s `Dictionary`
      indexer). `CopyTo` matches upstream: always throws (`NotSupportedException`
      upstream). `ListExtensions.Shuffle`'s `IList<T>` parameter maps to `std::vector<T>&`
      (matching every other C# `IList<T>`/array translation in this project) and
      correctly returns a reference to the same, now-shuffled vector, preserving
      upstream's fluent/chainable return. No upstream tests exist for any of the 3
      files — wrote fresh tests for `KeyedCollection`/`Shuffle` (determinism,
      element-preservation, edge cases). Test suite grew from 608 to 624 (16 net new
      tests). Both build modes clean.
- [x] Port `tests/MonoGame.Extended.Tests/{Math,Primitives,Shapes,Collections}` as
      GoogleTest suites (2026-07-13) — audited personally first: all 16 upstream files
      under the 4 literal named folders were already fully covered by tests ported
      during each type's own earlier implementation task, confirmed by reading every one
      and cross-checking against the corresponding `cna-extended` test file (not just
      trusting a stale assumption). **Widened the audit past the plan's literal 4-folder
      wording** to root-level upstream test files (`tests/MonoGame.Extended.Tests/*.cs`,
      not in a subfolder) testing Phase-1-scope types, since this task's actual intent —
      full Phase-1 test parity — is broader than its literal wording. That wider sweep
      found a real, substantial gap: `BoundingBox2D`/`BoundingCapsule2D`/
      `BoundingCircle2D`/`BoundingPolygon2D`/`OrientedBoundingBox2D`/`LineSegment2D` each
      had noticeably fewer tests ported than upstream's own root-level test files for
      those types actually have (`OrientedBoundingBox2D` was the largest gap: 13 ported
      vs. 33 active upstream). Delegated closing this gap to a forked sub-agent, given
      the volume (7 files audited for gaps, 8 more given a lighter verification pass).
      Fork's findings, independently verified before landing (re-read 2 of the 6 modified
      files' diffs against the actual upstream `.cs` source line-by-line, including the
      most calculation-heavy new tests like `OrientedBoundingBox2D`'s
      `CreateFromRotation90Degrees`/`TransformNonUniformScale` — all matched exactly):
      +43 tests total across `BoundingBox2D` (+4), `BoundingCapsule2D` (+5),
      `BoundingCircle2D` (+8), `BoundingPolygon2D` (+11), `LineSegment2D` (+6),
      `OrientedBoundingBox2D` (+9); `HslColor` had no real gap (the count mismatch was a
      `[Theory]`-vs-`TEST_P` parameterized-test counting artifact, not a coverage gap).
      Confirmed (against each type's own header-comment-documented deferred-vs-ported
      split, not assumed from test names) that every remaining un-ported upstream test in
      these 6 files genuinely depends on the not-yet-ported `Collision2D` system
      (`TryGetCollision_*`, `Contains`/`ContainsPoint` where `Contains` itself is
      deferred, `Distance*`/`Intersects` on `LineSegment2D`) or has no C++-translatable
      concept (`GetCorners_ThrowsWhenArrayNull` — this project's `std::vector<T>&`
      out-param convention has no "null" case). **No genuine discrepancies found** — no
      case where a header comment claimed something was ported but the functionality
      didn't actually exist. The lighter verification pass on 8 more root-level files
      (`Angle`, `ColorExtensions`, `ColorHelper`, `Line2D`, `MathExtended`, `Ray2D`,
      `RectangleExtensions`, `Vector2Extensions`) confirmed all already at parity or
      better (extra fresh coverage beyond upstream). `OrthographicCameraTests.cs` (53
      active upstream tests) intentionally excluded — `OrthographicCamera` itself remains
      correctly deferred to Phase 3 pending `ViewportAdapters`, not a Phase 1 gap. Test
      suite grew from 624 to 663 (39 net new tests). Both build configurations verified
      clean from a genuinely clean rebuild, independently re-confirmed by me (not just
      trusting the fork's self-report), including the headers-only compile-check config.
      **This was the last Phase 1 task — Phase 1 is now complete.**

### Phase 2 — Collisions 2D

Depends on Phase 1 (math/shapes).

- [ ] Root types: `Collision2D`, `CollisionResult2D`, `CollisionShape2D`,
      `CollisionShapeKind2D` — **IN PROGRESS (2026-07-13)**. `CollisionShapeKind2D` and
      `CollisionResult2D` fully ported. `Collision2D` (3,809 upstream lines) is now 100%
      ported — all 79 `public static` methods present, self-check diff confirms zero
      missing/extra — but its test suite is NOT at parity yet: `Projection Methods`,
      `Distance Calculations`, `ClosestPointRaySegment Tests`, `ClipLineToAabb`/
      `ClipLineToConvexPolygon` tests, `Overlap Methods`, and all 15 plain-`bool`
      `Intersects*` sub-regions from upstream `Collision2DTest.cs` still need porting
      (implementations exist and are exercised transitively, but have no direct test
      coverage). `CollisionShape2D` (713 lines) not yet started — depends on
      `Collision2D::TryGetCollision*`, now unblocked. See `NEXT.md` entry (20) for the
      full breakdown. Do not check this task off until both the test-parity gap is
      closed and `CollisionShape2D` lands.
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
- 2026-07-13 — `CircleF`/`EllipseF`/`Segment2` ported via a forked sub-agent. Found (and
  faithfully preserved, per the standing no-simplification mandate) a genuine upstream bug:
  `Segment2.SquaredDistanceTo` silently drops a `return` for points beyond the segment's
  `End` (see the Phase 1 checklist entry above for the exact line and confirmed wrong
  value) — independently re-verified by reading `Segment2.cs:96` directly, confirmed the
  fork's finding is accurate. This is the first confirmed upstream bug found during
  porting; if another turns up, apply the same rule: preserve it, document it prominently
  in the header, cover it with a regression test that names the discrepancy explicitly —
  never silently "fix" it while porting.
- 2026-07-13 — `Size`/`SizeF`/`Interval`/`Thickness` ported via a forked sub-agent, with
  an explicit heads-up in the prompt about the three specific `SizeF`-blocked follow-up
  spots (`ISizable`, `RectangleF`, `BoundingRectangle`) to check once `SizeF` landed. All
  three verified genuinely unblocked and landed in the same pass — see the Phase 1
  checklist entry above for exactly what was added to each. User also asked mid-task to
  push work so far to GitHub, and to create + push a new `develop` branch; both done
  (`master` and `develop` both exist on `origin` as of this session; ongoing work commits
  to `develop` going forward per the user's instruction).
- 2026-07-13 — `Matrix3x2`/`MatrixExtensions`/`Vector2Extensions` ported via a forked
  sub-agent, with an explicit heads-up about `Matrix3x2` being the single most-referenced
  blocker so far. Landed `Transform2` as a verified bonus follow-up in the same pass (see
  Phase 1 checklist). Precisely distinguished "unblocked" from "partially unblocked" for
  the other flagged spots — `RectangleF`/`BoundingRectangle`'s `Transform` and `CircleF`/
  `Segment2`'s remaining deferrals are still blocked on `PrimitivesHelper` specifically,
  not Matrix3x2; `OrientedRectangle` is now *partially* unblocked (constructor + field
  only need `SizeF`+`Matrix3x2`, both available) but its `Transform` method still needs
  `PrimitivesHelper` — flagged as a good near-term small follow-up task rather than
  guessed at. This session's second confirmed bug, this time in cna-extended's own new
  code (not upstream): a static-initialization-order fiasco in `Matrix3x2::Identity`
  (built from another translation unit's static `Vector2` constants), caught by 2 failing
  tests and fixed with literal values. Worth remembering as a general C++ hazard for any
  future `static const Type X = Type(other-class's-statics...)` pattern, not just this
  one instance.
- 2026-07-13 — Picked up the `OrientedRectangle` follow-up immediately (ported directly,
  no fork — small enough by this point). Re-scoping it turned up a correction to the
  previous task's fork report: `BoundingRectangle` (the property) and
  `explicit operator RectangleF(OrientedRectangle)` are *also* transitively blocked on
  `PrimitivesHelper` (both call `RectangleF::Transform` internally), not just the static
  `Transform` method as previously reported. A reminder that "only X is blocked" claims
  from a fork (or from this session's own prior notes) are worth re-verifying by reading
  the actual dependency chain, not just trusting the most recent summary — the same
  discipline already applied to fork self-reports applies to this session's own carried-
  forward notes too.
- 2026-07-13 — `FastRandom`/`RandomExtensions` ported directly (small enough, no fork).
  Deliberately did NOT reuse `sharp-runtime`'s own `System::Random::NextSingle()` for
  `RandomExtensions::NextSingle`, even though it exists and "reuse sharp-runtime, don't
  re-roll" is the usual rule — that method implements a different (more modern, matching
  real .NET 6+) algorithm than what `RandomExtensions.cs` itself actually does (a plain
  `(float)NextDouble()` cast). The "reuse, don't re-roll" principle is about not
  duplicating *equivalent* functionality; it does not override the fidelity mandate when
  the existing utility's behavior actually differs from what upstream's specific file
  does. Worth keeping in mind for any future case where a sharp-runtime type has a
  same-named member that isn't a drop-in behavioral match for what's being ported.
- 2026-07-13 — `PrimitivesHelper` ported directly (no fork; fully self-contained, ~145
  lines). While scoping `ShapeExtensions` (deferred whole to Phase 5 — pure `SpriteBatch`
  debug-drawing code, not a math utility despite its `Math/` folder location), discovered
  its dependency on `MonoGame.Extended.Shapes.Polygon`/`Polyline`, an entire upstream
  folder never tracked anywhere in this plan — a real gap, not an intentional exclusion.
  Both types were small and fully self-contained, so ported them alongside this task
  rather than leaving the gap open; first use of a `CNA::Extended::Shapes` sub-namespace.
  Then, rather than assuming `PrimitivesHelper` unblocked the several `RectangleF`/
  `BoundingRectangle`/`OrientedRectangle` deferrals flagged by name in earlier sessions'
  notes, independently re-verified each by reading the current header's deferral comment
  plus the actual upstream `.cs` implementation before touching any code (the same
  discipline established in the 2026-07-13 `OrientedRectangle` entry above, now applied to
  a 3-type sweep instead of a single correction). Confirmed `Line2D`/`LineSegment2D`'s
  deferrals are genuinely still blocked on `Collision2D`, not `PrimitivesHelper`, despite
  citing an overlapping algorithm source — left those deferred rather than guessing they
  were unblocked because a plausibly-related type had just landed. Landed the genuinely-
  unblocked follow-ups in the same commit: `RectangleF::Transform`/`CreateFrom(points)`/
  `UpdateFromPoints`/`SquaredDistanceTo`/`DistanceTo`/`ClosestPointTo`;
  `BoundingRectangle`'s equivalents; `OrientedRectangle::Transform`/
  `getBoundingRectangleProperty()`/`operator RectangleF()`. Porting `OrientedRectangle`'s
  upstream `Transform` test class surfaced a test-infrastructure fidelity question, not a
  code one: 2 of the 9 tests use upstream's `CollectionAssert.Equal` helper, which is
  order-*insensitive* (contains-check, not sequence-equality) — a literal `EXPECT_EQ` port
  of those two tests' expected point lists failed the build even though the underlying
  `Transform`/`getPointsProperty()` math was correct, because upstream's own hand-written
  expected order doesn't match its own `Points` getter's algorithmic order either (it only
  passes upstream *because* the helper ignores order). Resolved by writing a small
  `ExpectUnorderedPointsEqual` helper that replicates the C# helper's actual semantics for
  just those 2 tests, rather than silently reordering the expected values to make
  `EXPECT_EQ` pass — porting a test faithfully means porting what it actually asserts, not
  just its literal C# structure. Widened the sweep beyond just the 3 flagged types: grepped
  the whole tree for "PrimitivesHelper" in deferral comments and found 2 more genuinely-
  unblocked spots not previously called out by name — `CircleF`'s 4-overload (collapsed to
  2) `Intersects(CircleF, BoundingRectangle)`, which calls
  `BoundingRectangle::SquaredDistanceTo`, and `Segment2`'s
  `Intersects(RectangleF|BoundingRectangle, out Vector2)`, which calls
  `PrimitivesHelper.IntersectsSlab` directly — landed both in the same commit with fresh
  tests (upstream's own coverage for all of these is entirely commented out in its test
  suite, so nothing to port 1:1). Test suite grew from 429 to 464 (35 net new tests); both
  `-DCNA_EXTENDED_LINK_CNA=ON` and headers-only builds verified clean.
- 2026-07-13 — `Math/Triangulation` (6 upstream files) ported via a forked sub-agent.
  Before delegating, discovered all 6 files cite `MIT Licensed:
  https://github.com/nickgravelyn/Triangulator` in their own headers — the same "code
  directly derived from a different MIT project" situation as `Angle.cs`/SlimMath, not
  the usual "algorithm citation in a comment" pattern seen elsewhere in `Math/`. Did the
  license diligence myself (not delegated): fetched the cited URL directly (404) and via
  the GitHub API (404), searched for mirrors/archives/renamed accounts (none found),
  confirmed via web search that multiple independent sources agree the project was MIT
  licensed but none could supply the exact original copyright wording. Added a `NOTICE.md`
  entry using the standard MIT template with an explicit provenance caveat rather than
  guess at wording, and gave the fork the exact SPDX header text to place in all 6 files
  so it wouldn't need to make that judgment call itself. Fork's `CyclicalList<T>`/
  `IndexableCyclicalLinkedList<T>` design choice (composition around `sharp-runtime`'s
  `List<T>`/`LinkedList<T>` rather than replicating C#'s `new`-keyword-hiding
  subclassing) is a good precedent for any future upstream type that inherits from a C#
  collection purely for internal use. Independently re-verified the fork's work before
  committing (per this session's standing "never just trust a fork's self-report" rule):
  re-grepped all 6 upstream files for `public |internal ` members against the ported
  headers (nothing dropped), hand-traced `CutHoleInShape` — the trickiest method, with
  nullable-comparison logic and cyclical-list index injection — line-by-line against the
  C# source (faithful), and did a clean `rm -rf build` + full reconfigure/rebuild myself
  rather than trusting the fork's already-built `build/` directory, confirming zero new
  compiler warnings and 503/503 `ctest` from scratch. No upstream bugs found this time —
  the algorithm translated cleanly.
- 2026-07-13 — `FramesPerSecondCounter`/`FramesPerSecondCounterComponent` ported
  directly (no fork; tiny, ~100 lines of C# total). First `cna-extended` type to
  implement CNA's own `IUpdateable` interface and derive from `System::Object` — followed
  CNA's own `GameComponent.hpp`/`.cpp` pattern closely rather than inventing a new one
  (public `EventHandler<EventArgs>` members, `getXChangedEvent()` accessor overrides,
  `Raise(this, EventArgs::Empty)` for firing). Found this session's third confirmed
  upstream bug: `FramesPerSecondCounter.UpdateOrder`'s setter raises `EnabledChanged`
  instead of `UpdateOrderChanged` (a copy-paste error from the `Enabled` setter directly
  above it) — preserved exactly, documented prominently, covered by an explicit
  regression test naming the discrepancy, matching the `Segment2.SquaredDistanceTo`
  precedent rather than "fixing" it. `FramesPerSecondCounterComponent` needs a live
  `Game&` to construct (it's a `DrawableGameComponent`); confirmed via `cna`'s own test
  suite (`DrawableGameComponentTests.cpp`) that this project's sibling repo already
  established the precedent of not unit-testing that base class for exactly this reason
  — followed the same precedent rather than inventing test infrastructure CNA itself
  doesn't have. Caught and fixed one of my own test-authoring mistakes before landing:
  an initial test assumed `FramesPerSecondCounter`'s internal timer starts at zero, but
  it actually starts pre-loaded at a full second (matches upstream's own
  `_timer = _oneSecondTimeSpan;` field initializer) — meaning the very first `Update()`
  call always latches a `FramesPerSecond` reading immediately, regardless of how little
  time has actually elapsed. Traced the arithmetic by hand to confirm this is correct
  *upstream* behavior (not a fidelity bug in the port), then rewrote the test to assert
  the real behavior instead of masking it.
- 2026-07-13 — `SimpleGameComponent`/`SimpleDrawableGameComponent` ported directly (no
  fork; ~127 lines of C# total). First time this port had to translate C#'s explicit
  interface implementation — decided the policy case-by-case rather than applying one
  blanket rule: where upstream itself used two *different* names for the public property
  vs. the interface-only one (`IsEnabled` vs. `IUpdateable.Enabled`), preserved both as
  distinct C++ members (public getter + a private override of the interface method,
  reachable only through a base-interface reference — the closest C++ analog to
  "interface-only access," and legal C++: access specifiers gate name lookup, not
  virtual dispatch, so a private override is still called correctly through a base
  pointer/reference). Where upstream reused the *identical* name for both (`Visible` /
  `IDrawable.Visible`), there was no distinct name left to preserve, so collapsed to one
  public override — documented as a naming/visibility simplification only (both C#
  members always read the same backing field regardless, so no behavior differs either
  way). Worth remembering as the general rule for any future explicit-interface-
  implementation case: check whether upstream gave the two members different names
  before deciding whether a two-member or one-member C++ translation is the more
  faithful choice.
- 2026-07-13 — `Bag<T>`/`Deque<T>` ported via a forked sub-agent (combined ~1053 lines of
  C# source + 456 lines of upstream tests, well past the fork threshold). Before
  delegating, discovered `Bag.cs` carries a BSD-2-clause-style license from a *different*
  upstream project (GAMADU.COM/artemis_CSharp), not MonoGame.Extended's usual MIT
  header — handled that license research myself (see the Phase 1 checklist entry above
  for the full account) and gave the fork the exact SPDX header text to use, the same
  pattern established for the `Triangulation`/nickgravelyn case.
  The fork's report claimed to have found three genuine upstream correctness bugs in
  `Deque<T>` (not just fidelity/GC-hygiene quirks like everything found before this) —
  including two ("RemoveAt's back-half-shift is broken even unwrapped" and "front-half-
  shift is ALSO broken once wrapped") it said were discovered *empirically*, by writing a
  test that asserted the mathematically-correct result, watching it fail against a
  faithful 1:1 translation, then hand-tracing to confirm the failure was inherent to
  upstream's algorithm rather than a translation defect. Given the significance of that
  claim — "upstream's Deque has real data-corruption bugs" is a much bigger claim than
  "this rounds slightly differently" — did NOT just trust the fork's self-report or its
  test's assertions at face value. Independently re-derived all three bugs from scratch:
  hand-traced `IndexOf`'s formula and `RemoveAt`'s middle-index branches against the
  actual upstream `.cs` source using fresh concrete examples I constructed myself (not
  the fork's examples), computing expected physical-array and logical-view state by hand
  at each step. All three bugs reproduced exactly as claimed, and the fork's specific
  test-expected-values (e.g. the exact post-removal element sequence in
  `RemoveAtMiddleShiftsShorterBackHalfReproducesKnownUpstreamBug`) matched my
  independently-computed values precisely. This is the deepest independent re-verification
  a fork's bug-finding claim has received in this project so far, given the stakes (silent
  data corruption is a much more serious class of finding than a rounding/formatting
  quirk) — worth treating "this fork found a real bug, not just a fidelity note" claims
  as needing proportionally more scrutiny before committing, not just the standard
  self-check-plus-spot-read pass. Also re-ran the full build from a genuinely clean `rm
  -rf build` (not reusing the fork's already-built directory) for both CMake
  configurations before trusting the reported 589/589 pass count.
- 2026-07-13 — `ObjectPool<T>`/`Pool<T>`/`IPoolable`/`ItemEventArgs` ported directly (no
  fork; small, 244 lines of C# total). Found this session's fourth confirmed upstream
  bug, and by far the most severe: a genuine infinite loop in `ObjectPool<T>.
  GetEnumerator()` for the single most basic usage pattern (create one pooled object,
  enumerate the pool) — see the Phase 1 checklist entry above for the full mechanism.
  Unlike the `Deque<T>` bugs (found by a fork, then independently re-verified by me),
  this one was found by me directly while hand-tracing `CreateObject()`/`Use()` during
  the port itself — a useful reminder that reading upstream source line-by-line during
  direct (non-forked) porting surfaces real bugs just as often as adversarial
  re-verification of a fork's claims does; the "verify carefully" discipline this
  session established isn't only for reviewing other agents' work. Given the bug is a
  literal infinite loop, took explicit care that the regression tests demonstrating it
  can never accidentally hang the test suite: they assert on `getNextNodeProperty()`/
  `getPreviousNodeProperty()` directly rather than ever iterating the pool in the exact
  state that triggers the bug (a prominent warning to this effect is in both
  `ObjectPool.hpp`'s header and `ObjectPoolTests.cpp`'s header, so a future session
  doesn't "simplify" a test into iteration and reintroduce a hang) — and re-ran the
  entire suite under `timeout 60` as an explicit extra safety net before trusting a
  clean pass, on top of the normal build+test verification. Also caught and fixed my own
  test-authoring mistake: initially assumed `ObjectPool<T>`'s full-pool policy applies as
  soon as `TotalCount > Capacity` at the very next `New()` call, but upstream's actual
  guard is `TotalCount <= Capacity` (allowing `Capacity + 1` total creations before the
  policy ever triggers) — traced and corrected before committing, the same "verify, don't
  assume, even for my own test code" discipline applied throughout this session.
- 2026-07-13 — `KeyedCollection`/`ListExtensions` ported directly (no fork; `Dictionary
  Extensions` deliberately NOT ported — see the Phase 1 checklist entry above). This is
  the mirror image of the `RandomExtensions::NextSingle` situation from task 21: there, a
  same-named `sharp-runtime` method turned out to implement a *different* algorithm and
  had to be re-ported to preserve fidelity; here, `sharp-runtime`'s own `Dictionary<TKey,
  TValue>::GetValueOrDefault` was checked line-by-line against upstream's
  `DictionaryExtensions.GetValueOrDefault` and found to be a true semantic match (both:
  value-if-present, else a supplied-or-default fallback) — so "reuse, don't re-roll"
  applies cleanly this time, and no port was needed. Worth remembering as the general
  rule alongside the earlier lesson: a same-named `sharp-runtime` method isn't
  automatically reusable OR automatically to-be-avoided — check the actual algorithm
  each time, and the answer can go either way.
- 2026-07-13 — Phase 1's final task, the test-suite parity pass, started with a direct
  personal audit (not delegated) of all 16 upstream files under the plan's literal
  4-folder wording (`Math`/`Primitives`/`Shapes`/`Collections`) — confirmed 100% already
  covered. Then deliberately widened scope past that literal wording to root-level
  upstream test files testing Phase-1 types, since the task's actual purpose (full
  Phase-1 parity) is broader than 4 named subfolders — a plain `grep -c '\[Fact\]\|
  \[Theory\]'` count comparison between each upstream file and its ported counterpart
  surfaced a real, substantial gap in 6 bounding-volume/line-segment files (up to 20
  missing tests in one file). This is a useful precedent: when a task's literal written
  scope and its evident purpose diverge, checking the purpose-implied scope (here, cheap:
  a few `grep -c` calls) is worth doing before declaring a task done on the literal
  wording alone. Delegated the confirmed gap-filling to a forked sub-agent (7 files to
  check for real gaps, 8 more for a lighter verification pass) with explicit instructions
  to distinguish "genuinely un-ported, portable now" tests from "correctly deferred to
  Phase 2 pending `Collision2D`" tests by reading each type's own header-comment-
  documented deferral list, not by guessing from test names, and to flag (not silently
  paper over) any case where a test needed functionality a header comment claimed was
  already ported but which turned out not to actually exist. None were found — every
  header's documented deferred-vs-ported split held up exactly. Independently
  re-verified 2 of the fork's 6 modified files by reading the actual diffs against the
  upstream `.cs` source line-by-line (including the most calculation-heavy new tests,
  e.g. `OrientedBoundingBox2D::CreateFromRotation`/`Transform` with non-uniform scale) —
  all matched exactly — then did a genuinely clean `rm -rf build` + full rebuild of both
  CMake configurations myself before trusting the reported 663/663 pass count, the same
  "never just trust a fork's self-report" discipline applied to every fork in this
  session. **Phase 1 is now complete: all 30 tasks landed**, from `Version` through this
  test-parity pass, spanning ~50 forked-and-direct porting tasks, ~130 ported/created
  source files, and a test suite that grew from 0 to 663 passing tests. Four genuine
  upstream bugs were found and faithfully preserved along the way (`Segment2.
  SquaredDistanceTo`'s missing `return`, `RectangleF.Extensions.Clip`'s mutate-before-
  derive ordering, `FramesPerSecondCounter.UpdateOrder`'s wrong-event-raised copy-paste
  error, and `ObjectPool<T>`'s severe self-referencing-node infinite-loop bug), plus two
  confirmed-and-preserved `Deque<T>` correctness bugs (`IndexOf`'s not-found handling and
  `RemoveAt`'s middle-index shift logic) found by a fork and independently re-verified.
  Next per `plan.md` §5: Phase 2 ("Collisions 2D"), which unblocks the large cluster of
  `Intersects`/`Contains`/`TryGetCollision` methods deferred throughout Phase 1 — see
  `NEXT.md` for the concrete next-task pointer.

## 7. Open items to resolve during implementation (not blocking plan approval)

- `Graphics/Effects` shader resources: whether CNA's graphics backend(s)
  (`SDL_RENDERER`/`EASYGL`/`BGFX`/`VULKAN`) can consume a ported `DefaultEffect` as-is or
  need a backend-specific re-authoring — investigate at the start of Phase 5 and raise
  with the user if it's a real design fork, not a mechanical port.
- Whether `Serialization/*` should wrap `sharp-runtime`'s `System::Text::Json` directly or
  needs its own converter layer — decide at the start of Phase 6.
