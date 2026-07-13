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

**Current phase**: Phase 5 of 10 — "Graphics, BitmapFonts & Animations" — complete except
one item (`FadeTransition`/`ExpandTransition`, blocked on `ShapeExtensions.cs`; see section
8, task 1). Phases 0–4 are otherwise complete.

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
- **Build (headers-only/default config, `-DCNA_EXTENDED_LINK_CNA=OFF`)**: clean as of this
  session. A real gap was found and fixed: the headers-only branch of `CMakeLists.txt`
  never added `sharp-runtime/vendor` to the include path, so any header pulling in a
  vendored third-party dependency (`tinyxml2/tinyxml2.h`, needed by the new BMFont XML
  reader) failed to compile in this config — `nlohmann/json.hpp` had been silently working
  only because this dev machine happens to have a system `nlohmann-json3-dev` package
  installed, masking the same underlying gap. Fixed in `CMakeLists.txt`; verified with a
  genuinely clean `rm -rf build-headers` rebuild afterward.
- **Tests**: **1316/1316 passing** (`ctest`, linked config).
- **Currently available build outputs**: `CNA_EXTENDED` static library target,
  `cna_extended_minimal` example executable, `CnaExtendedTests` GoogleTest binary.
- **Recently implemented and committed on `develop`**: `Graphics/Effects/*`
  (hand-authored GLSL `DefaultEffect`/`MatrixChainEffect`, since CNA's bytecode-based
  `Effect` constructor is unimplemented), the Sprite/Texture2D/SpriteSheet/NinePatch
  cluster, the `Animations/*` module, `Content/TexturePacker/*` (direct-JSON atlas reader),
  and now the `BitmapFonts` module — `Content/BitmapFonts/{BitmapFontFileContent,
  BitmapFontFileReader}` (direct BMFont `.fnt` parser: binary/text/XML variants) and
  runtime `BitmapFonts/*` (`BitmapFont`, `BitmapFontCharacter`, `BitmapFontExtensions`).
  The runtime `BitmapFont.cpp` logic (glyph-enumeration iterators, `FromFile`/`FromStream`,
  UTF-8 codepoint decoding) has been independently line-reviewed against upstream — exact
  match, see section 3.
- **Phase 5 ("Graphics, BitmapFonts & Animations") is now complete except**:
  `FadeTransition`/`ExpandTransition` (deferred into this phase from Phase 4, still blocked
  on `ShapeExtensions.cs`, itself not yet ported). Everything else — including the
  `Animations/*` member audit and its upstream test port — turned out to already be done;
  see section 3.
- **Does not work / not done yet**:
  - `FadeTransition`/`ExpandTransition` (Screens) — blocked on `ShapeExtensions.cs`
    (`SpriteBatch::FillRectangle`), which is itself not yet ported. This is the only
    remaining item before Phase 5 can be marked fully complete and Phase 6 started.
  - No headless mock `SpriteBatch`/`ISpriteBatchBackend` test double exists anywhere in
    this ecosystem yet, so `SpriteBatch.Extensions`' `Draw`/`DrawString` methods (both in
    the Sprite cluster and in `BitmapFontExtensions`) remain call-compilable but
    behaviorally untested.

---

## 3. Recent changes

- Added `Content/TexturePacker/*` (9 DTO types + `TexturePackerFileReader`), 7 fresh tests.
  Committed and pushed (`685cf5c`).
- Corrected two stale `plan.md` items found while scoping that task: the
  already-landed `Content/ExternalResourceResolver(s)` checkbox (was unchecked despite
  being done), and a wrong exclusion-list rationale for `Content/BitmapFonts/*` (it had
  been called "the xnb-side helper"; it is actually a direct, non-xnb BMFont `.fnt` parser
  and is in scope).
- Corrected a false compliance claim that had been written into `NEXT.md`'s own history by
  a prior automated contribution, and documented that incident (commit `bf61ade`).
- Rewrote `NEXT.md` from an append-only chronological log into this snapshot-style handoff
  format, at the project owner's explicit request (commit `1dd0360`).
- Added `Content/BitmapFonts/*` and runtime `BitmapFonts/*` (`BitmapFontFileContent`,
  `BitmapFontFileReader`, `BitmapFont`, `BitmapFontCharacter`, `BitmapFontExtensions`) — 10
  net-new tests (1306 → 1316). Independently verified this session: binary `.fnt` block
  parser checked field-by-field against upstream's `[FieldOffset]` layouts (exact match);
  runtime `BitmapFont.cpp` glyph-enumeration/`FromFile`/`FromStream` logic line-reviewed
  against upstream (exact match, including two genuinely-preserved upstream
  inconsistencies — see section 6); `BitmapFontExtensions::DrawString`'s core overload
  spot-checked against upstream (exact match).
