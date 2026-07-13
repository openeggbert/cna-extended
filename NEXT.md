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

**Current phase**: Phases 0–6, 8, and 9 are complete. Phase 7 ("Tilemaps") is in progress:
the core data model and `Tiled/*` (TMX/JSON — priority, given the user's existing
`tiled-blupi` project) are done; `Rendering/*`, `LDtk/*`, `Ogmo/*` remain.

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
- **Tests**: **1761/1761 passing** (`ctest`, linked config).
- **Currently available build outputs**: `CNA_EXTENDED` static library target,
  `cna_extended_minimal` example executable, `CnaExtendedTests` GoogleTest binary.
- **Phases 0–6, 8, and 9 are complete.** Phase 7's core Tilemaps data model and `Tiled/*`
  just landed — see section 3.
- **Does not work / not done yet**:
  - No headless mock `SpriteBatch`/`ISpriteBatchBackend` test double exists anywhere in
    this ecosystem, so every `SpriteBatch`-drawing extension method ported so far
    (`SpriteBatch.Extensions`, `BitmapFontExtensions`, `ShapeExtensions`,
    `FadeTransition`/`ExpandTransition::Draw`) remains call-compilable but behaviorally
    untested. No upstream tests exist for any of these either — a standing architectural
    gap, not a regression. `Tilemaps/Rendering/*` (next up) will hit this same gap.
  - Phase 7: `Rendering/*`, `LDtk/*`, `Ogmo/*` — not started.

---

## 3. Recent changes

One long autonomous session (owner authorized, unavailable for hours). Order so far: Phases
4-5, then Phase 6 (Serialization) + Phase 9 (ECS) in parallel, then Phase 7's Tilemaps core
+ the bulk of Phase 8 (Particles) in parallel, then `Tilemaps/Tiled/*` + the deferred
`ParticleEffectSerializer.cs` in parallel (Phase 8 now fully complete). See `git log` for
every batch's individual commits; this section covers the most recent (Tiled +
ParticleEffectSerializer) batch in detail, condensing earlier ones.

- **`Tilemaps/Tiled/*`** (TMX/JSON parser, priority per `plan.md`): preserves upstream's
  raw-XML → document-model → `TilemapData` two-stage design; tile-layer decoding covers
  CSV, base64 uncompressed/gzip/zlib (zstd explicitly rejected, matching upstream), verified
  against Python's own `gzip`/`zlib` output. See `plan.md`'s Phase 7 entry for the two bugs
  found here (one in this port's own new code, self-fixed; one a genuine `sharp-runtime`
  naming collision, worked around not fixed).
- **`ParticleEffectSerializer.cs`** (completes Phase 8): no reflection anywhere upstream, as
  anticipated — pure manual type-name dispatch. Found and fixed a real, **pre-existing**
  bug from Phase 6 that had gone uncaught until now: `XmlWriterExtensions.cpp`'s float
  writers used `std::to_string`, always emitting fixed 6-decimal notation instead of C#'s
  shortest-round-trippable form — this would have made every `Serialize` path in this new
  file fail. See `plan.md`'s Phase 8 entry for detail.
- **Both sub-agents in this batch were fully process-compliant** — no commits, no pushes, no
  `plan.md`/`NEXT.md`/`NOTICE.md` edits. Notably including the fork that produced `Tiled/*`,
  despite having the prior batch's plan.md-editing incident (see below) visible in its own
  inherited context — a data point that the correction communicated to that prior fork (and
  now baked into every subsequent fork prompt) is holding, not that the problem is
  structural to every fork.
- Verified independently before landing (as always): genuinely clean `rm -rf build`
  rebuild — zero warnings, both linked and headers-only configs; full `ctest` —
  **1761/1761 passing** (was 1671 before this batch). Several concrete technical claims
  independently re-verified against upstream/evidence, not just taken on trust (dead-code
  skip claims via grep, the `sharp-runtime` naming-collision claim via direct header
  inspection, the ring-buffer and decode-logic structure via line-level upstream comparison).

For the earlier Phase 4-5, Phase 6/9, and Tilemaps-core/Particles-bulk batches — including
the process incident from a Tilemaps-core sub-agent editing `plan.md` twice despite explicit
correction — see `git log` for individual commits and section 5 for the standing
process-risk summary; not re-detailed here to keep this section focused on the current
batch.

---

## 4. Current blocker / main problem

**None.** No build-breaking or test-failing issue. `cmake --build build -j$(nproc)`,
`cmake --build build-headers -j$(nproc)`, and `ctest --test-dir build` all currently
succeed (1761/1761 tests, zero warnings in both configs). The next substantive work is
Phase 7's remaining sub-modules (`Rendering/*`, `LDtk/*`, `Ogmo/*`); see section 8. Phase 8
is now fully complete.

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

2. **Phase 7 — Tilemaps `LDtk/*` and `Ogmo/*`** (LDtk/Ogmo JSON document models +
   integration). Independent of each other and of the now-landed `Tiled/*` — good
   candidates for parallel sub-agents; `Tiled/*`'s just-landed
   `Converters/TiledTilemapDataConverter.cpp` is a template for how a format-parser's
   "document model → runtime `TilemapData`" stage should look.
   - Command: `ctest --test-dir build -R "LDtk|Ogmo"` — expect 100% passing; both CMake
     configs stay clean.

Delegating to sub-agent forks remains appropriate for the larger items above (user-approved
this session, standing safeguard: forks never commit/push/edit `plan.md`/`NEXT.md`/
`NOTICE.md`, orchestrator verifies then commits) — but see section 5's process-risk entry:
independently check `git status`/`git diff` (not just `git log`) after every fork turn,
including resumed ones, and be prepared for a "completed" notification to actually mean
"stopped partway through, including possibly mid-violation" rather than genuinely done.

3. **Phase 10 — Integration, polish, documentation**, once Phase 7 lands. Scope depends on
   what that phase actually produces — not detailed here yet.

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
