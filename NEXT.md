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

**Current phase**: Phases 0–6, 8, and 9 are complete. Phase 7 ("Tilemaps") is ~95% done:
the core data model, `Tiled/*` (TMX/JSON — priority, given the user's existing
`tiled-blupi` project), `LDtk/*`, `Ogmo/*`, and most of `Rendering/*` are done.
**`TilemapRenderer`/`TilemapWorldRenderer` are `needs_human`-blocked** — see section 4.
This is the sole remaining blocker in the entire porting plan outside Phase 10.

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
- **Tests**: **1888/1888 tests run, 100% passing** (2 additional tests exist but are
  deliberately `GTEST_SKIP()`-guarded — see section 5's `cna` `BoundingFrustum` bug entry).
- **Currently available build outputs**: `CNA_EXTENDED` static library target,
  `cna_extended_minimal` example executable, `CnaExtendedTests` GoogleTest binary.
- **Phases 0–6, 8, and 9 are complete.** Phase 7's core Tilemaps data model, `Tiled/*`,
  `LDtk/*`, `Ogmo/*`, and most of `Rendering/*` all landed — see section 3.
- **CORRECTED this session, was wrong in earlier entries of this same file**: a real
  headless `GraphicsDevice`/`SpriteBatch`/`Texture2D` triple genuinely works end-to-end in
  this environment (confirmed by `Rendering/*`'s own new tests actually calling
  `Begin`/`Draw`/`End` against real EasyGL-over-Mesa software rendering and passing) — the
  "no headless SpriteBatch test infra" claim below was an untested assumption inherited
  forward through this file, not a verified fact. See section 5.
- **Does not work / not done yet**:
  - `TilemapRenderer`/`TilemapWorldRenderer` — genuine architectural blocker,
    `needs_human`. See section 4.
  - `SpriteBatch.Extensions`, `BitmapFontExtensions`, `ShapeExtensions`,
    `FadeTransition`/`ExpandTransition::Draw` still have zero *actual* behavioral test
    coverage today (not because it's impossible — see the correction above — just because
    nobody has gone back and added it yet). Worth a follow-up task; see section 8.

---

## 3. Recent changes

One long autonomous session (owner authorized, unavailable for hours). Order so far: Phases
4-5, then Phase 6 (Serialization) + Phase 9 (ECS) in parallel, then Phase 7's Tilemaps core
+ the bulk of Phase 8 (Particles) in parallel, then `Tilemaps/Tiled/*` +
`ParticleEffectSerializer.cs` in parallel (Phase 8 completed in full), then
`Tilemaps/LDtk/*` + `Tilemaps/Ogmo/*` in parallel, then `Tilemaps/Rendering/*` (this batch,
the last item in Phase 7). See `git log` for every batch's individual commits; this section
covers the Rendering batch in detail, condensing earlier ones.

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
  this very file, was an untested assumption, not a fact — this batch's own new tests
  construct a real `GraphicsDevice`/`SpriteBatch`/`Texture2D` and call real
  `Begin`/`Draw`/`End` (EasyGL-over-Mesa software rendering), and they pass. See section 5
  and section 8's follow-up task.
- Verified independently before landing (as always): genuinely clean `rm -rf build`
  rebuild — zero warnings; full `ctest` — **1888 tests run, 100% passing** (2 additional
  tests exist but are the documented `GTEST_SKIP()` cases above; was 1804 before this
  batch). Spot-checked the fork's three most consequential claims directly against
  source, not trusted as reported: the `DefaultEffect`/`VertexPositionColorTexture`
  layout mismatch (read both directly — genuinely incompatible), the `cna`
  `BoundingFrustum` bug (read the exact branch — genuinely present), and the
  `OrthographicCamera` deferral history (read `plan.md`'s own prior entry — genuinely
  never completed).

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

**One genuine architectural blocker, `needs_human`; nothing else.** No build-breaking or
test-failing issue — `cmake --build build -j$(nproc)`, `cmake --build build-headers
-j$(nproc)`, and `ctest --test-dir build` all currently succeed (1888 tests run, 100%
passing, zero warnings in both configs).

- **Symptom**: `Tilemaps/Rendering/TilemapRenderer.cs`/`TilemapWorldRenderer.cs` (the
  `VertexBuffer`/`IndexBuffer`-based tilemap renderers, as opposed to the already-landed
  `SpriteBatch`-based ones) are not ported.
- **Root cause**: they use `VertexPositionColorTexture`, whose real field layout in this
  project (`Vector3 Position`, `Color`, `Vector2 TextureCoordinate` — confirmed directly in
  `cna`) does not match `DefaultEffect`'s GLSL vertex shader layout (`vec2 aPos`@0, `vec2
  aTexCoord`@1, `vec4 aColor`@2 — confirmed directly in this project's own
  `DefaultEffect.cpp`): different attribute order *and* a different position component
  count (`vec2` vs `Vector3`). Using one with the other wouldn't just fail to compile — it
  would silently bind the wrong bytes to the wrong attributes.
- **What this needs**: a human decision between (at least) three real options, each with
  real tradeoffs:
  1. A new `Effect` whose GLSL layout matches `VertexPositionColorTexture` exactly (new
     code only, `DefaultEffect` untouched).
  2. Change `DefaultEffect`'s own shader layout to match `VertexPositionColorTexture`
     (risks regressing every already-shipped consumer of `DefaultEffect` — Sprite
     rendering, `NinePatch`, etc. — all already tested and working).
  3. Give `TilemapRenderer`/`TilemapWorldRenderer` their own vertex struct matching
     `DefaultEffect`'s existing layout instead of literally using
     `VertexPositionColorTexture` (deviates from upstream's exact type choice, but touches
     nothing already shipped).
  This is the same category of decision as the `Graphics/Effects/*` bytecode blocker
  resolved earlier this session via `AskUserQuestion` (see that phase's `plan.md` entry) —
  a real design fork, not a guessable implementation detail.
- **Why this wasn't escalated synchronously**: per the standing autonomous-session
  instruction ("mark `needs_human`, continue with other independent work" — not "always
  ask immediately"), and because no other Phase 7 work remained to fill the gap while
  waiting (this was the last item in the phase). Documented here and in `plan.md` instead,
  for the owner to decide whenever they're back.
- **What's NOT blocked by this**: everything else in `Rendering/*`
  (`TilemapSpriteBatchRenderer`, `TilemapWorldSpriteBatchRenderer`, `TilemapRendererShared`,
  `RenderMode`) is fully landed, tested, and working — this only affects the two
  `VertexBuffer`-based renderer variants specifically.

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
- **CORRECTED this session — this entry was wrong in every earlier version of this file**:
  previously claimed "no headless `SpriteBatch`/`ISpriteBatchBackend` mock exists anywhere
  in this ecosystem," used to justify zero behavioral test coverage for
  `SpriteBatch.Extensions`, `BitmapFontExtensions`, `ShapeExtensions`,
  `FadeTransition`/`ExpandTransition::Draw`. **This was never actually verified — just
  assumed and repeated forward.** `Tilemaps/Rendering/*`'s own tests this session construct
  a plain `GraphicsDevice graphicsDevice;` + `SpriteBatch spriteBatch(graphicsDevice);` +
  `Texture2D(graphicsDevice, w, h)` and call real `Begin`/`Draw`/`End` — genuinely
  rendering end-to-end via EasyGL-over-Mesa software rendering, no mock needed, no display
  needed — and all pass. **The remaining gap is not "impossible," it's "nobody has gone
  back to add the tests yet"** for the four modules listed above. See section 8 for this as
  a concrete follow-up task. (Real-GPU-only example programs under `cna/examples/` still
  aren't part of the GoogleTest suite, and that's unrelated/unaffected by this correction.)
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

1. **`TilemapRenderer`/`TilemapWorldRenderer` — needs a human decision first** (section 4).
   This is a `needs_human` blocker, not a task to just start on — read section 4's 3 options
   and get the project owner's call on which one before writing any code here. Once decided:
   - Command (after implementing): `ctest --test-dir build -R Tilemap` — expect 100%
     passing; both CMake configs stay clean.

2. **Add real behavioral test coverage for the "compiles but untested" `SpriteBatch`-drawing
   modules** (`SpriteBatch.Extensions`, `BitmapFontExtensions`, `ShapeExtensions`,
   `FadeTransition`/`ExpandTransition::Draw`) — now genuinely possible per section 5's
   correction (a plain `GraphicsDevice`/`SpriteBatch`/`Texture2D` triple already works
   headlessly in this environment; `Tilemaps/Rendering/*`'s own tests are the working
   template to copy). Independent of item 1 — safe to do while waiting on that decision.
   No production code changes expected, only new test files.
   - Command: `ctest --test-dir build` — expect 100% passing (net new tests only, no
     regressions); both CMake configs stay clean.

3. **Phase 10 — Integration, polish, documentation**, once item 1 lands (or is explicitly
   deferred by the owner). Scope depends on what's actually left to polish at that point —
   not detailed here yet. Likely candidates worth considering when scoping it: a real
   `README.md` (mentioned in `CLAUDE.md`'s own "read first" list but not yet confirmed to
   exist), a pass over every `NOTICE.md`-flagged licensing note for completeness, and
   whether item 2 above should be folded into Phase 10 or done before it.

Delegating to a sub-agent fork remains appropriate for items 1-2 (user-approved this
session, standing safeguard: forks never commit/push/edit `plan.md`/`NEXT.md`/`NOTICE.md`,
orchestrator verifies then commits) — but see section 5's process-risk entry: independently
check `git status`/`git diff` (not just `git log`) after every fork turn, including resumed
ones, and be prepared for a "completed" notification to actually mean "stopped partway
through" rather than genuinely done.

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