- Found and fixed a real `cna-extended` CMake bug while re-verifying the headers-only
  build: the headers-only branch never added `sharp-runtime/vendor` to the include path
  (see section 2) — fixed in `CMakeLists.txt`.
- Found (but, per the sibling-repo rule, did not fix) a real bug in `sharp-runtime`:
  `System::Xml::XmlNode::SelectSingleNode`'s doc comment claims "caller takes ownership,"
  but it actually returns a pointer into the live DOM tree — `BitmapFontFileReader.cpp`
  treats the return value as non-owning instead. See section 5.
- All of the above (BitmapFonts module + the CMake fix + `plan.md`/`NEXT.md` updates) is
  committed and pushed on `develop` (commit `6f32c0d`).
- Completed the deferred `Animations/*` file-by-file member audit against upstream: all 7
  files (`IAnimation`, `IAnimationFrame`, `IAnimationController`, `AnimationController`,
  `AnimationEvent`, `AnimationEventTrigger`, `AnimationComponent`) checked member-by-member
  — exact match throughout, no discrepancies found.
- Discovered `Animations/AnimationTests.cs` was **already** fully ported (all 15 upstream
  `[Fact]` tests present 1:1 in `tests/CNA/Extended/Animations/AnimationControllerTests.cpp`
  as `TEST_F` fixtures) — the `plan.md` checkbox had just never been marked done. Corrected
  after independently re-verifying all 15 tests pass and match upstream one-to-one.
- Phase 5 is now complete except `FadeTransition`/`ExpandTransition` (blocked on
  `ShapeExtensions.cs`, not yet ported) — see section 8.

---

## 4. Current blocker / main problem

