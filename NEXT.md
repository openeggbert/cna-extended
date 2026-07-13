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

**Current phase**: Phase 5 of 10 — "Graphics, BitmapFonts & Animations" — nearly complete.
Phases 0–4 are complete (Phase 4 is marked "mostly complete": two Screen transitions were
deliberately deferred into Phase 5, see section 8).

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
- **Build (headers-only/default config, `-DCNA_EXTENDED_LINK_CNA=OFF`)**: last verified
  clean at commit `685cf5c`. **Not yet re-verified** against the current working tree,
  which has additional uncommitted files since that commit (see below).
- **Tests**: **1316/1316 passing** (`ctest`, linked config), against the current working
  tree including uncommitted files.
- **Currently available build outputs**: `CNA_EXTENDED` static library target,
  `cna_extended_minimal` example executable, `CnaExtendedTests` GoogleTest binary.
- **Recently implemented** (landed and committed on `develop`): `Graphics/Effects/*`
  (hand-authored GLSL `DefaultEffect`/`MatrixChainEffect`, since CNA's bytecode-based
  `Effect` constructor is unimplemented), the Sprite/Texture2D/SpriteSheet/NinePatch
  cluster, the `Animations/*` module, `Content/TexturePacker/*` (direct-JSON atlas reader).
- **Implemented but NOT YET COMMITTED** (present in the working tree only): the
  `BitmapFonts` module — `Content/BitmapFonts/{BitmapFontFileContent,BitmapFontFileReader}`
  (direct BMFont `.fnt` parser: binary/text/XML variants) and runtime `BitmapFonts/*`
  (`BitmapFont`, `BitmapFontCharacter`, `BitmapFontExtensions`). Builds clean and its tests
  pass as part of the 1316 total above, but has not been committed, pushed, or checked off
  in `plan.md` yet.
- **Does not work / not done yet**:
  - `FadeTransition`/`ExpandTransition` (Screens) — blocked on `ShapeExtensions.cs`
    (`SpriteBatch::FillRectangle`), which is itself not yet ported.
  - Upstream test-directory ports for this phase are incomplete: `Texture2DAtlasTests.cs`
    is ported; `BitmapFontTests.cs`/`BitmapFontFileReaderTests.cs` are ported but
    uncommitted; `Animations/AnimationTests.cs` is **not yet ported**.
  - `Animations/*` has not yet had the deferred file-by-file member audit against upstream
    that the rest of this phase's modules received (only a spot-check was done when it
    landed).
  - No headless mock `SpriteBatch`/`ISpriteBatchBackend` test double exists anywhere in
    this ecosystem yet, so `SpriteBatch.Extensions`' `Draw`/`DrawString` methods (both in
    the Sprite cluster and in the new, uncommitted `BitmapFontExtensions`) remain
    call-compilable but behaviorally untested.

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
- Added (working tree only, **uncommitted**): `Content/BitmapFonts/*` and runtime
  `BitmapFonts/*` — see section 2. 10 net-new tests (1306 → 1316).
- Behavior note: `BitmapFontFileReader.cpp` treats `System::Xml::XmlNode::SelectSingleNode`'s
  return value as a non-owning pointer, deliberately not following that method's own doc
  comment (which says "caller takes ownership") — see section 5.

---

## 4. Current blocker / main problem

There is **no build-breaking or test-failing blocker** right now — the last full build and
test run were clean. The main open problem is a **verification and integration gap**, not a
bug:

- **Symptom**: the BitmapFonts module (5 header files, 5 source files, 2 test files, plus 4
  `.fnt` fixture files) exists only as untracked files in the working tree. It has not
  received the same level of independent line-by-line review that the binary `.fnt` block
  parser received (only that specific piece was checked in detail so far); the runtime
  `BitmapFont.cpp` logic (glyph enumeration iterators, `FromFile`/`FromStream` texture
  loading, UTF-8 codepoint decoding) has not yet been independently re-verified.
- **Failing command**: none — `cmake --build build -j$(nproc)` and `ctest --test-dir build`
  both currently succeed.
- **Affected files/modules**: `include/CNA/Extended/BitmapFonts/*`,
  `include/CNA/Extended/Content/BitmapFonts/*`, `src/CNA/Extended/BitmapFonts/*`,
  `src/CNA/Extended/Content/BitmapFonts/*`, `tests/CNA/Extended/BitmapFonts/*`.
- **Suspected cause**: this is process/sequencing, not a defect — the work was produced by
  a delegated sub-agent and intentionally left uncommitted pending independent review
  (standing project discipline after two prior incidents where delegated work was committed
  without authorization; see section 5).
- **What has already been tried / done**: `git status` confirmed nothing was committed by
  the sub-agent (compliant this time). The binary `.fnt` parser was checked field-by-field
  against upstream's `[FieldOffset]` struct layouts and matches exactly. A real bug was
  found and confirmed in `sharp-runtime`'s `XmlNode::SelectSingleNode` (see section 5) and
  worked around correctly without editing the sibling repo. A clean full rebuild (linked
  config) and full `ctest` run both passed. **Not yet done**: headers-only config rebuild;
  line-level review of `BitmapFont.cpp`'s runtime logic; commit/push; `plan.md` update.

