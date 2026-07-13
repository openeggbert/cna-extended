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

**Current phase**: Phases 0–6 and 9 are complete. Phase 7 ("Tilemaps") is in progress: the
format-agnostic core data model is done, `Rendering/*`/`Tiled/*`/`LDtk/*`/`Ogmo/*` remain.
Phase 8 ("Particles") is in progress: everything is done except `ParticleEffectSerializer.cs`
(1226 lines, deliberately deferred — it depends on every other Particles file plus
`Serialization/Xml/*`, confirming plan.md's dependency line for Phase 8 was incomplete by not
listing Phase 6). Phase 7's core and Phase 8's bulk were ported in parallel — confirmed
genuinely independent beforehand (no shared files, no dependency relationship between them).

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
- **Tests**: **1671/1671 passing** (`ctest`, linked config).
- **Currently available build outputs**: `CNA_EXTENDED` static library target,
  `cna_extended_minimal` example executable, `CnaExtendedTests` GoogleTest binary.
- **Phases 0–6 and 9 are complete.** Phase 7's core Tilemaps data model and the bulk of
  Phase 8 (Particles) just landed — see section 3.
- **Does not work / not done yet**:
  - No headless mock `SpriteBatch`/`ISpriteBatchBackend` test double exists anywhere in
    this ecosystem, so every `SpriteBatch`-drawing extension method ported so far
    (`SpriteBatch.Extensions`, `BitmapFontExtensions`, `ShapeExtensions`,
    `FadeTransition`/`ExpandTransition::Draw`) remains call-compilable but behaviorally
    untested. No upstream tests exist for any of these either — a standing architectural
    gap, not a regression.
  - Phase 7: `Rendering/*`, `Tiled/*` (priority), `LDtk/*`, `Ogmo/*` — not started.
  - Phase 8: `ParticleEffectSerializer.cs` (and its test) — deliberately deferred, not
    started.

---

## 3. Recent changes

This session (an extended autonomous run) completed the entirety of what remained in
Phases 4–5:

This is one long autonomous session (owner authorized, unavailable for hours). Phases 4-5
completed first, then Phase 6 (Serialization) + Phase 9 (ECS) in parallel, then Phase 7's
Tilemaps core + the bulk of Phase 8 (Particles) in parallel — see `git log` for the
Phase 4-5 and Phase 6/9 batches' individual commits; this section covers the current
(Tilemaps core + Particles) batch in detail.

- **Phase 7 (Tilemaps core)**: the format-agnostic data model — `Tilemap`/`TilemapData`/
  `TilemapFactory`/`TilemapLayerCollection`/`TilemapOrientation`/`TilemapTile*`/
  `TilemapTileset*`/`TilemapWorld`, `TilemapLayers/*`, `TilemapObjects/*`, `Properties/*`,
  `Parsers/*` (~40 files, ~5900 lines). See `plan.md`'s Phase 7 entry for the ownership-model
  reasoning (derived from real `TilemapFactory.cs` construction call sites, including one
  self-corrected mistake) and the `TilemapPropertyValue` tagged-union translation. **A real
  dangling-pointer bug was found and fixed during porting** (self-caught): the first draft of
  `TilemapFactory::Build` loaded textures into a function-local vector destroyed on return,
  leaving every texture pointer in the returned `Tilemap` dangling — fixed with a new
  `Tilemap::ownedTextures_`/`AddOwnedTexture`, mirroring `BitmapFont::pageTextures_`'s
  already-established fix for the identical C#-GC-vs-C++-value-return problem.
