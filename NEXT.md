# NEXT.md — session handoff log

This file is the short-term continuity document for `cna-extended`. Read it first in any
new session before touching code or `plan.md`. Append a new dated entry at the top after
every session with material progress; do not silently overwrite prior entries.

---

## 2026-07-13 (2) — Color helpers ported (Phase 1 task 6); user corrected the check-in cadence

The user asked "proč jsi se zastavil a autonomně nepokracoval" (why did you stop instead of
continuing autonomously) after the previous check-in. Correction applied: stop pausing to
report progress between tasks — that's not the same as being blocked, and it was making the
user wait/prompt "pokracuj" each time despite having explicitly set up this session for
autonomous, unattended operation. **From here on: keep working through `plan.md`'s task
list without stopping for status updates. Only stop for a genuine blocker** — something
that needs the user's judgment (like the `MulticastAction` extension question earlier this
session), not "a task finished." If resuming this session, keep applying that correction.

**Ported directly (~620 lines, no fork needed)**: `ColorExtensions` (the `ToHex` extension
method → free function), `ColorHelper` (`FromHex`/`FromName`/`FromAbgr`), `HslColor` (full
HSL color type with RGB conversion). Two dependency resolutions, both handled without
deferring anything:
- `ColorHelper`'s name→`Color` lookup table is built via C# reflection upstream
  (`typeof(Color).GetRuntimeProperties()`) — no C++ equivalent, so it's a hand-written
  table instead. Script-generated from CNA's `Color.hpp` (`grep -oP` for all
  `static const Color X` declarations) to get all 141 entries correct rather than
  transcribing by hand — cross-check tooling like this is worth reaching for whenever a
  port needs a complete enumeration of something.
- `HslColor::ToRgb` references `MathExtended.MachineEpsilon`, which isn't ported yet
  (next task). Checked its actual value first (`1.19209290e-7f` — exactly the standard
  IEEE-754 float epsilon) before deciding: this is a trivial constant
  (`std::numeric_limits<float>::epsilon()`), not an algorithm, so used the standard-library
  equivalent directly rather than deferring or duplicating a magic number. Same judgment
  call category as `Collision2D::Epsilon` earlier, just resolved even more cleanly since a
  real standard-library equivalent existed this time.

