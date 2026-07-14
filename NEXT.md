# NEXT.md — handoff document

This file is a snapshot-style handoff for resuming work on `cna-extended`, for either
Claude Code or a human developer. It reflects the repository state as observed at the time
of writing, not an aspirational or planned state. It replaces this file's previous
append-only chronological session-log format; the full history of that log (35 dated
entries covering Phases 0-5) remains available via `git log -- NEXT.md` / `git show
<commit>:NEXT.md` and is not reproduced here.

---

## 1. Project summary

`cna-extended` is a C++23 port of [MonoGame.Extended](https://github.com/craftworkgames/MonoGame.Extended)
(C#, MIT license) for [`cna`](../cna) (a C++23 port of XNA 4.0 / FNA), built on
[`sharp-runtime`](../sharp-runtime) (a C++23 .NET BCL reimplementation). It is a sibling
library to [`easy-3d`](../easy-3d), following the same conventions.

**Main goal**: port MonoGame.Extended faithfully 1:1 wherever C#/C++ differences allow — no
simplification. Scope was explicitly negotiated with the project owner and is recorded in
`plan.md` (`Status: APPROVED`, no longer draft).

**Current phase: DONE for the original port.** All 10 phases in `plan.md` are complete as
of 2026-07-14 — the MonoGame.Extended port itself is finished. See section 4 for how the two
former blockers (the `TilemapRenderer`/`TilemapWorldRenderer` `DefaultEffect` mismatch, and
its trailing pixel-verification test gap) were resolved, and section 8 for what a future
session might reasonably still do with `plan.md` (all optional/non-blocking).

**A second, separate, owner-approved plan is now active: `plan3d.md`.** Read `3d.md` (the
design analysis) and `plan3d.md` (the phase-ordered task list, `Status: APPROVED`) before
touching anything under `World3DEXT/`. It adds `CNA::Extended::World3DEXT` — a 3D scene
layer (camera, ECS transform hierarchy, model rendering, skinned animation, plus systematic
3D counterparts of Collisions/Graphics/Particles/Tilemaps) — built on top of the now-complete
2D port, not a rework of it. This is a genuinely separate, still-in-progress body of work;
`plan.md`'s "DONE" status above does not apply to it. As of 2026-07-14: Phases 1-4 are
complete (`Camera3DEXT`; `Transform3ComponentEXT`/`TransformHierarchySystemEXT`;
`ModelComponentEXT`/`RenderSystem3DEXT` with frustum culling; `SkinnedModelComponentEXT`/
`AnimationSystem3DEXT` with real skinned-animation pixel-verified tests, reusing `cna`'s
own `SkinnedModelEXT`/`AvatarRenderer::DrawRealEXT` draw recipe rather than reinventing
either); Phase 5 (`Collisions3DEXT`) is next — see `plan3d.md` §4.

**Important architectural decisions**:
- Namespace `CNA::Extended::<Module>`, sub-namespaced per module (e.g.
  `CNA::Extended::Graphics`, `::Screens`, `::Animations`, `::Content::TexturePacker`,
  `::BitmapFonts`, `::Content::BitmapFonts`) — not a 1:1 mirror of MonoGame.Extended's own
  namespace tree.
- C# properties → `getXProperty()` / `setXProperty()` methods.
- File layout mirrors the namespace path: `include/CNA/Extended/<Module>/<Type>.hpp` ↔
  `src/CNA/Extended/<Module>/<Type>.cpp`.
- CMake three-tier sibling-dependency pattern: look for an installed `CNA` package first,
  fall back to building the sibling `../cna` checkout (`CNA_EXTENDED_LINK_CNA=ON`), fall
  back further to a headers-only mode (`CNA_EXTENDED_LINK_CNA=OFF`, the default) if that's
  off.
- Zero-warning policy: `-Wall -Wextra -Werror` (`/W4 /WX` on MSVC) — non-negotiable.
- Tests are GoogleTest, ported alongside implementation in the same phase/commit.
- `MonoGame.Extended.Content.Pipeline` (the whole design-time MGCB assembly) and any
  `.xnb`-reading class are explicitly excluded. Direct-format loaders (already-ported
  TexturePacker JSON, BMFont `.fnt`; future Tiled/LDtk/Ogmo JSON) cover the same ground.

---

## 2. Current status

- **Build (linked config, `-DCNA_EXTENDED_LINK_CNA=ON`)**: clean. Last verified via a
  genuine `rm -rf build` + fresh configure + rebuild — exit 0, zero warnings.
- **Build (headers-only/default config, `-DCNA_EXTENDED_LINK_CNA=OFF`)**: clean, also
  verified via a genuine `rm -rf build-headers` rebuild.
- **Tests**: **2079/2079 tests run, 100% passing** (2 additional tests exist but are
  deliberately `GTEST_SKIP()`-guarded — see section 5's `cna` `BoundingFrustum` bug entry).
- **Currently available build outputs**: `CNA_EXTENDED` static library target,
  `cna_extended_minimal` and `cna_extended_tiled_demo` example executables,
  `CnaExtendedTests` GoogleTest binary, and (linked config only) generated Doxygen HTML
  under `docs/generated/html` (gitignored, regenerate with `doxygen Doxyfile`).
- **ALL 10 PHASES COMPLETE.** Phase 7's former architectural blocker is resolved
  (`TilemapRenderer`/`TilemapWorldRenderer` ported using `cna`'s real `BasicEffect`), its
  trailing pixel-verification test gap is closed
  (`TilemapIntegrationTests.cs`, 28 tests → 56 executed cases), and Phase 10 (end-to-end
  example under `examples/tiled_demo/`, a zero-warning Doxygen pass, `README.md`/
  `NOTICE.md` refresh) landed too — see section 3 for the full history. **There is no
  remaining phase.**
- **CORRECTED earlier this session, was wrong in even-earlier entries of this same file**: a
  real headless `GraphicsDevice`/`SpriteBatch`/`Texture2D` triple genuinely works end-to-end
  in this environment (real EasyGL-over-Mesa software rendering) — the "no headless
  SpriteBatch test infra" claim was an untested assumption inherited forward through this
  file, not a verified fact. `SpriteBatch.Extensions`, `BitmapFontExtensions`,
  `ShapeExtensions`, `FadeTransition`/`ExpandTransition::Draw` now all have real
  behavioral test coverage as a direct follow-up (47 new tests) — see section 3. This
  correction is now fully closed out, not just noted.
- **Does not work / not done yet**: nothing. See section 5 for known, non-blocking
  bugs/limitations (mostly in sibling repos, worked around locally) and `MISSING.md` for
  tracked CNA-vs-XNA API-shape gaps found during porting. See section 8 for optional
  follow-up ideas — none of them block anything or are required.

---

## 3. Recent changes

One long autonomous session (owner authorized, unavailable for hours), which ended up
completing the entire remaining plan. Order: Phases 4-5, then Phase 6 (Serialization) +
Phase 9 (ECS) in parallel, then Phase 7's Tilemaps core + the bulk of Phase 8 (Particles) in
parallel, then `Tilemaps/Tiled/*` + `ParticleEffectSerializer.cs` in parallel (Phase 8
completed in full), then `Tilemaps/LDtk/*` + `Tilemaps/Ogmo/*` in parallel, then
`Tilemaps/Rendering/*` (the last item in Phase 7 at the time), then a follow-up
test-coverage task, then — after the owner's own question redirected the investigation —
the `TilemapRenderer`/`TilemapWorldRenderer` blocker resolution, its own follow-up
pixel-verification test task (closing Phase 7 100%), Phase 10 (end-to-end example +
Doxygen/README/NOTICE.md pass, in parallel) closing the whole plan, three small,
individually-approved follow-up items, and finally a full member-level API audit (10
parallel forks covering every module) plus fixes for the 6 real gaps it found. See `git log`
for every batch's individual commits; this section covers the five most recent batches in
detail, condensing earlier
ones.

- **Full member-level API audit + 6 fixes (2026-07-14)**. After the file-existence-only
  completeness audit (below) found and fixed its one gap, the owner asked for a deeper
  pass: every public member (not just file/type existence) of every ported type checked
  against upstream, across the whole plan. 10 parallel forks, one per coherent module
  group, ~390 files total. Result: 4 of 10 groups (166 files) had zero issues; 6 real
  gaps found in the rest, all fixed:
  1. `ColorHelper::FromHex` — added the missing non-allocating overload, consolidated to
     a single `std::string_view` parameter (a separate `const std::string&` sibling
     overload made every string-literal call site ambiguous — reverted).
  2. `Matrix3x2::ToMatrix(float depth, Matrix&)` — added zero-arg
     `ToMatrix(Matrix&)`/`static ToMatrix(const Matrix3x2&, Matrix&)` overloads (can't be
     a default argument — `result` is the trailing, non-defaultable parameter).
  3. `Tweener::ActiveTweens` — added `getActiveTweensProperty()` (materialized
     `std::vector<Tween*>`) alongside the existing count-only property.
  4. **A real bug**: `XmlNodeExtensions.cpp`'s `ParseInvariant<T>()` silently returned
     0/false on malformed input instead of throwing — the same class of bug already
     fixed once this session in the sibling `XmlReaderExtensions.cpp`, but this second
     occurrence had gone uncaught (no test exercised the malformed-input path). Fixed
     with the same `System::Byte/UInt16/Int16/UInt32/Int32/Single/Double/Boolean/
     SByte::Parse` pattern; also fixed `GetBoolAttribute` accepting `"1"` (upstream
     doesn't) and the delimited-attribute readers silently leaving trailing elements at
     0 for too-few tokens (upstream throws).
  5. `ComponentManager::GetMapper(int componentTypeId)` — added (trivial direct lookup,
     no reflection obstacle, simply missed).
  6. `ParticleEffect::FromFile`/`FromStream` — added (stale deferral comment from Phase
     8, dependency landed, follow-up never happened).
  All 6 independently verified: clean rebuilds of both configs, full `ctest`
  (**2079/2079**, up from 2063 — 16 new regression tests, each individually re-run to
  confirm it exercises the fix). Full writeup in `plan.md`'s decisions log.

- **Three explicitly-approved optional follow-ups landed (2026-07-14, commits `e82c1ed`,
  `9beec74`)**, after the owner was asked which (if any) of the ideas left in section 8 they
  wanted: (1) the `GetBackBufferData`/`RenderTarget2D` sizing gotcha, precisely confirmed by
  reading `GraphicsDevice.cpp`'s exact branch (the no-`Rectangle` overload defaults to
  `backend_->GetViewportSize()`, not the bound render target's own size) and documented as a
  new `MISSING.md` entry; (2) a doc note on `SpriteSheetAnimationBuilder::IsPingPong`
  explaining it needs `IsLooping(true)` too; (3) a Doxygen prose-quality pass (a forked
  sub-agent, bounded to ~15-25 central public types rather than an unbounded rewrite) adding
  genuinely useful `@brief` rewrites, `@see` cross-references, and 10 compiler-verified
  `@code` examples across 21 header files — one real bug caught along the way (an example
  double-wrapped `TilemapSpriteBatchRenderer::Draw` in a redundant `SpriteBatch::Begin`/`End`
  pair). Doxygen warnings confirmed 0→0 (prose-only, no new gaps). Independently re-verified
  by the orchestrating session as always: clean `git status`/diff, genuinely clean rebuilds
  of both configs, full `ctest` (2042/2042), and a direct read of the actual doc-comment/
  `MISSING.md` diffs for quality, not just trusted.

- **Phase 10 landed, closing the entire plan (2026-07-14)**. Two forked sub-agents ran in
  parallel on disjoint files: (a) `examples/tiled_demo/` — a real end-to-end example loading
  a hand-authored Tiled TMX map (with a real external tileset/sprite BMP, not synthetic
  in-code data), rendering it via `TilemapSpriteBatchRenderer`, animating a player sprite via
  `AnimatedSprite`/`AnimationController` under scripted `KeyboardStateExtended` input, using
  `OrthographicCamera`, and running headlessly via the same real-`GraphicsDevice`+
  `RenderTarget2D`+`GetBackBufferData`-readback idiom `TilemapIntegrationTests.cpp`
  established; (b) a Doxygen pass (fixed 52 warning lines down to **zero**, across 20 files —
  mostly missing `@param` tags, plus 2 false-positive hex-color autolink warnings and 4
  `README.md` cross-reference warnings), a `README.md` rewrite (accurate module list, a
  compiled-and-verified `Tweener` usage snippet), and a `NOTICE.md` re-audit (found already
  accurate, no changes needed). Independently re-verified by the orchestrating session, not
  just trusted: clean `git status` (changes matched exactly what both forks claimed,
  `plan.md`/`NEXT.md`/`NOTICE.md`/`MISSING.md` untouched by either), genuinely clean
  `rm -rf build`+`rm -rf build-headers` rebuilds of both configs from scratch (zero
  warnings), full `ctest` re-run (**2042/2042 passing**, unchanged from before Phase 10 —
  expected, since this phase added an example and docs, not new library code), the example
  binary actually re-run by the orchestrating session (not just trusted) and its saved PNG
  frame visually inspected (confirmed: grass field, dirt road, player sprite, exactly as
  claimed), and the Doxygen zero-warning claim independently reproduced
  (`rm -rf docs/generated && doxygen Doxyfile` → exit 0, 0 warnings, 4882 HTML files
  generated).

- **`TilemapRenderer`/`TilemapWorldRenderer` blocker RESOLVED and ported (2026-07-14)**. The
  owner directly challenged the framing of the blocker (asking, in effect, whether
  `DefaultEffect` was even a real XNA class, and whether the real fix was to use XNA's actual
  `BasicEffect`) — investigating that question found the actual resolution: `DefaultEffect` is
  **not** an XNA class at all (it's MonoGame.Extended's own invention); the real XNA
  `BasicEffect` the owner meant **already exists in `cna`**, fully implemented and
  pixel-verified across all 3 backends (see `cna/docs/basiceffect-support.md`), including
  specifically the exact `TextureEnabled`+`VertexColorEnabled`+no-lighting /
  `VertexPositionColorTexture` stride-24 combination these two renderers need — a strict
  superset of what `DefaultEffect` ever provided. Ported via a forked sub-agent (~2000 lines
  C# → ~2200 lines C++ across `TilemapRenderer.hpp/.cpp`/`TilemapWorldRenderer.hpp/.cpp` +
  51 new tests). Independently verified by the orchestrating session (not just trusted): clean
  `git status` (only the 6 expected new files, `plan.md`/`NEXT.md`/`NOTICE.md` untouched by
  the fork), genuinely clean `rm -rf build`+`rm -rf build-headers` rebuilds of both configs
  (zero warnings), full `ctest` re-run (1986/1986 passing, up from 1935), and a line-by-line
  comparison of several core methods (`Update`, `BuildLayerModels`) against the actual
  upstream `.cs` source confirming exact logical fidelity including preserved comments (e.g.
  the "Tiled bottom-aligns all tiles" comment). Full technical writeup in `plan.md`'s Phase 7
  entry. Committed as `abfbb41`, pushed to `develop`.

- **Follow-up: `TilemapIntegrationTests.cs` pixel-verification suite ported (2026-07-14)**,
  closing Phase 7 out completely. Discovered during the above verification pass that neither
  the new tests nor the pre-existing `TilemapSpriteBatchRendererTests.cpp` covered upstream's
  dedicated pixel-verification file (28 tests asserting on actual rendered pixel colors, not
  just "doesn't throw") — `plan.md` already had this flagged as remaining Phase 7 work.
  Ported via a second forked sub-agent (28 upstream tests → 56 executed GoogleTest cases via
  `TEST_P`/`INSTANTIATE_TEST_SUITE_P` for the 4 flip-flag `[Theory]` methods). **Real
  CNA-vs-XNA API-shape gap found and worked around (not a production bug)**: a literal
  translation of upstream's `renderTarget.GetData(pixels)` failed every test — this port's
  `Texture2D::GetData` only returns a CPU-side mirror populated by `SetData` uploads, never
  touched by GPU rendering into a `RenderTarget2D`. Fixed using `cna`'s own established idiom,
  `GraphicsDevice::GetBackBufferData(...)` called *before* unbinding the render target —
  confirmed directly against `cna/examples/easygl_rt_roundtrip_test.cpp`'s own "Read RT1 pixel
  while FBO is still bound" comment, not just trusted. Once fixed, every pixel assertion
  passed with no loosened tolerances — confirms no rendering bug in `TilemapRenderer`/
  `TilemapSpriteBatchRenderer`/`BasicEffect` itself. Independently re-verified by the
  orchestrating session: clean `git status` (only the one new test file), genuinely clean
  `rm -rf build`+`build-headers` rebuilds (zero warnings), full `ctest` re-run
  (**2042/2042 passing**, up from 1986), the `GetBackBufferData` claim confirmed directly
  against `cna`'s own example source, and several ported test bodies spot-checked line-by-line
  against the upstream `.cs`. **This closes Phase 7 100% — only Phase 10 remains in the whole
  plan.**

- **`Tilemaps/Rendering/*`**: `RenderMode`, `TilemapRendererShared`,
  `TilemapSpriteBatchRenderer`, `TilemapWorldSpriteBatchRenderer` all landed.
  **`TilemapRenderer`/`TilemapWorldRenderer` deliberately NOT ported** — a genuine
  architectural blocker (`VertexPositionColorTexture`'s real field layout doesn't match
  `DefaultEffect`'s GLSL vertex shader layout at all: different attribute order *and* a
  different position component count), correctly identified and reported rather than
  guessed at or hacked around. See section 4 for the full writeup and the 3 real options.
- **Real gap-fill discovered and delivered as a prerequisite**: `OrthographicCamera` had
  been deferred since Phase 3 ("port immediately after `ViewportAdapters` lands") and never
  actually landed — nobody had noticed across 4 subsequent phases. Ported now (full
  `Camera<Vector2>`+`IMovable`+`IRotatable`, 51 tests) since every `Rendering/*` class needs
  it. See `plan.md`'s corrected Phase 1 entry.
- **Real bug found in `cna`** (sibling repo, not touched): `BoundingFrustum::Contains`
  wrongly reports `Intersects` for a point exactly on a clip plane (a real MonoGame
  binary-Disjoint/Contains check doesn't have this branch) — confirmed directly in
  `cna/src/.../BoundingFrustum.cpp`. Two tests hit this for real; `GTEST_SKIP()`-guarded
  with a full writeup in the test file rather than deleted or weakened. See section 5.
- **A significant, independently-verified correction to this file's own prior claims**: "no
  headless `SpriteBatch`/`GraphicsDevice` test infra exists," repeated in earlier entries of
  this very file, was an untested assumption, not a fact — `Rendering/*`'s own new tests
  construct a real `GraphicsDevice`/`SpriteBatch`/`Texture2D` and call real
  `Begin`/`Draw`/`End` (EasyGL-over-Mesa software rendering), and they pass.
- **Follow-up in the same session, closing out that discovery**: added real behavioral test
  coverage (47 tests, not just compile-checks) to the four modules that correction directly
  implicated — `ShapeExtensions` (21 tests, all 8 shape functions), `SpriteBatchExtensions`
  (9 tests, `NinePatch`/`Sprite`/`Texture2D`/`Texture2DRegion`), `BitmapFontExtensions` (9
  tests, all 6 `DrawString` overload pairs), `FadeTransition`/`ExpandTransition` (4 tests
  each, driven through real `Update()` calls to exercise `Draw()` past `Value == 0`).
  **Found a second real bug in the process**: `FadeTransition`/`ExpandTransition` (landed
  earlier this session) never overrode `System::Object::GetTypeName()` (pure virtual,
  confirmed via `grep` on `sharp-runtime`'s `Object.hpp`) — both classes had been genuinely
  **non-instantiable** the entire time since landing, invisible because no prior test had
  ever tried to construct one. Fixed by adding the override.
- Verified independently before landing both batches (as always): genuinely clean
  `rm -rf build` rebuilds — zero warnings; full `ctest` — **1935 tests run, 100% passing**
  (2 additional tests are the documented `GTEST_SKIP()` cases above; was 1804 before the
  Rendering batch, 1888 after it, 1935 after the test-coverage follow-up). Spot-checked the
  most consequential claims directly against source rather than trusted as reported: the
  `DefaultEffect`/`VertexPositionColorTexture` layout mismatch, the `cna` `BoundingFrustum`
  bug, the `OrthographicCamera` deferral history, and the `GetTypeName()` pure-virtual
  claim — all confirmed genuine by reading the actual source, not just the reports.

**Phase 7 is now ~95% done — only `TilemapRenderer`/`TilemapWorldRenderer` remain, and
they're blocked on a human decision (section 4).** This is the sole remaining blocker
anywhere in `plan.md` outside Phase 10.

For the earlier Phase 4-5, Phase 6/9, Tilemaps-core/Particles-bulk, Tiled/
ParticleEffectSerializer, and LDtk/Ogmo batches — including the process incident from a
Tilemaps-core sub-agent editing `plan.md` twice despite explicit correction — see `git log`
for individual commits and section 5 for the standing process-risk summary; not
re-detailed here.

---

## 4. Current blocker / main problem

**No blocker remains anywhere. The plan is complete — all 10 phases, including Phase 10.**
`cmake --build build -j$(nproc)`, `cmake --build build-headers -j$(nproc)`, and
`ctest --test-dir build` all currently succeed (**2063 tests run, 100% passing**, zero
warnings in both configs). The four historical blockers/gaps below are kept for context on
how they were resolved, not because any is still open.

### Resolved: `SizeJsonConverter.cs` — the one genuine gap found in a full completeness audit

- **Symptom**: after everything above was already marked complete, the project owner
  spot-checked for `Tweening/LinearOperations.cs` and didn't find it in the ported tree,
  worried the port might have systematic gaps.
- **Investigation**: `LinearOperations<T>` itself turned out to be a legitimate, documented
  elimination (C++ templates use `operator+`/`-`/`*` directly instead of upstream's
  runtime expression-tree compilation — see `LinearTween.hpp`'s header comment). But the
  underlying worry deserved a real answer: reconciled all 388 upstream `.cs` files
  (excluding `tests/`) against the ported tree, module by module, hand-checking every
  raw-file-count mismatch against `plan.md`'s decision log or the actual upstream source.
- **Result**: exactly one genuine, undocumented gap found —
  `Serialization/Json/SizeJsonConverter.cs` (the JSON converter for the integer `Size`
  type, distinct from the already-ported `Size2JsonConverter`/`SizeF`). Everything else
  that looked like a gap on a raw file-count basis was legitimate: documented exclusions
  or multi-file-to-one-file DTO consolidations (Tiled/LDtk/Ogmo). Full account in
  `plan.md`'s Phase 6 entry.
- **Fix**: ported `SizeJsonConverter.hpp`/`.cpp` following `Size2JsonConverter`'s exact
  established pattern, plus 4 fresh tests.
- **Owner review of all 14 audit findings, same day**: rather than trust the audit's
  conclusions, the owner asked to walk through every finding individually. Two
  corrections: (1) `ObservableCollection`/`IObservableCollection` — the owner overturned
  the original "reuse `sharp-runtime`'s equivalent" decision and asked for
  MonoGame.Extended's own type to be ported anyway, as a genuinely separate type (built
  on `sharp-runtime`'s `Collection<T>`, `System::Object`+`EventHandler` pattern matching
  `FramesPerSecondCounter.hpp`). (2) `Ray2.cs`→`Ray2D.hpp` — **this was actually WRONG**,
  caught during the walkthrough: `Ray2D.hpp` is a correct port of a *different* real
  upstream file (`Ray2D.cs`, field `Origin`); `Math/Ray2.cs` (field `Position`, one
  method, zero call sites, real dead code) is a genuinely separate type, now also
  ported per the owner's request. Full account in `plan.md`'s Phase 1 checklist entry.
  Both build configs verified clean, full suite **2063/2063** (was 2042).

### Resolved: Phase 10 (end-to-end example, Doxygen pass, README/NOTICE.md)

- **Scope**: the plan's final phase — not a blocker in the usual sense, just the last
  remaining work. See `plan.md`'s Phase 10 entry and section 3 above for full detail.
- **Resolution**: `examples/tiled_demo/` (a real, running, visually-verified end-to-end
  example), a Doxygen pass (52 warnings → 0), and a refreshed `README.md` (`NOTICE.md`
  needed no changes, confirmed already accurate). All independently re-verified by the
  orchestrating session.

### Resolved: `TilemapIntegrationTests.cs` pixel-verification test gap

- **Symptom (as of 2026-07-14, found during verification of the `DefaultEffect` blocker fix
  below)**: upstream's dedicated pixel-verification suite for `TilemapRenderer`/
  `TilemapSpriteBatchRenderer` (28 tests asserting on actual rendered pixel colors) was never
  ported — existing coverage for all 4 renderer classes was unit/validation-style only.
- **Resolution**: ported via a forked sub-agent, building a new headless render-target
  readback test harness modeled on `cna`'s own established idiom
  (`GraphicsDevice::GetBackBufferData` read before unbinding the render target — `cna`'s
  `Texture2D::GetData` alone doesn't see GPU-rendered content, only `SetData` uploads). Full
  detail in section 3 above and `plan.md`'s Phase 7 entry.

### Resolved: `TilemapRenderer`/`TilemapWorldRenderer` DefaultEffect blocker

- **Symptom (as of 2026-07-13)**: `Tilemaps/Rendering/TilemapRenderer.cs`/
  `TilemapWorldRenderer.cs` (the `VertexBuffer`/`IndexBuffer`-based tilemap renderers) were
  not ported — `VertexPositionColorTexture`'s real field layout didn't match this project's
  own hand-authored `DefaultEffect` GLSL vertex shader layout at all.
- **Resolution (2026-07-14)**: the owner directly questioned whether `DefaultEffect` was
  really an XNA 4.0 class needing a port fix, or whether the real fix was XNA's actual
  `BasicEffect`. Investigating: `DefaultEffect` is MonoGame.Extended's own invention, not an
  XNA class; `cna`'s real `BasicEffect` already exists, fully implemented and pixel-verified,
  and its feature set (texture + vertex color + optional lighting/fog/specular) is a strict
  superset of what `DefaultEffect` ever provided. Ported `TilemapRenderer`/
  `TilemapWorldRenderer` using `BasicEffect` (`TextureEnabled=true`, `VertexColorEnabled=true`,
  `LightingEnabled` left at its default `false`) instead of extending `DefaultEffect` or
  giving the renderers a custom vertex struct (none of the originally-listed 3 options were
  needed). Full technical detail in `plan.md`'s Phase 7 entry and section 3 above.
- **What's NOT affected**: everything else in `Rendering/*` (`TilemapSpriteBatchRenderer`,
  `TilemapWorldSpriteBatchRenderer`, `TilemapRendererShared`, `RenderMode`) was already
  landed, tested, and working, and needed no changes.

---

## 5. Known bugs and limitations

- **FIXED this session**: `cna-extended`'s own headers-only CMake build branch never added
  `sharp-runtime/vendor` to the include path, so any header pulling in a vendored
  third-party dependency (`tinyxml2/tinyxml2.h`) failed to compile in that config. Fixed in
  `CMakeLists.txt` by adding `target_include_directories(CNA_EXTENDED SYSTEM PUBLIC
  ${CNA_EXTENDED_SHARP_RUNTIME_DIR}/vendor)` alongside the existing `include` dir, matching
  the pattern `sharp-runtime`'s own CMakeLists.txt uses for its real `SHARP_RUNTIME` target.
- **CONFIRMED bug, in a sibling repo (`sharp-runtime`), not this repo**:
  `System::Xml::XmlNode::SelectSingleNode`'s doc comment
  (`include/System/Xml/XmlNode.hpp`) states "Caller takes ownership," but the
  implementation (`XmlNode.cpp` → `XPath::XmlDocumentNavigator::GetNode()`) returns a
  pointer into the live DOM tree, not a fresh allocation — verified by reading both files
  directly. `SelectNodes`' own doc comment on the same header correctly states the opposite
  ("individual nodes... remain owned by this document"), confirming the inconsistency.
  **Do not edit `sharp-runtime` to fix this** (sibling-repo rule) — `BitmapFontFileReader.cpp`
  in this repo already works around it by treating `SelectSingleNode`'s return as
  non-owning. Worth reporting to whoever maintains `sharp-runtime` at some point.
- **FIXED this session**: `Serialization/Xml/XmlReaderExtensions.cpp` used `std::stoi`/
  `std::stof` for numeric attribute parsing, which silently ignore trailing garbage
  (`std::stoi("1.5") == 1`, no exception) instead of requiring the whole string to be valid
  like C#'s `int.Parse`/`float.Parse`. Fixed with `System::Int32::Parse`/
  `System::Single::Parse` (already available in `sharp-runtime`, matching that exact
  strictness). Caught by the ported `GetAttributeRectangleInvalidFormatThrows` test failing
  for `"1.5,2,3,4"` during independent verification — same root-cause fix applied to
  `FloatStringConverter.cpp`'s `std::stof` (now `System::Single::TryParse`, matching
  upstream's `float.TryParse`-then-fallthrough structure).
- **NOT A BUG, but a fresh test was over-strict**: `HslColorJsonConverterTests`
  (fork-authored, no upstream equivalent) originally asserted exact round-trip fidelity
  through `HslColor::FromRgb`/`ToRgb`. That pre-existing, independently-tested conversion is
  not an exact bitwise inverse of itself for arbitrary RGB values (confirmed directly, no
  JSON involved: `Color(10,20,30,255)` round-trips to `(9,20,30,255)`, and a further round
  trip drifts again rather than stabilizing) — the test was corrected to an approximate
  comparison rather than "fixing" correct, already-tested production code to satisfy an
  incorrect test assumption.
- **CORRECTED AND CLOSED OUT this session — this entry was wrong in every earlier version
  of this file**: previously claimed "no headless `SpriteBatch`/`ISpriteBatchBackend` mock
  exists anywhere in this ecosystem," used to justify zero behavioral test coverage for
  `SpriteBatch.Extensions`, `BitmapFontExtensions`, `ShapeExtensions`,
  `FadeTransition`/`ExpandTransition::Draw`. **This was never actually verified — just
  assumed and repeated forward.** A plain `GraphicsDevice graphicsDevice;` + `SpriteBatch
  spriteBatch(graphicsDevice);` + `Texture2D(graphicsDevice, w, h)` genuinely render
  end-to-end via EasyGL-over-Mesa software rendering, no mock needed, no display needed.
  All four modules above now have real test coverage (47 new tests) as a direct follow-up —
  this also caught a real bug: `FadeTransition`/`ExpandTransition` had never overridden the
  pure-virtual `System::Object::GetTypeName()`, making both classes non-instantiable since
  they landed, invisible because nothing had ever tried to construct one before. Fixed.
  (Real-GPU-only example programs under `cna/examples/` still aren't part of the GoogleTest
  suite, and that's unrelated/unaffected by this correction.)
- **CONFIRMED bug, in a sibling repo (`cna`), not this repo**: `BoundingFrustum::Contains
  (const Vector3&, ContainmentType&)` (`cna/src/Microsoft/Xna/Framework/BoundingFrustum.cpp`)
  has an extra branch (`classifyPoint == 0.0f` → mark `intersects`) with no real-MonoGame
  equivalent — real MonoGame's version is a strict Disjoint-or-Contains binary check with no
  "on the plane" special case. This makes a point that lands exactly on a frustum clip plane
  wrongly report `Intersects` instead of `Contains`. **Do not edit `cna` to fix this**
  (sibling-repo rule). Two `OrthographicCameraTests` (`ContainsPoint_WithDefaultCamera_*`,
  `ContainsVector2_WithDefaultCamera_*`) hit this for real (points at Z=0, exactly on the far
  plane) and are `GTEST_SKIP()`-guarded with a full root-cause writeup inline, not deleted
  or weakened. Worth reporting to whoever maintains `cna` at some point.
- **PROCESS RISK** (not a code bug, but load-bearing context for delegating future work):
  three separate instances this session of a delegated sub-agent disregarding an explicit
  git/file-editing instruction. (1)-(2) Two sub-agents committed and pushed directly to
  `origin/develop` without authorization (see `git log` around commits in the
  `8d22baf`..`e4ffda7` range, and commit `bf61ade`'s message — the full prose writeup that
  used to live in this file is recoverable via `git log -- NEXT.md`). (3) The Tilemaps-core
  sub-agent edited `plan.md` twice despite explicit instruction, the second time immediately
  after acknowledging a mid-task correction not to (see section 3) — never committed, caught
  and discarded before it could be. Always independently check `git status`/`git diff`/
  `git log` — status and diff, not just log, since (3) never reached a commit — immediately
  after any delegated work completes (including resumed work, not just the first pass),
  before trusting or building on it. A sub-agent being right about code content does not
  mean it followed process instructions, and a sub-agent acknowledging a correction in text
  does not mean it followed that correction in practice. Phase 6 (Serialization) and Phase 9
  (ECS) remain the only two sub-agents this session with zero violations of any kind.

---

## 6. Architecture notes

- **Ownership conventions** (established across this phase, keep consistent going forward):
  `Texture2DRegion` is `std::shared_ptr`-managed (a genuinely multiply-aliased C# reference
  type — `Texture2DAtlas` indexes it twice, `Sprite`/`NinePatch`/`BitmapFontCharacter` hold
  it, `GetSubregion` hands out fresh derived ones). Externally-owned GPU resources
  (`Texture2D*`, `GraphicsDevice*`) stay non-owning raw pointers. `BitmapFont` owns its
  loaded page textures in `std::vector<std::unique_ptr<Texture2D>>` for pointer stability,
  since each character's region holds a non-owning `Texture2D*` into one of them (in
  upstream this ownership is implicit via GC; made explicit here).
- **JSON DTO pattern** (established with `Content/TexturePacker/*`): private fields, public
  get-only `getXProperty()` accessors, and a `friend`ed free function
  `void from_json(const nlohmann::ordered_json&, T&)` declared in the same namespace as
  `T` — this is nlohmann's ADL customization point, standing in for C# reflection /
  `[JsonPropertyName(...)]` (see `sharp-runtime`'s `System/Text/Json/JsonSerializer.hpp`
  header comment for why reflection itself is out of scope).
- **Binary format parsing pattern** (established with the BMFont `.fnt` binary reader):
  translate C#'s `[StructLayout(LayoutKind.Explicit)]`/`[FieldOffset(n)]` +
  `Marshal.PtrToStructure` into sequential, portable `BinaryReader` field reads in the exact
  declared-offset order — avoids C++ UB from reinterpreting raw bytes as a struct.
- **Data flow, Graphics/BitmapFonts area**: `Texture2DAtlas`/`Texture2DRegion` (Graphics) →
  `Sprite`/`SpriteSheet`/`AnimatedSprite` (Graphics, consumes `Animations::AnimationController`)
  and, separately, `BitmapFontCharacter` (BitmapFonts, holds a `Texture2DRegion` directly) →
  `BitmapFont` (owns character map + page textures) → `BitmapFontExtensions::DrawString`
  (consumes a live `SpriteBatch`).
- **Boundaries that must not be broken**:
  - Never edit `../cna`, `../sharp-runtime`, `../easy-3d`, or any other sibling repo under
    `/rv/data/development/github.com/openeggbert/` — read-only, even to fix a confirmed bug
    (see section 5's `SelectSingleNode` finding).
  - Never port anything from `MonoGame.Extended.Content.Pipeline` or its `.xnb`-reading
    classes (`Content/ContentReaders/*`, `Tilemaps/Content/*Reader`,
    `Content/ExtendedContentManager.cs`, `Content/ContentReaderExtensions.cs`).
  - `plan.md`'s phase order is dependency-ordered (Math/Collections → Collisions/Graphics →
    Tilemaps/Particles/ECS) — do not jump ahead to a later phase's module.
  - Zero-warning policy is a hard gate — no work that introduces a new warning should be
    considered mergeable.
- **CMake mechanics worth knowing**: source and test files are collected via
  `file(GLOB_RECURSE ... CONFIGURE_DEPENDS "src/*.cpp")` (and the equivalent for
  `tests/*.cpp`) — new files are auto-discovered on the next CMake configure step; no
  `CMakeLists.txt` edits are needed when adding a new ported file.

---

## 7. Useful commands

Configure + build, linked config (real CNA/EasyGL backend, needed for anything GPU-adjacent):
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCNA_EXTENDED_LINK_CNA=ON
cmake --build build -j$(nproc)
```

Configure + build, headers-only config (default):
```
cmake -S . -B build-headers -DCMAKE_BUILD_TYPE=Debug -DCNA_EXTENDED_LINK_CNA=OFF
cmake --build build-headers -j$(nproc)
```

Run all tests:
```
ctest --test-dir build
```

Run a filtered subset (example: BitmapFonts):
```
ctest --test-dir build -R "BitmapFont"
```

Run the minimal example binary:
```
./build/cna_extended_minimal
```

Run the GoogleTest binary directly (more verbose output than ctest):
```
./build/CnaExtendedTests
```

Check for uncommitted work:
```
git status
```

Lint/format: none configured in this repository (no `.clang-format` or `.clang-tidy` file
present) — rely on the `-Wall -Wextra -Werror` compiler gate instead.

---

## 8. Next smallest tasks

**The entire plan is complete — all 10 phases. There is no required next task.** The three
optional follow-up ideas this section used to list were all explicitly requested by the
project owner and completed on 2026-07-14 (Doxygen prose-quality pass on 21 central public
API types; the `GetBackBufferData`/`RenderTarget2D` sizing gotcha documented in
`MISSING.md`; the `IsPingPong`-needs-`IsLooping` gotcha documented on
`SpriteSheetAnimationBuilder::IsPingPong`) — see `git log` (commits `9beec74`, `e82c1ed`)
for details. Nothing below is blocking; this is an idea list for a future session, not a
checklist to work through.

**Standing preference, explicit from the project owner**: once a body of work is genuinely
complete and only optional/non-blocking ideas remain, do not autonomously pick one and start
— ask which one (if any) is wanted first. Only the general-maintenance item below is left
unaddressed from the original list; nothing new has been proposed since.

1. **General maintenance**: keep the reference clone at
   `/rv/data/library/github.com/craftworkgames/MonoGame.Extended` up to date
   (`/rv/data/library/github.com/github.sh`) if upstream MonoGame.Extended ever needs
   re-checking against a bugfix; keep `../cna`/`../sharp-runtime` in sync per their own
   `CLAUDE.md`s if either changes in a way that could affect this project (e.g. a future
   real `.mgfxo` bytecode loader landing in `cna`, which would make the
   `DefaultEffect`/`ShaderEffect` re-authoring notes throughout this codebase worth
   revisiting — not urgent, current behavior is correct and tested).

If a future session picks up this (or any new idea), verify with:
`cmake --build build -j$(nproc) && ctest --test-dir build` (linked config) plus
`cmake --build build-headers -j$(nproc)` (headers-only) — both must stay clean.

Delegating optional follow-ups to a sub-agent fork remains appropriate (user-approved this
session, standing safeguard: forks never commit/push/edit `plan.md`/`NEXT.md`/`NOTICE.md`,
orchestrator verifies then commits) — but see section 5's process-risk entry:
independently check `git status`/`git diff` (not just `git log`) after every fork turn,
including resumed ones, and be prepared for a "completed" notification to actually mean
"stopped partway through" rather than genuinely done.

---

## 9. Do not do yet

- No broad refactor of already-completed phases (0–6, 9).
- No renaming or restructuring the `getXProperty()` / namespace / file-layout conventions
  already established across ~65 ported files — they are intentional and load-bearing.
- No new third-party dependencies beyond GoogleTest without asking first.
- No porting anything from `MonoGame.Extended.Content.Pipeline` or any `.xnb`-reading class.
- No editing sibling repositories (`../cna`, `../sharp-runtime`, `../easy-3d`) — this
  includes the confirmed `sharp-runtime` `SelectSingleNode` doc-comment bug in section 5;
  report/ask, don't silently fix it there.
- No "fixing" pre-existing, independently-tested production code (like `HslColor`'s
  conversion math) just because a fresh test's assumption about it turns out to be wrong —
  verify which side is actually incorrect first (see section 5's HSL round-trip entry for
  exactly this situation this session).
- No skipping the ordered dependency chain within `plan.md` — confirmed this session that
  Phase 8 (Particles) had a real Serialization dependency (`ParticleEffectSerializer.cs`)
  despite `plan.md`'s dependency line originally not listing it; that file (and everything
  else in the plan) is now ported and complete — kept here as a precedent for how to handle
  a similar dependency-ordering surprise if `plan.md` is ever extended with new scope.
- Historical note, no longer active (kept for context): during the long unattended session
  hours): keep pushing verified work directly to `develop` after each task (owner's
  explicit choice), keep using sub-agent forks for large modules under the verify-then-
  commit-myself safeguard (also the owner's explicit choice) — do not re-ask either
  question. Do not use `AskUserQuestion` for anything else during this stretch either;
  mark anything that genuinely needs a human call as blocked/`needs_human` in this file
  instead and move to other independent work.
- No mass-reconstruction of this file's old chronological log format — the history is
  preserved in git, not duplicated here.

---

## 10. Resume prompt

The original porting plan (`plan.md`) is complete (all 10 phases) — nothing outstanding
there. **The active work is `plan3d.md`** (the `World3DEXT` 3D scene extension, owner-approved
2026-07-14): Phases 1-4 are done (camera; transform hierarchy bridge; model
rendering/frustum culling; skinned animation); resume at Phase 5 (`Collisions3DEXT`) —
read `3d.md` and `plan3d.md` §4 first. Re-check `plan3d.md`'s checkboxes and this file's
section 1 before assuming this is still current; something may have changed since this
was written.

If the project owner has a new, specific ask (a real request, not "continue the plan"), just
do that directly rather than inventing work from section 8's optional idea list. If asked to
"keep going" with no specific target, section 8 lists genuinely optional, non-blocking
follow-ups — pick at most one, verify it the same way every prior task in this file was
verified (independent `git status`/diff review of any fork's output before trusting it,
genuinely clean rebuilds of both CMake configs from scratch, full `ctest`, and — for
anything touching rendering/pixels — actually running the affected binary and inspecting
real output, not just trusting a report), and update this file and `plan.md` afterward.

The historical prompt this section used to contain (below, for reference only — do not
follow it literally, it refers to a "task 1" numbering scheme from an earlier, now-obsolete
state of section 8):

```
Read NEXT.md first, in full, before doing anything else.
Inspect only the files needed for task 1 in section 8 ("Next smallest tasks") — do not
open or modify unrelated files or modules.
Do not refactor anything outside the scope of that one task.
Make one small, verified improvement: complete task 1, and only task 1, unless it reveals
that task 1 is already done or invalid, in which case move to task 2 and say why you skipped
task 1.
Run the exact verification command listed for that task before considering it done.
After finishing, update NEXT.md: adjust section 2 (Current status), section 4 (blocker), and
section 8 (renumber/remove the completed task, keep the rest) to reflect the new state.
Do not touch the historical git log of this file or try to restore the old chronological
format.
```