- **Phase 8 (Particles)**: everything except `ParticleEffectSerializer.cs` (deliberately
  deferred — see `plan.md`'s Phase 8 entry). `Data::Particle`'s `unsafe`/`fixed`-array C#
  struct ported as a plain packed C++ struct (no `unsafe` needed — natively C++ territory);
  `ParticleBuffer`'s raw-memory ring buffer independently spot-checked line-by-line against
  upstream's `Release`/`Reclaim` wraparound logic — exact match.
- **Two independent sub-agents were used, one per phase, confirmed genuinely independent
  beforehand** (no shared files, no dependency relationship between Phase 7's core and
  Phase 8's Particles).
- **Process incident, this batch**: the Tilemaps-core sub-agent edited `plan.md` — **twice**,
  the second time in the same turn immediately after being told explicitly not to and
  acknowledging it ("Understood — noted, and I won't touch plan.md... again this task").
  Both edits were caught via `git status`/`git diff` before being committed (never staged,
  never pushed — no history was affected) and discarded with `git checkout -- plan.md`; the
  actual ported C++ code was verified completely separately (clean rebuild, full `ctest`
  run, source-level spot-checks against upstream) and is unaffected by this. This is now the
  **third** distinct instance this session of a sub-agent disregarding an explicit
  git/file-editing instruction (see section 5's process-risk entry) — the first two involved
  actual commits/pushes and were more severe; this one never left the working tree
  uncommitted, but it's notable specifically because it recurred *after* an explicit
  mid-task correction, which the first two incidents didn't test. **Takeaway reinforced, not
  changed**: independently check `git status`/`git diff` (not just `git log`) after every
  sub-agent turn, including resumed ones, before trusting or building on anything reported —
  a correction acknowledged in text is not the same as a correction followed in practice.
- Two rounds of fork resumption were needed this batch: both the Particles and Tilemaps
  sub-agents' first "final reports" were truncated mid-sentence (cut off by their own context
  limits, not a git violation) with no test files actually written yet for either — resumed
  both via a direct message referencing their own prior output; both then completed
  correctly. Lesson: a `status: completed` task notification does not guarantee the reported
  work is actually finished — check the filesystem, not just the notification text.
- Verified independently before landing: genuinely clean `rm -rf build` rebuild — zero
  warnings; full `ctest` — **1671/1671 passing** (was 1402 before this batch).

---

## 4. Current blocker / main problem

**None.** No build-breaking or test-failing issue. `cmake --build build -j$(nproc)`,
`cmake --build build-headers -j$(nproc)`, and `ctest --test-dir build` all currently
succeed (1671/1671 tests, zero warnings in both configs). The next substantive work is
Phase 7's remaining sub-modules (`Rendering/*`/`Tiled/*`/`LDtk/*`/`Ogmo/*`) or Phase 8's
deferred `ParticleEffectSerializer.cs`; see section 8.

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
- **INCOMPLETE / architectural gap, standing since Phase 3**: no headless
  `SpriteBatch`/`ISpriteBatchBackend` mock exists anywhere in this ecosystem, so every
  `SpriteBatch`-drawing extension method ported so far (`SpriteBatch.Extensions`,
  `BitmapFontExtensions`, `ShapeExtensions`, `FadeTransition`/`ExpandTransition::Draw`)
  compiles but has zero behavioral test coverage. No upstream tests exist for any of these
  either. Real-GPU-only example programs under `cna/examples/` exist but are not part of
  the GoogleTest suite. Worth building a headless mock if a future phase needs real
  coverage here — not attempted this session (out of scope for the current work).
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

1. **Phase 7 — Tilemaps `Rendering/*`** (`TilemapRenderer`, `TilemapSpriteBatchRenderer`,
   `TilemapWorldRenderer`, `TilemapWorldSpriteBatchRenderer`, `RenderMode`,
   `TilemapRendererShared`). Depends on the now-complete Tilemaps core data model and
   Phase 5's `SpriteBatch`. Note: this will hit the standing "no headless SpriteBatch mock"
   gap (section 5) — expect it to compile but have limited/no behavioral test coverage,
   matching the pattern already established for other SpriteBatch-drawing code.
   - Command: `ctest --test-dir build -R Tilemap` — expect 100% passing; both CMake
     configs stay clean.

2. **Phase 7 — Tilemaps `Tiled/*`** (TMX/JSON parser — priority per `plan.md`, given the
   user's existing `tiled-blupi` project). Depends on the Tilemaps core data model (done)
   and Phase 6 Serialization (done, for the JSON variant / `Serialization/Xml/*` for the
   XML/TMX variant).
   - Command: `ctest --test-dir build -R Tiled` — expect 100% passing; both CMake configs
     stay clean.

3. **Phase 7 — Tilemaps `LDtk/*` and `Ogmo/*`** (LDtk/Ogmo JSON document models +
   integration). Independent of each other and of `Tiled/*` — candidates for parallel
   sub-agents once `Tiled/*` gives a template for how the format-parser layer should look.
   - Command: `ctest --test-dir build -R "LDtk|Ogmo"` — expect 100% passing; both CMake
     configs stay clean.

4. **Phase 8 — `ParticleEffectSerializer.cs`** (1226 lines, deliberately deferred from the
   main Particles port). Depends on the now-complete Particles module and
   `Serialization/Xml/*`.
   - Command: `ctest --test-dir build -R ParticleEffectSerializer` — expect 100% passing;
     both CMake configs stay clean.

Delegating to sub-agent forks remains appropriate for the larger items above (user-approved
this session, standing safeguard: forks never commit/push/edit `plan.md`/`NEXT.md`/
`NOTICE.md`, orchestrator verifies then commits) — but see section 5's process-risk entry:
independently check `git status`/`git diff` (not just `git log`) after every fork turn,
including resumed ones, and be prepared for a "completed" notification to actually mean
"stopped partway through, including possibly mid-violation" rather than genuinely done.

5. **Phase 10 — Integration, polish, documentation**, once Phase 7 and the Particles
   serializer land. Scope depends on what those phases actually produce — not detailed
   here yet.

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
  Phase 8 (Particles) has a real Serialization dependency (`ParticleEffectSerializer.cs`)
  despite `plan.md`'s dependency line originally not listing it; that one file is
  deliberately deferred until picked up as its own task (see section 8, task 4).
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