**Real bug the build caught**: `ColorHelper::FromAbgr` tried to construct a `Color` from a
packed `uint32`, matching upstream — but CNA's `Color(UInt32)` constructor is **private**
(unlike upstream's public one). Reworked to decompose the packed value into R/G/B/A ints
and use the public 4-int constructor instead; same resulting color. Also caught and fixed a
genuine test bug of my own: my first draft of the `FromAbgr` test had R and B swapped
(computed `rgba` packing order backwards) — worth remembering that a wrong *test* is just
as real a bug as a wrong *implementation*, and building+running is what caught it, not
inspection.

**Test coverage**: unlike the last three tasks, all upstream tests were portable this time
(no `Collision2D` dependency) — ported `ColorExtensionsTests.cs`/`ColorHelperTests.cs`/
`HslColorTests.cs` **1:1**, using GoogleTest `TEST_P`/`INSTANTIATE_TEST_SUITE_P` for the
xUnit `[Theory]`/`[InlineData]` cases (first use of parameterized tests in this project;
matches xUnit's per-row reporting granularity better than folding rows into one `TEST`
with a loop). One upstream sub-test (`AreEqualObjectMethod.WhenGivenObjectOfAntotherType_
ReturnsFalse`, comparing against a boxed `DateTime` via the object-typed `Equals`) has no
C++ equivalent and was skipped, matching the established `object obj`-overload precedent.

**Verification**: both build modes clean, `ctest` → **100% passed, 180/180** (was 138
before this task).

**State / next step:** Phase 1 is 6 of ~20 tasks in. Next per `plan.md` §5 Phase 1:
`MathExtended`, `FloatHelper`, `Angle`. Keep applying the established workflow (check real
C# dependencies first, fork only for genuinely large reads ~300+ lines, independently
verify a fork's `Equals`/`GetHashCode` claims via `grep`, build+test both modes before
every commit, one `plan.md` task = one commit) — and, per the correction above, keep going
through the list without pausing to check in.

---

## 2026-07-13 (1) — Camera<T> ported, OrthographicCamera deferred to Phase 3 (Phase 1 task 5)

Continued from session (5) after another "pokracuj". Small enough (144-line `Camera.cs`)
to port directly without a fork this time; `OrthographicCamera.cs` (512 lines) turned out
to need a full deferral, not a partial one.

**`Camera<T>` fully ported** (`include/CNA/Extended/Camera.hpp`, header-only — it's fully
abstract, no `.cpp` needed): all members pure virtual, templated on position type
(`Vector2` for 2D, `Vector3` for 3D, matching upstream's generic `Camera<T>`).
`getBoundingRectangleProperty()` returns `RectangleF`, forward-declared (not yet ported —
a pure virtual declaration doesn't need the complete type). No upstream tests exist for
this abstract type (nothing instantiates `Camera<T>` directly upstream either — only
`OrthographicCamera` does). Added a compile-only smoke test
(`tests/CNA/Extended/CameraTests.cpp`); **real instantiation-based tests are deferred
until `RectangleF` lands** — a concrete override of `getBoundingRectangleProperty()`
needs a complete `RectangleF` to construct/return one, so no concrete `Camera<T>`
subclass can exist yet, even for testing purposes.

**`OrthographicCamera` deferred in full to Phase 3** — this is a different situation from
every previous deferral this session (`Transform2`, the bounding-volume
`Contains`/`Intersects`, `LineSegment2D`'s distance methods), which were all narrow: port
everything else in the type/file, defer just the blocked members. `OrthographicCamera`
stores a `ViewportAdapter` as a **required** field, takes one as a **required**
constructor parameter, and calls into it from multiple methods throughout the class —
not a couple of peripheral helpers. `ViewportAdapters` is a whole separate module
scheduled for **Phase 3** ("Input, Timers, Tweening, ViewportAdapters, VectorDraw"), not
this phase. There is no meaningful partial port here; the whole type waits. Recorded in
`plan.md`'s Phase 1 checklist with this reasoning, so a future session doesn't
mis-scope it as "just forward-declare `ViewportAdapter` and defer a couple of methods"
the way `Transform2` was handled — that pattern doesn't fit here.

**Verification:** `cmake --build build -j"$(nproc)"` clean (both `-DCNA_EXTENDED_LINK_CNA=ON`
and headers-only), `ctest` → **100% passed, 138/138** (was 137 before this task — only
+1 since `Camera<T>` only got a compile-smoke test, not real coverage yet).

**State / next step:** Phase 1 is 5 of ~20 tasks in (task 5, "Camera + OrthographicCamera",
is really only half-done — `Camera<T>` shipped, `OrthographicCamera` is a Phase 3 item
now, tracked separately in `plan.md`). Next per `plan.md` §5 Phase 1: color helpers
(`ColorExtensions`, `ColorHelper`, `HslColor`). Keep checking each new task's actual C#
dependencies before starting — this is now the 4th task in a row that turned up an
ordering surprise not visible from `plan.md`'s flat list (`Transform2`→`Matrix3x2`,
`ISizable`/`IRectangularF`→`SizeF`/`RectangleF`, `BoundingCapsule2D`→`LineSegment2D`→
(really)→`Collision2D`, `OrthographicCamera`→`ViewportAdapters`/Phase 3). This is a
structural property of MonoGame.Extended's codebase (it's not layered as cleanly as
`plan.md`'s phase grouping implies), not bad luck — keep budgeting time for it on every
remaining task, not just the first few.

---

## 2026-07-12 (5) — Line2D, LineSegment2D, Ray2D ported (Phase 1 task 4), fork lesson applied

Continued from session (4) after another "pokracuj" (continue). Same forked-sub-agent
pattern (~2166 lines of C# across the 3 files), but this time explicitly told the fork
about the `GetHashCode` omission from the previous task and asked it to self-check an
`Equals`/`GetHashCode`/`ToString`/operators/`Deconstruct` checklist per type before
reporting back. Independently re-verified with `grep -n "public "` against the 3 upstream
`.cs` files anyway (per the standing rule from session (4): don't just trust a fork's
self-report) — this time the self-check held up, `GetHashCode` is present in all 3 types.

**Ported**: all fields, constructors, factory methods, self-contained geometry
(`DistanceToPoint`/`ClosestPoint`/`GetPoint`/`GetBounds`/`Midpoint`/`Length`/`Normalize`
etc.), `Equals`/`GetHashCode`/`ToString`/operators/`Deconstruct` for all 3 types.
**Deferred** (documented per-header): every `Intersects(...)` overload across all 3
types, plus `LineSegment2D::DistanceSquaredToPoint`/`DistanceToPoint`/
`DistanceSquaredToSegment`/`DistanceToSegment` — all need real `Collision2D` algorithms
(`SolveParametricIntersectionWithImplicitLine`, `ClipLineToAabb`,
`ClipLineToConvexPolygon`, `DistanceSquaredPointSegment`, `DistanceSquaredSegmentSegment`),
Phase 2.

**Important correction the fork caught and reported** (did not silently paper over):
`BoundingCapsule2D.hpp`'s deferral comment, written during the previous task before
`LineSegment2D` existed, assumed `LineSegment2D` landing would unblock
`CreateFromSegment`/`CreateMerged`. Wrong — `LineSegment2D::DistanceSquaredToPoint`'s own
body needs `Collision2D`, so those two `BoundingCapsule2D` members are still blocked on
Phase 2, not on `LineSegment2D`. **Fixed `BoundingCapsule2D.hpp`'s comment** to say this
correctly (own edit, not the fork's — the fork was told not to touch already-ported
files). Lesson: a deferred-dependency note written *before* the blocking type exists is a
guess, not a fact — re-verify it once the type actually lands, don't assume the original
note was right.

**Verification**: `cmake --build build -j"$(nproc)"` clean (both `-DCNA_EXTENDED_LINK_CNA=ON`
and headers-only), `ctest` → **100% passed, 137/137** (was 92 before this task). Rebuilt
again after the `BoundingCapsule2D.hpp` comment fix to confirm nothing broke (it's a
comment-only change, but it touches a header several files transitively include).

**Test coverage note** (same tradeoff as bounding volumes, see `plan.md`): upstream has
48 test methods across the 3 types but most exercise the deferred `Intersects`/`Distance*`
methods; wrote 43 fresh tests covering what's actually ported rather than porting
upstream 1:1. Bundled into the same "port the full upstream test suites once Collision2D
lands" Phase-2-start follow-up already noted for the bounding volumes.

**Committed as one commit** (task-granular): the fork's 9 files + the
`BoundingCapsule2D.hpp` correction together, since the correction was found during this
same task's review, before anything was committed.

**State / next step:** Phase 1 is 4 of ~20 tasks in. Next per `plan.md` §5 Phase 1: `Camera`
+ `OrthographicCamera`. **Before starting, check its actual dependencies** the same way —
two of the last three tasks turned up a dependency surprise plan.md's flat list didn't
show. The forked-sub-agent + independent-grep-verify + build/test-before-commit pattern
established over the last two tasks is working well; keep using it for large tasks (roughly:
anything reading and porting more than ~1 file or ~300 lines of upstream C# at once).

---

## 2026-07-12 (4) — Bounding volumes ported (Phase 1 task 3), via a forked sub-agent + review pass

Continued from session (3) after the user said "pokracuj" (continue).

**What happened:** Delegated the 5-type bounding-volume port (`BoundingBox2D`,
`BoundingCircle2D`, `BoundingCapsule2D`, `BoundingPolygon2D`, `OrientedBoundingBox2D` —
~3200 lines of C# source across the 5 `.cs` files) to a forked sub-agent, specifically to
keep that much source-reading out of the orchestrating session's context. Gave it the
established conventions (reference files, property naming, SPDX headers, build/test
commands, the "defer + document, don't half-declare" pattern from the Transform2 case)
and told it not to commit — orchestrator reviews and commits.

**Fork's output** (all 15 files: 5 headers, 5 sources, 5 tests, ~2289 lines):
- Fully ported: all fields, properties, factory methods, `Transform`/`Translate`/
  `Deconstruct`, `Equals`/`ToString`/operators for all 5 types.
- Correctly deferred (documented in each header's top comment): all 16
  `Contains`/`Intersects`/`TryGetCollision` overloads (need `Collision2D`, Phase 2, not
  this phase) and `BoundingCapsule2D::CreateFromSegment`/`CreateMerged` (need
  `LineSegment2D::DistanceToPoint`, the *next* task in this phase — do these two as a
  follow-up once `LineSegment2D` lands, don't forget).
- Build/test: reported clean build (both `-DCNA_EXTENDED_LINK_CNA=ON` and headers-only)
  and 100% tests passed (86/86 at that point).

**What the orchestrating-session review pass caught and fixed** — this is the important
part, read it before trusting a forked port's self-report next time: **`GetHashCode()`
was silently dropped from all 5 types.** The fork's own report didn't mention this; it
only surfaced by cross-checking the actual upstream `.cs` files' full public member list
(`grep -n "public "` against each file) against what got ported, not by reading the
fork's summary. C# pairs `Equals`/`GetHashCode` by convention (equal objects must hash
equal); dropping it silently would have been a real fidelity gap. Fixed by adding
`int GetHashCode() const` to all 5 types — see `plan.md`'s decisions log for the exact
approach (reused `sharp-runtime`'s `ArraySegment<T>` XOR-combine convention for 4 types,
`sharp-runtime`'s `System::HashCode` for `BoundingPolygon2D` to match upstream's
`HashCode.Add`/`ToHashCode()` loop). Added a `GetHashCode` round-trip test to all 5 test
files too (the fork's tests predated this fix, so they didn't cover it).

**Lesson for the next forked porting task**: don't just trust "it builds and its own
tests pass." Cross-check the ported member list against upstream's actual public API
(`grep -n "public "` on the `.cs` file is fast and cheap) before committing. Do this for
every forked task from here on, not just this one.

**Also flagged, accepted as a deliberate deferral (not silently dropped)**: upstream has
~130 test methods across these 5 types' own test files, but the fork wrote its own
(smaller, non-upstream-1:1) test set rather than porting the ~100 portable-now upstream
tests faithfully, since ~20-27% per file need the deferred `Collision2D` methods anyway.
Decided to revisit this as one pass at the start of Phase 2 (port the full upstream
`Primitives`/`Shapes` test suites 1:1 once `Collision2D` makes everything portable), not
now. Recorded in `plan.md`.

**Verification after the fix:** `cmake --build build -j"$(nproc)"` clean (both
`-DCNA_EXTENDED_LINK_CNA=ON` and default headers-only), `ctest` → **100% passed, 92/92**.

**Committed as one commit** (task-granular, per the agreed workflow) covering the fork's
15 files plus the `GetHashCode` fix and its tests together — the fix was found during
review of the same task, before anything was committed, so it's one logical unit, not a
separate task.

**State / next step:** Phase 1 is 3 of ~20 tasks in. Next: `Line2D`, `LineSegment2D`,
`Ray2D` (`plan.md` §5 Phase 1) — and remember `BoundingCapsule2D::CreateFromSegment`/
`CreateMerged` are waiting on `LineSegment2D::DistanceToPoint` specifically, so check
whether to fold that follow-up into the same commit as `LineSegment2D` itself once it's
ported.

---

## 2026-07-12 (3) — Phase 1 started: marker interfaces + Transform3 ported, MulticastAction extended

Continued directly from session (2) in the same sitting. User confirmed (in response to
"any questions before we start?"): port faithfully, 1:1 wherever C#/C++ differences allow,
no simplification — and commit after every completed `plan.md` task (already the agreed
workflow, reconfirmed).

**Completed this session (each its own commit — see `git log`):**
1. Task "Marker interfaces" (`plan.md` Phase 1): `IMovable`, `IRotatable`, `IScalable`,
   `ISizable`, `IRectangular`/`IRectangularF`, `IColorable`, `IEquatableByRef<T>` — all
   ported as abstract classes with `getX/setXProperty()` accessors. `ISizable`/
   `IRectangularF` forward-declare `SizeF`/`RectangleF` (not yet ported — see below).
   Tests in `tests/CNA/Extended/InterfaceTests.cpp`.
2. **Discovered and fixed a real infra gap while testing task 1**: CNA's XNA types
   (`Vector2`, `Color`, `Rectangle`, ...) are declared in headers but only *defined* in
   CNA's compiled `.cpp` files (same caveat `easy-3d` documents). A test that constructs
   one needs CNA actually linked to run. `tests/CMakeLists.txt` now follows `easy-3d`'s
   `CNA_EXTENDED_CNA_LINKED`-gated pattern for the whole suite (real gtest executable when
   linked, `OBJECT`-library compile-check otherwise). **Standard build/verify command for
   this project going forward:**
   ```
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCNA_EXTENDED_LINK_CNA=ON
   cmake --build build -j"$(nproc)"
   cd build && ctest --output-on-failure
   ```
   This actually builds CNA itself (EASY_GL backend) alongside cna-extended — takes a few
   minutes the first time, incremental after. Confirmed working in this environment. The
   plain headers-only default build (`cmake -S . -B build`, no `-DCNA_EXTENDED_LINK_CNA`)
   still compile-checks cleanly — verified both paths after every subsequent change so far.
3. Task "Transform" (`plan.md` Phase 1) — **partially complete, intentionally split**:
   `TransformFlags`, `BaseTransform<TMatrix>`, and `Transform3` are fully ported and
   tested (`include/CNA/Extended/Transform.hpp`, `src/.../Transform.cpp`,
   `tests/.../TransformTests.cpp`). **`Transform2` (`BaseTransform<Matrix3x2>`) is
   explicitly deferred** — its `RecalculateLocalMatrix`/`RecalculateWorldMatrix` bodies
   need `Matrix3x2::CreateScale/CreateRotationZ/CreateTranslation/Multiply/Decompose`,
   and `Matrix3x2` itself (1037-line C# source) is a later task in this same phase. Do
   not leave `Transform2` half-declared — port it as the immediate follow-up once
   `Matrix3x2` lands, not standalone. This is recorded in `plan.md`'s Phase 1 checklist,
   not just here.
4. **Cross-repo change, user-approved**: `Transform.cs`'s `BaseTransform<TMatrix>`
   resubscribes to *every* ancestor's `TransformBecameDirty` C# event whenever `Parent`
   changes, unsubscribing the entire old ancestor chain by delegate identity first.
   `sharp-runtime`'s `System::MulticastAction<Args...>` only supported `+=`/replace/clear
   — no way to remove one specific handler (C++ lambdas/`std::function` have no identity
   equality the way C# delegates do). Asked the user how to resolve this (extend
   `sharp-runtime`, add a local workaround, or diverge from the exact mechanism); **user
   chose extending `sharp-runtime`** (normally off-limits per `CLAUDE.md` without explicit
   permission — this was explicit permission for this specific need). Added
   `Token Add(HandlerType)` / `bool Remove(Token)` to `MulticastAction`
   (`sharp-runtime/include/System/MulticastAction.hpp`), purely additive — `operator+=`
   unchanged, existing behavior unchanged. Added 5 new tests
   (`sharp-runtime/tests/System/MulticastActionTests.cpp`); **all 11562 sharp-runtime
   tests pass** (ran the full suite, not just the new ones). Committed separately in
   `sharp-runtime`'s own repo (branch `develop`) — that commit is **not** part of
   `cna-extended`'s history; if resuming on a machine without that sharp-runtime commit,
   `Transform.hpp`'s use of `MulticastAction::Add/Remove` will fail to compile.
   **Expect this same need (a C# `event Action` with `-=`) to recur elsewhere in the
   port — reach for `Add`/`Remove` first before inventing another local workaround.**

**Verification status:** All local tests pass as of the last commit —
`cd build && ctest --output-on-failure` → 16/16 (`MarkerInterfaces.*` ×6,
`Transform3Tests.*` ×7 incl. a reparenting/dirty-propagation test that specifically
exercises the new `MulticastAction::Remove`, `Version.*` ×2). Headers-only default build
also compile-checks clean.

**State / next step:** Phase 1 is 2 of ~20 tasks in (see `plan.md` §5 Phase 1 checklist
for the authoritative live list — check it, not this prose, for exact remaining items).
Next up in list order: bounding volumes (`BoundingBox2D`, `BoundingCircle2D`,
`BoundingCapsule2D`, `BoundingPolygon2D`, `OrientedBoundingBox2D`). **Before starting
each new task, check its actual C# dependencies against what's already ported** — two
ordering surprises already turned up this session (`Transform2`→`Matrix3x2`,
`ISizable`/`IRectangularF`→`SizeF`/`RectangleF`) that weren't visible from `plan.md`'s
flat task list alone; forward-declare-and-defer is the established pattern when a
same-phase dependency isn't ready yet (see `ISizable.hpp`/`IRectangular.hpp` for the
forward-declaration style, and `InterfaceTests.cpp`'s header comment for how the deferred
tests get appended later). Do not silently skip or simplify the blocked piece — defer it
explicitly, in both the code comment and `plan.md`.

This is a genuinely large effort (Phase 1 alone has ~18 remaining tasks; 10 phases total,
likely several hundred source files by the end). Pace accordingly across sessions —
prefer several fully-tested, fully-committed tasks over rushing ahead into a half-checked
state.

---

## 2026-07-12 (2) — Plan approved; Phase 0 scaffolding complete and green

The user approved `plan.md` and confirmed two things explicitly:
1. **Strict fidelity**: port 1:1 wherever C#/C++ language differences allow — no
   simplifying, no dropping edge cases, no "cleaning up" while porting. `plan.md`'s
   Status line now records this; `CLAUDE.md`'s "Working rules" section already matches it
   (reuse `sharp-runtime` types, don't re-roll, keep the full task list) but treat this as
   the standing bar for every phase, not just something said once.
2. **Git workflow for this long unattended session**: commit after every completed task
   (granular, one task from `plan.md` = one commit), never push without explicit
   permission (unchanged baseline rule — nothing here overrides it). Applied from Phase 0
   onward.

**Phase 0 executed in full this session**, one task per commit (see `git log` — 9 commits
from `LICENSE` through the final build-verification pass, each following the
`plan.md`-task → commit pattern the user asked for). All Phase 0 checkboxes in `plan.md`
are now checked. Key decisions made while executing (not asked as separate questions —
these were mechanical/low-stakes, following existing house convention):
- CMake target `CNA_EXTENDED`, alias `CNA::Extended` (matches `CNA`/`SHARP_RUNTIME`
  uppercase convention + the `CNA::Extended` C++ namespace).
- Root `CMakeLists.txt` follows `easy-3d`'s exact three-tier sibling-dependency pattern
  (`if(TARGET CNA)` / opt-in `CNA_EXTENDED_LINK_CNA` standalone build / headers-only
  fallback), applied to **both** `cna` and `sharp-runtime`.
- Doxyfile generated via `doxygen -g` then hand-edited for the same fields
  `sharp-runtime` customizes (`PROJECT_NAME`, `OUTPUT_DIRECTORY=docs/generated`,
  `EXTRACT_ALL=YES`, `RECURSIVE=YES`, `GENERATE_LATEX=NO`,
  `INPUT=include README.md`) rather than hand-writing one from scratch.
- `.gitignore` merged `easy-3d`'s broader build/IDE/OS coverage with `sharp-runtime`'s
  `docs/generated/` entry.

**Build verification (Phase 0 exit criterion — met):**
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j"$(nproc)"
./build/examples/minimal/cna_extended_minimal   # -> "cna-extended 0.1.0"
cd build && ctest --output-on-failure            # -> 100% tests passed, 2/2
```
Default build (no `-DCNA_EXTENDED_LINK_CNA=ON`, so CNA/sharp-runtime are header-only-
resolved, not actually linked — Phase 0's only code, `Version`, doesn't need them
compiled in anyway) configures, builds, and tests clean with zero errors/warnings. Both
`../cna/include` and `../sharp-runtime/include` were found automatically at their default
relative paths, so the sibling-repo layout assumption in `plan.md` §4 holds in this
environment.

**State**: Phase 0 done and committed. **Phase 1 (Math, Shapes, Interfaces &
Collections) starts next** — no blockers. Read `plan.md` §5 Phase 1 for the exact task
list; work through it in the same one-task-one-commit rhythm.

**Not yet exercised**: `-DCNA_EXTENDED_LINK_CNA=ON` (building CNA itself alongside
cna-extended) hasn't been tried yet — Phase 1 doesn't need it either (pure math/data
types, no `GraphicsDevice`/`SpriteBatch`). First real need will be Phase 5 (Graphics) or
earlier if a Phase 1-4 test wants to construct a real XNA type that only compiles cleanly
against CNA's actual headers (should be fine headers-only, but worth watching for).

---

## 2026-07-12 (1) — Repository bootstrap: research, scope Q&A, planning docs

**State at session start:** `cna-extended` contained nothing but an empty `.git` — a
genuinely greenfield repository. The user's ask: port
[MonoGame.Extended](https://github.com/craftworkgames/MonoGame.Extended) to C++, to be
consumed by `../cna` (an XNA/FNA C++ port) and built on `../sharp-runtime` (a C++ .NET BCL
reimplementation), using `../easy-3d` as CMake-integration inspiration. Explicit
instruction: **create `plan.md`, get it approved, only then start porting.** Also asked to
identify MonoGame.Extended's feature areas and get an explicit per-area go/no-go, and to
verify licensing before doing anything.

**What was done this session (research + planning only — no C++ code written):**

1. Explored `../cna`, `../sharp-runtime`, `../easy-3d` (via parallel sub-agents) to learn
   house conventions: C++23, GoogleTest, Doxygen, header/source split mirroring namespace
   paths, `getXProperty()`/`setXProperty()` accessor naming, and — most importantly —
   `easy-3d`'s CMake pattern for depending on a sibling repo (`if(TARGET CNA) ... elseif
   (LINK_CNA option) add_subdirectory(../cna) ... elseif (headers-only fallback)`). This
   pattern is what `plan.md` §4 proposes reusing for `cna-extended`'s own dependency on
   `cna` and `sharp-runtime`.
2. Cloned MonoGame.Extended. **Correction to the user's request:** the org is
   `craftworkgames/MonoGame.Extended`, not `MonoGame-Extended/...` (that org doesn't
   exist on GitHub — a 401/"Repository not found" from the real GitHub API, not a network
   restriction in this environment; general GitHub cloning works fine here).
   **Correction to the user's suggested clone location:** cloned to
   `/rv/data/library/github.com/craftworkgames/MonoGame.Extended` instead of the
   suggested `/rv/tmp`, because `cna`'s own `CLAUDE.md` already establishes
   `/rv/data/library/github.com/<owner>/<repo>` (managed by
   `/rv/data/library/github.com/github.sh`) as this ecosystem's convention for reference
   clones (e.g. `/rv/data/library/github.com/FNA-XNA/FNA`, which `cna` treats as its
   authoritative behavioral reference). Run `github.sh` with no args from
   `/rv/data/library/github.com` to refresh all reference clones including this one.
3. Verified license: MonoGame.Extended is MIT (Copyright 2015–2024 Dylan Wilson, Lucas
   Girouard-Stranks, Christopher Whitley; every source file carries a Craftwork Games MIT
   header, including the Particles module despite it being derived-in-spirit from the
   separate Mercury Particle Engine — see `plan.md` §3). This clears porting +
   relicensing under `cna-extended`'s own MIT license, with attribution in `NOTICE.md`
   (not yet created — Phase 0 task).
4. Surveyed the full module structure (file counts, dependencies between modules,
   which classes are xnb/Content-Pipeline-dependent vs directly portable) and put the
   findings to the user as a consolidated set of scoping questions (`AskUserQuestion`,
   4 questions covering ~19 module-level decisions). **All proposed modules were
   approved** except the already-agreed Content Pipeline exclusion — see `plan.md` §2 for
   the full in/out list and §6 for the decisions log. Namespace decided:
   `CNA::Extended::` (not a 1:1 `MonoGame::Extended::` mirror, not a flat
   `CnaExtended::`).
5. Wrote `plan.md` (10 phases, dependency-ordered, ~90 checkbox-level tasks) and this
   file. **`CLAUDE.md` is the next thing to write, in this same session if time allows.**

**Explicit blocker — read this before doing anything else:**
Per the user's original instruction, *no porting/implementation work, and no repo
scaffolding beyond `plan.md`/`NEXT.md`/`CLAUDE.md`, has been done yet.* Phase 0 of
`plan.md` (LICENSE, NOTICE.md, README.md, CMakeLists.txt, directory skeleton, etc.) is
written but **not executed**. The next session must not start Phase 0 (or any later
phase) until Robert Vokáč has actually reviewed and approved `plan.md`. If you are
resuming this session and the user has since approved the plan (check the most recent
chat turns / ask if unclear — do not assume silence means approval), start at Phase 0.
If not yet approved, your job is to answer any remaining open questions and refine
`plan.md`, not to write C++.

**Assumptions baked into `plan.md` that were not asked as explicit questions** (call
these out if the user pushes back — they're the "safe reversible default" per the
autonomous-session ground rules, not settled facts):
- C++23, GoogleTest, Doxygen, header/source-mirrors-namespace layout — copied wholesale
  from `cna`/`sharp-runtime`/`easy-3d` since there was no reason to diverge.
- Tests get ported alongside each phase's implementation, not deferred.
- `plan.md` (markdown checkboxes) is the tracking mechanism, not `sharp-runtime`'s
  `plan.sqlite3` approach — user explicitly asked for tasks to live in `plan.md`.
- FNA variant of MonoGame.Extended's `#if FNA`/`#if KNI` conditionals is the reference
  branch where they diverge, since `cna` mirrors FNA.

**Open design risk flagged in `plan.md` §7, not yet investigated:** `Graphics/Effects`
ships pre-compiled shader blobs (`DefaultEffect.dx11.mgfxo`/`.ogl.mgfxo`) alongside an
`.fx` source. Whether these map cleanly onto CNA's graphics backends
(`SDL_RENDERER`/`EASYGL`/`BGFX`/`VULKAN`) or need real re-authoring per backend is
unknown — first thing to investigate at the start of Phase 5, and worth flagging to the
user if it turns out to be a real fork rather than a mechanical port.

**Commands / validation status:** None yet — no buildable code exists. Phase 0's exit
criterion is "empty lib + empty test binary builds green"; record the actual build
command and result here once that happens.

**Recommended next step:** Get `plan.md` reviewed/approved by the user. If approved,
start at Phase 0 exactly as written, and update this file the moment Phase 0's scaffold
builds green (not at the end of a multi-phase run — keep entries granular).