---

## 5. Known bugs and limitations

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
  `ShapeExtensions.cs`/`SpriteBatch::FillRectangle`, itself not yet ported (see section 8,
  task 5).
- **INCOMPLETE**: `Animations/*` module has not had its deferred file-by-file member audit
  against upstream C# (only a spot-check was done at landing time).
- **INCOMPLETE**: `Animations/AnimationTests.cs` (upstream) not yet ported.
- **INCOMPLETE / architectural gap, not specific to this phase**: no headless
  `SpriteBatch`/`ISpriteBatchBackend` mock exists anywhere in this ecosystem, so every
  `SpriteBatch`-drawing extension method ported so far (`SpriteBatch.Extensions`,
  `BitmapFontExtensions`) compiles but has zero behavioral test coverage.
  Real-GPU-only example programs under `cna/examples/` exist but are not part of the
  GoogleTest suite.
- **NEEDS VERIFICATION**: headers-only CMake build config against the current working tree
  (including the uncommitted BitmapFonts files) — last verified clean at an earlier commit,
  not the current state.
- **NEEDS VERIFICATION**: `BitmapFont.cpp`'s runtime logic beyond the binary block parser
  (glyph enumeration, `FromFile`/`FromStream`, UTF-8 decoding) — only self-reported by the
  sub-agent that produced it, not yet independently line-checked against upstream the way
  the binary parser was.
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

Check for uncommitted work (currently the BitmapFonts module):
```
git status
```

Lint/format: none configured in this repository (no `.clang-format` or `.clang-tidy` file
present) — rely on the `-Wall -Wextra -Werror` compiler gate instead.

---

## 8. Next smallest tasks

1. **Re-verify the headers-only build against the current working tree.**
   - Goal: confirm the uncommitted BitmapFonts files don't break the headers-only config.
   - Files: none modified; verification only.
   - Command: `cmake -S . -B build-headers -DCMAKE_BUILD_TYPE=Debug -DCNA_EXTENDED_LINK_CNA=OFF && cmake --build build-headers -j$(nproc)` — expect exit 0, zero warnings.

2. **Line-review `BitmapFont.cpp`'s runtime logic against upstream `BitmapFont.cs`.**
   - Goal: give the glyph-enumeration iterators, `FromFile`/`FromStream`, and the UTF-8
     codepoint decoder (`DecodeUtf8CodePointAt`) the same fidelity check already done for
     the binary `.fnt` block parser.
   - Files: `src/CNA/Extended/BitmapFonts/BitmapFont.cpp`, upstream
     `MonoGame.Extended/BitmapFonts/BitmapFont.cs`.
   - Command: no code change expected if it checks out; if a fix is needed, re-run
     `ctest --test-dir build -R BitmapFont`.

3. **Commit, push, and check off the BitmapFonts work in `plan.md`.**
   - Goal: land the verified module on `develop` and update the plan's Phase 5 checklist.
   - Files: `include/CNA/Extended/BitmapFonts/*`, `include/CNA/Extended/Content/BitmapFonts/*`,
     `src/CNA/Extended/BitmapFonts/*`, `src/CNA/Extended/Content/BitmapFonts/*`,
     `tests/CNA/Extended/BitmapFonts/*`, `plan.md`.
   - Command: `git log --oneline -3` should show the new commit(s); `git push` should
     succeed; `ctest --test-dir build` should still be 100% passing afterward.

4. **Port `Animations/AnimationTests.cs` and audit `Animations/*` against upstream.**
   - Goal: close the two open items noted against the `Animations/*` module in `plan.md`.
   - Files: `tests/CNA/Extended/Animations/*`, upstream
     `tests/MonoGame.Extended.Tests/Animations/AnimationTests.cs`,
     `include/CNA/Extended/Animations/*`, `src/CNA/Extended/Animations/*`.
   - Command: `ctest --test-dir build -R Animation` — expect 100% passing.

5. **Port `Math/ShapeExtensions.cs` (Phase-5-scoped despite its upstream folder), then
   `FadeTransition`/`ExpandTransition`.**
   - Goal: unblock the two deferred Screen transitions, which need
     `SpriteBatch::FillRectangle`.
   - Files: new `include/src/CNA/Extended/Graphics/ShapeExtensions.*` (or wherever
     `plan.md` currently scopes it — check first), then
     `include/CNA/Extended/Screens/Transitions/{FadeTransition,ExpandTransition}.hpp`
     + matching `.cpp`, upstream `Screens/Transitions/{FadeTransition,ExpandTransition}.cs`.
   - Command: `ctest --test-dir build -R Transition` — expect 100% passing; both CMake
     configs stay clean.

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
- No committing or pushing the current BitmapFonts work until tasks 1–2 above are done.
- No skipping ahead to Phase 6+ (Serialization/Tilemaps/Particles/ECS) before Phase 5's
  remaining items are finished — the phase order in `plan.md` is dependency-ordered on
  purpose.
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