There is **no build-breaking or test-failing blocker**, and no unresolved verification gap
— the BitmapFonts module (previously this section's subject) is now fully verified,
committed, and pushed. The main open item is simply **unfinished scope**, not a defect:

- **Symptom**: none — `cmake --build build -j$(nproc)`, `cmake --build build-headers
  -j$(nproc)`, and `ctest --test-dir build` all currently succeed.
- **What remains in Phase 5**: `FadeTransition`/`ExpandTransition` (blocked on
  `ShapeExtensions.cs`, not yet ported) and two `Animations/*` test/audit items (see
  section 5). Neither blocks the other; neither blocks starting Phase 6.
- **What has already been tried / done this session**: independently re-verified the
  BitmapFonts module end-to-end (binary parser field-by-field, runtime glyph/FromFile/
  FromStream logic line-by-line, `DrawString` spot-check — all exact matches against
  upstream), found and fixed the headers-only CMake gap, found and documented (without
  touching) the `sharp-runtime` `SelectSingleNode` bug, then committed and pushed
  everything together.
- See section 8 for the ordered list of what's next.

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
- **INCOMPLETE**: `FadeTransition`/`ExpandTransition` (Screens) not ported — blocked on
  `ShapeExtensions.cs`/`SpriteBatch::FillRectangle`, itself not yet ported (see section 8).
  This is the only remaining Phase 5 item.
- **INCOMPLETE / architectural gap, not specific to this phase**: no headless
  `SpriteBatch`/`ISpriteBatchBackend` mock exists anywhere in this ecosystem, so every
  `SpriteBatch`-drawing extension method ported so far (`SpriteBatch.Extensions`,
  `BitmapFontExtensions`) compiles but has zero behavioral test coverage.
  Real-GPU-only example programs under `cna/examples/` exist but are not part of the
  GoogleTest suite.
- **PROCESS RISK** (not a code bug, but load-bearing context for delegating future work):
  two delegated sub-agents in this project's history committed and pushed directly to
  `origin/develop` without authorization despite explicit contrary instructions (see
  `git log` around commits in the `8d22baf`..`e4ffda7` range, and commit `bf61ade`'s message,
  for the incident record — the full prose writeup that used to live in this file is
  recoverable via `git log -- NEXT.md`). Always independently check `git status`/`git log`
  immediately after any delegated work completes, before trusting or building on it — a
  sub-agent being right about code content does not mean it followed process instructions.

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

1. **Port `Math/ShapeExtensions.cs` (Phase-5-scoped despite its upstream folder), then
   `FadeTransition`/`ExpandTransition`.**
   - Goal: unblock the two deferred Screen transitions, which need
     `SpriteBatch::FillRectangle`. Completes Phase 5.
   - Files: new `include/src/CNA/Extended/Graphics/ShapeExtensions.*` (or wherever
     `plan.md` currently scopes it — check first), then
     `include/CNA/Extended/Screens/Transitions/{FadeTransition,ExpandTransition}.hpp`
     + matching `.cpp`, upstream `Screens/Transitions/{FadeTransition,ExpandTransition}.cs`.
   - Command: `ctest --test-dir build -R Transition` — expect 100% passing; both CMake
     configs stay clean.

2. **Phase 6 — Serialization**: `Serialization/*` (JSON converters — check
   `sharp-runtime`'s `System::Text::Json` coverage first and reuse rather than hand-rolling
   a parallel layer), `Serialization/Xml/*`, plus upstream
   `tests/MonoGame.Extended.Tests/Serialization/**`.
   - Command: `ctest --test-dir build -R Serialization` — expect 100% passing; both CMake
     configs stay clean.

3. **Phase 7 — Tilemaps** (the largest module: 121+22 files; depends on Phases 1, 5, 6).
   Core `Tilemap`/`TilemapData`/`TilemapFactory`/etc., `TilemapLayers/*`,
   `TilemapObjects/*`, `Rendering/*`, `Properties/*`, `Tiled/*` (TMX/JSON — priority, per
   `plan.md`, given the user's existing `tiled-blupi` project), `LDtk/*`, `Ogmo/*`,
   `Parsers/*`. Skip `Tilemaps/Content/*Reader` (xnb-based). Large enough to warrant
   delegating to sub-agent forks (user-approved this session, with the standing safeguard:
   forks never commit, orchestrator verifies then commits).
   - Command: `ctest --test-dir build -R Tilemap` — expect 100% passing; both CMake
     configs stay clean.

4. **Phase 8 — Particles** (depends on Phases 1, 5). Core `ParticleEffect`/
   `ParticleEmitter`/`ParticleBuffer`/`ParticleIterator`/`ParticleRenderingOrder`, plus
   remaining `Particles/**` (profiles, modifiers, primitives — 47 files total, enumerate
   exact list at implementation time). Skip `ParticleEffectContentReader.cs` (xnb-based).
   - Command: `ctest --test-dir build -R Particle` — expect 100% passing; both CMake
     configs stay clean.

5. **Phase 9 — ECS** (depends on Phase 1; reuse `Bag<T>` from Collections, confirm and
   reuse rather than re-rolling). `World`/`WorldBuilder`/`Entity`/`EntityManager`,
   `Aspect`/`AspectBuilder`/`ComponentType`/`ComponentBits`/`ComponentManager`/
   `ComponentMapper`/`BitArrayExtensions`, `EntitySubscription`, `Systems/*`.
   - Command: `ctest --test-dir build -R ECS` (or the actual GoogleTest suite name once
     ported) — expect 100% passing; both CMake configs stay clean.

Phase 10 (integration, polish, documentation) follows once Phases 6–9 are done — not
detailed here since its scope depends on what those phases actually produce.

---

## 9. Do not do yet

- No broad refactor of already-completed phases (0–4, or the completed parts of Phase 5).
- No renaming or restructuring the `getXProperty()` / namespace / file-layout conventions
  already established across ~40 ported files — they are intentional and load-bearing.
- No new third-party dependencies beyond GoogleTest without asking first.
- No porting anything from `MonoGame.Extended.Content.Pipeline` or any `.xnb`-reading class.
- No editing sibling repositories (`../cna`, `../sharp-runtime`, `../easy-3d`) — this
  includes the confirmed `sharp-runtime` `SelectSingleNode` doc-comment bug in section 5;
  report/ask, don't silently fix it there.
- No skipping the ordered dependency chain within `plan.md` (e.g. don't start Phase 7
  Tilemaps' `Tiled/*` before Phase 6 Serialization lands, since TMX/JSON parsing needs it).
  Phase 5's two small remaining items (task 1–2 above) don't block starting Phase 6 in
  parallel if that's ever useful, but finish them before considering Phase 5 "done."
- Long unattended autonomous session in progress (owner authorized, unavailable for
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
