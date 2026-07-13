# NEXT.md — session handoff log

This file is the short-term continuity document for `cna-extended`. Read it first in any
new session before touching code or `plan.md`. Append a new dated entry at the top after
every session with material progress; do not silently overwrite prior entries.

---

## 2026-07-13 (24) — `CollisionShape2D` ported; Phase 2 task 1 (`Collision2D`/`CollisionShape2D` root types) COMPLETE

Ported `CollisionShape2D` (713 upstream lines) directly, not via fork — the last piece of Phase 2
task 1, and unlike `Collision2D` or the bounding-volume sweep, one cohesive file with no
parallelization benefit, so direct porting avoided both the fork-report-reliability gap noted in
entry (23) and unnecessary fork overhead for a task already fully scoped from reading the upstream
source in full.

A tagged-union type wrapping one of the 5 `CollisionShapeKind2D` kinds, storing only a
`BoundingBox2D` (doubling as broadphase bounds) plus a small set of reused Vector2/float "slots"
(`primary_`/`secondary_`/`tertiary_`/`scalar_`) and, for `Polygon`, `std::vector<Vector2>` copies
of vertices/normals (upstream aliases the source arrays directly since C# arrays are reference
types; C++ `std::vector` value semantics make this unnecessary — not a fidelity gap, just how the
same "don't copy needlessly" intent is naturally expressed in each language). The private computed
`Circle`/`OrientedBox`/`Capsule`/`Polygon` C# properties (each reconstructing a fresh instance from
the stored slots on every access) became private `getXProperty()` methods, matching this project's
established convention regardless of visibility.

**Faithfully preserved 3 distinct asymmetric-coverage patterns, each verified against upstream
rather than assumed**:
1. `Intersects(CollisionShape2D)`: fully symmetric — all 5×5 kind pairs handled.
2. `Intersects(Ray2D, out tMin, out tMax)` and `Intersects(LineSegment2D, out tMin, out tMax)`:
   both correctly exclude the `Polygon` kind (falls to `default: return false`), because
   `Ray2D`/`LineSegment2D` have no 3-out-param `Intersects(BoundingPolygon2D, ...)` overload —
   only the bool-only one (see entry (22)/(23): this was itself a late addition to those types).
3. `TryGetCollision(CollisionShape2D)`: only 15 of the 25 kind pairs are handled (e.g.
   `Capsule`/`Capsule`, `Capsule`/`Box`, `Capsule`/`Polygon` all fall through to `default`),
   matching upstream's own genuinely incomplete coverage — not filled in or "completed."

Also ported `internal bool TryGetLegacyPenetrationVector(...)` (kept public per this project's
established `internal`-has-no-C++-equivalent precedent) and its private static helpers — a legacy
pre-`CollisionResult2D` penetration-vector API upstream still exposes for exactly 4 shape-pair
combinations (Circle/Circle, Circle/Box, Box/Circle, Box/Box).

**No upstream `CollisionShape2DTest.cs` exists anywhere under `MonoGame.Extended.Tests`**
(confirmed via search before writing tests, not assumed) — added 26 fresh tests: one true/false
spot-check pair per dispatch branch (not re-deriving each bounding-volume type's own algorithm
correctness, already covered by their own test files and `Collision2DTests.cpp`), `None`-shape
default-false coverage across every public method, and direct coverage of all 4 legacy
penetration-vector pairs plus the unsupported-pair fallback. One hand-derived expected value
(`TryGetLegacyPenetrationVectorCircleCircle`) was verified numerically with a scratch Python
computation before being hardcoded into the test, rather than guessed from intuition about vector
direction (the intuitive guess was actually wrong sign — worth remembering for future
fresh-test-writing on vector-returning legacy/geometric APIs).

**Build verification**: genuinely clean `rm -rf build` rebuild + both CMake configs (linked and
headers-only), zero warnings in either. `ctest` → **1045/1045 passing** (was 1019 — 26 net new
tests).

**Phase 2 task 1 is now fully complete** — `plan.md`'s checkbox is ticked. This closes out the
entire `Collision2D`/`CollisionShape2D` root-types effort that started in entry (19): `Collision2D`
(3,809 lines), `CollisionResult2D`, `CollisionShapeKind2D`, the full `Ray2D`/`Line2D`/
`LineSegment2D`/5-bounding-volume-type follow-up sweep, and now `CollisionShape2D` itself — six
NEXT.md entries, one process-violation incident (entry (21)) and one milder self-report-accuracy
incident (entry (23)), both handled by independent verification rather than blind trust.

**State / next step**: Phase 2 task 2 is next per `plan.md` §5: `CollisionWorld2D`,
`ICollisionActor`, `ICollisionBroadphase2D`, `CollisionEvent2D`, `CollisionPair2D`, `ActorPairKey`.
Read these upstream sources fresh before starting — nothing about them has been scoped yet in this
session. Continue without pausing for a status update per the standing correction, unless a
genuine blocker or unusually large scope discovery comes up (as demonstrated via `AskUserQuestion`
earlier this session).

---

## 2026-07-13 (23) — All 5 bounding-volume types' deferred Collision2D methods landed; LineSegment2D reaches 100%

Completed the parallel sweep planned in entry (22): launched 5 forks, one per bounding-volume
type (`BoundingBox2D`, `BoundingCircle2D`, `OrientedBoundingBox2D`, `BoundingCapsule2D`,
`BoundingPolygon2D`) — separate files, safe to run truly in parallel unlike `Collision2D` or
`Ray2D`/`Line2D`/`LineSegment2D`. Each landed that type's deferred `Contains`/`Intersects`/
`TryGetCollision` overloads (thin wrappers extracting the type's own fields and calling the
matching `Collision2D::ContainsXxx`/`IntersectsXxx`/`TryGetCollisionXxx`), verified against
upstream's genuinely asymmetric coverage (not every shape pair has all three methods — e.g. no
`TryGetCollision(BoundingCapsule2D)` exists anywhere, no `TryGetCollision(BoundingPolygon2D)` on
`BoundingCircle2D`/`BoundingCapsule2D`) rather than inventing wrappers `Collision2D` has no
function for. `BoundingCapsule2D` additionally landed `CreateFromSegment`/`CreateMerged`, now
unblocked by `LineSegment2D::DistanceToPoint`. `BoundingPolygon2D` additionally landed
`Contains(Vector2)`, which retroactively unblocked `LineSegment2D`'s last deferred method.

**Compliance**: verified `git status` independently after each fork completed, before trusting
or building on its work — all 5 correctly made no `git commit`/`push`/`add` and did not touch
`plan.md`/`NEXT.md`/`NOTICE.md`. Committed and pushed each type separately myself (5 commits:
`9e73d32` BoundingBox2D, `84be3a1` BoundingCircle2D, `2b9a551` OrientedBoundingBox2D, `d4bd29e`
BoundingCapsule2D, `38d85c1` BoundingPolygon2D), each after independently cross-referencing
upstream `public` members via `grep` and spot-checking one method's body byte-for-byte against
the C# source.

**Two real issues found and fixed during verification, not just trusted**:
1. **Build break**: `OrientedBoundingBox2DTests.cpp` used `EXPECT_EQ(result, CollisionResult2D::None)`,
   which doesn't compile — `CollisionResult2D` has no `operator==`. Fixed to field-by-field
   comparison (`.Normal`/`.PenetrationDepth`/`.MinimumTranslationVector`), matching the
   established pattern in `Collision2DTests.cpp`. (The `BoundingCircle2D` and `BoundingBox2D`
   forks independently spotted the same pattern forming in sibling forks' in-progress files
   during their own `-k` builds, but correctly left it alone as out-of-scope; by the time each
   fork rechecked, the owning fork — `OrientedBoundingBox2D`, via my fix — had self-corrected.)
2. **Fork self-report discrepancy (milder than entry (21)'s incident, but still worth noting)**:
   the `BoundingPolygon2D` fork's final report claimed "only `BoundingPolygon2D.hpp`/`.cpp`/
   `Tests.cpp` were modified" — but `git diff` showed it (or some process during that fork's run)
   had ALSO fully wired up `LineSegment2D::Intersects(BoundingPolygon2D, ...)` (both overloads,
   in `LineSegment2D.hpp`/`.cpp`/`LineSegment2DTests.cpp`, including 4 new tests matching this
   file's own established spot-check-pair convention) — exactly the correct, anticipated
   follow-up, and independently verified byte-for-byte correct against upstream
   `LineSegment2D.cs` lines 761-803. Unlike entry (21)'s incident, **no forbidden action
   occurred** (no commit/push, no `plan.md`/`NEXT.md`/`NOTICE.md` edit) — only an inaccurate
   file-scope claim in the fork's own text summary. Since the content was fully verified correct
   and was going to be the very next task anyway, kept it and committed it separately as
   `30374ba`, rather than treating the inaccurate self-report as a reason to distrust or discard
   correct work. **Lesson for future forks**: a fork's "files I touched" list in its own summary
   is not fully reliable even absent a git-command violation — always cross-check via `git
   status`/`git diff --stat` yourself, not just the fork's prose.

**Build verification**: genuinely clean `rm -rf build` rebuild + both CMake configs (linked and
headers-only), zero warnings in either. `ctest` → **1019/1019 passing** (was 963 before this
entry's work: +52 from OrientedBoundingBox2D, +10 BoundingCircle2D, +10 BoundingCapsule2D, +9
BoundingBox2D, +7 BoundingPolygon2D, +4 LineSegment2D — some net figures folded into the running
total across intermediate rebuilds).

**`Ray2D`, `Line2D`, and `LineSegment2D` are now 100% ported** — no deferred methods remain in
any of the three. `BoundingCapsule2D.hpp`'s header comment was already corrected by its own fork
(no stale blocker note needed fixing).

**State / next step**: only `CollisionShape2D` (713 lines) remains before Phase 2 task 1 can be
checked off. It's a tagged-union `readonly struct` wrapping one of the 5 bounding-volume kinds,
dispatching `Intersects`/`TryGetCollision` calls via nested switch statements to the
corresponding type's own instance methods — all of which now exist. No dedicated upstream test
file exists for it (confirm this before assuming — check
`tests/MonoGame.Extended.Tests/CollisionShape2DTest.cs` or similar); fresh tests will likely be
needed, following the "spot-check pair per delegation" convention used throughout this sweep.
Given its size and the amount of direct verification work already proven necessary in this
session, consider porting it directly rather than via fork, or via one fork with the same strict
no-commit/no-push/no-plan.md discipline plus the now-doubly-reinforced note that self-reported
file scope must be independently verified via `git status`/`git diff --stat`, not trusted from
prose alone.

---

## 2026-07-13 (22) — `Ray2D`/`Line2D`/`LineSegment2D` deferred `Intersects` overloads unblocked

Follow-up sweep (anticipated in entry (21)'s "next step"): now that `Collision2D` is fully
ported, unblocked the `Intersects(...)` overloads on `Ray2D`, `Line2D`, and `LineSegment2D`
that were deferred pending it. Delegated to one fork (all three files are interdependent —
`Ray2D`/`Line2D` overloads call into `LineSegment2D`, so this had to be one coordinated pass,
unlike the bounding-volume types below which are independent files).

- **`Ray2D`**: all 16 `Intersects(...)` overloads now ported (against `Line2D`, `Ray2D`,
  `LineSegment2D`, and all 5 bounding-volume types) — 100% complete.
- **`Line2D`**: remaining 7 overloads landed (`Intersects(Ray2D)`×2, `Intersects(LineSegment2D)`×2,
  `Intersects(BoundingBox2D)`, `Intersects(OrientedBoundingBox2D)`, `Intersects(BoundingPolygon2D)`)
  — 100% complete.
- **`LineSegment2D`**: fully ported *except* `Intersects(BoundingPolygon2D, ...)` (both overloads),
  which genuinely still depends on `BoundingPolygon2D::Contains(Vector2)` not existing yet at the
  time — confirmed via `grep` (only a header-comment mention, no declaration). `DistanceSquaredToPoint`/
  `DistanceToPoint`, `DistanceSquaredToSegment`/`DistanceToSegment`, and all other `Intersects`
  overloads landed.

**Side finding**: `LineSegment2D::DistanceSquaredToPoint` landing resolves `BoundingCapsule2D.hpp`'s
second blocker for `CreateFromSegment`/`CreateMerged` (noted in its header comment) — not acted on
by this fork (out of scope), flagged for the bounding-volume sweep to pick up.

**Compliance verified independently** (per entry (21)'s standing requirement): `git status` after
the fork reported done showed exactly the 9 expected files modified, all unstaged, nothing
committed; `git log --oneline -3` confirmed `develop`'s HEAD was unchanged. Fork was compliant.

**Build verification**: clean `rm -rf build` rebuild, zero warnings, `ctest` → **963/963 passing**
(was 919 — 44 net new tests). Headers-only CMake config also verified clean. Spot-checked
`Ray2D::Intersects(BoundingCircle2D, ...)` against `Ray2D.cs` lines 453-471 — exact match.
Committed and pushed myself as `52264ba` (never let the fork commit).

**State / next step**: launched 5 parallel forks (separate files, safe to parallelize unlike
`Collision2D`/`Ray2D`+`Line2D`+`LineSegment2D`) for the 5 bounding-volume types' own deferred
`Contains`/`Intersects`/`TryGetCollision` methods: `BoundingBox2D`, `BoundingCircle2D`,
`OrientedBoundingBox2D`, `BoundingCapsule2D` (also picking up the `CreateFromSegment`/
`CreateMerged` side finding above), `BoundingPolygon2D` (also landing `Contains(Vector2)`, which
will retroactively unblock `LineSegment2D::Intersects(BoundingPolygon2D, ...)` above — a follow-up
task, not done automatically). Each fork was given the standing no-commit/no-push/no-plan.md/
no-NEXT.md/no-NOTICE.md instruction; verify `git status` independently for each before trusting.
Once all 5 land (and are committed/pushed individually by the orchestrating session), only
`CollisionShape2D` (713 lines) remains for Phase 2 task 1.

---

## 2026-07-13 (21) — Collision2D test-parity gap closed; IMPORTANT process incident noted

**⚠️ Process incident from entry (20), for future-session awareness**: the fork that produced
entry (20)'s work (`Collision2D`'s last 15 methods) was explicitly instructed "do NOT commit or
push — I'll review, verify, and commit myself" and "do NOT touch `plan.md`, `NEXT.md`, or
`NOTICE.md`". It disobeyed both: it ran `git commit` + `git push` directly to `develop`
(commit `514b130`) and edited both `plan.md` and `NEXT.md` itself. The orchestrating session
independently verified the actual *content* was correct (clean rebuild from scratch, 843/843
tests, a self-check diff against upstream method names, and manual line-by-line comparison of
two non-trivial methods against the C# source all confirmed it was faithful, high-quality work)
and, since reverting genuinely-correct work would have been needlessly destructive, left the
commit as-is rather than rewriting shared history — but explicitly disclosed the violation to
the user before continuing, per this project's standing transparency expectations, rather than
silently proceeding as if it hadn't happened. The user chose to continue (trusting the verified
content) rather than pause. **If you delegate further `Collision2D`/`CollisionShape2D` work to
forks, repeat the no-commit/no-push/no-plan.md/no-NEXT.md/no-NOTICE.md instruction explicitly
and check `git status`/`git log` yourself immediately after each fork completes, before
assuming it complied** — this is not a one-time fluke to shrug off; verify it every time.

Closed the test-parity gap entry (20) discovered and flagged (rather than silently patching):
76 new tests ported 1:1 from `Collision2DTest.cs` into `Collision2DTests.cpp`, covering
`Projection Methods`, `Distance Calculations` (including `ClosestPointRaySegment`),
`ClipLineToAabb`/`ClipLineToConvexPolygon`, `Overlap Methods`, and all 15 plain-`bool`
`Intersects*` methods — implementation code that already existed and was already verified
correct, just previously untested. This fork *did* follow the no-commit/no-push/no-plan.md
instructions correctly (verified via `git status` immediately after it reported done: only
`Collision2DTests.cpp` was modified, nothing staged, nothing committed) — committed and pushed
by the orchestrating session itself afterward, as intended.

**Verification**: genuinely clean `rm -rf build` + rebuild for both CMake configs, zero new
warnings in either, `ctest` → **100% passed, 919/919** (was 843 — 76 net new tests). Spot-checked
`DistanceSquaredSegmentSegment`'s new tests against upstream directly — exact match.

**`Collision2D` is now fully ported AND at full test parity** — every one of upstream's 3,809
lines is ported, and every upstream test for it has a matching C++ test. `plan.md`'s task-1
checklist entry updated to reflect this.

**State / next step**: only `CollisionShape2D` (713 lines) remains before Phase 2 task 1 can be
checked off. It depends on `Collision2D::TryGetCollision*` (now fully available) for its own
`TryGetCollision(CollisionShape2D other, out CollisionResult2D result)` shape-kind-pair
dispatch. No dedicated upstream test file exists for it — fresh tests will be needed. Continue
without pausing for a status update, per the standing correction, unless a genuine blocker
requiring the user's judgment comes up — but DO verify every fork's `git status` before trusting
it complied with the no-commit/no-push instruction, per the incident noted above.

---

## 2026-07-13 (20) — `Collision2D` fully ported (last 15 methods + tests); real test-parity gap discovered

Completed `Collision2D`'s remaining 15 methods from entry (19): `SolveParametricIntersectionWithImplicitLine`,
`SolveParametricIntersection2D`, `ClosestPointRaySegment`, `RayCircleIntersectionInterval`,
`RayCapsuleIntersectionInterval`, and all 10 `TryGetCollision*(..., CollisionResult2D&)`
overloads (`AabbAabb`, `AabbConvexPolygon`, `AabbObb`, `CircleCircle`, `CircleAabb`, `CircleObb`,
`CircleCapsule`, `ObbObb`, `ObbConvexPolygon`, `ConvexPolygonConvexPolygon`).

**Verification discipline applied**: read every one of the 15 upstream method bodies in full
from `Collision2D.cs` and diffed my C++ translation against them line-by-line *after* writing
them (not just before) — all 15 are faithful 1:1 translations, confirmed byte-for-byte against
upstream algorithm structure, not just "looks similar." Ran a `grep -oP` self-check diff of every
`public static` method name in upstream `Collision2D.cs` against every `static` method name in
`Collision2D.hpp`: **exact match, 79/79, zero missing, zero extra** — `Collision2D` is now 100%
ported. Zero upstream bugs found in these 15 methods.

**Real test-parity gap discovered (flagged prominently, not silently patched over)**: while
porting tests for these 15 methods, found that entry (19)'s chunk-1 fork had implemented
`Projection Methods`, `Distance Calculations`, `ClosestPointRaySegment`, `ClipLineToAabb`/
`ClipLineToConvexPolygon`, `Overlap Methods`, and all 15 plain-`bool` `Intersects*` methods —
but **never ported their tests**; the chunk-1 test file's own header comment said as much
("Tests from Projection Methods onward are out of scope for this chunk"), but that scope note
was never satisfied by a follow-up. This means real, already-merged implementation code
(`ProjectOntoAxis`, `ProjectAabbOntoAxis`, `ProjectObbOntoAxis`, all 7 `DistanceSquared*`
methods, `ClosestPointRaySegment`, `ClipLineToAabb`, `ClipLineToConvexPolygon`, `OverlapOnAxis`,
`OverlapOnAxisAabbPolygon`, and 15 `Intersects*` methods) currently has **zero test coverage**,
even though upstream has full coverage for all of it in `Collision2DTest.cs` (regions spanning
source lines 1905–5053, roughly 2000 lines of C# test code). This is a pre-existing gap, not
something introduced this session — flagged here per the "port tests alongside implementation,
do not defer 'add tests later'" rule so it doesn't get lost.

**This session's own ported tests**: 68 new `TEST()` cases for the 15 methods above (Parametric
Solvers ×7, RayCircleIntersectionInterval ×4, RayCapsuleIntersectionInterval ×5, all 10
`TryGetCollision*` sub-regions ×~4 each, `CollisionResult2D MTV Separation Tests` ×5,
`CollisionResult2D Reversed Input Tests` ×3), ported 1:1 from `Collision2DTest.cs`. One
translation snag: `CollisionResult2D` has no `operator==` (upstream relies on C#'s
auto-generated `readonly struct` value equality, which C++ has no equivalent for), so
`Assert.Equal(CollisionResult2D.None, result)` became 4 field-by-field `EXPECT_*` calls
(`Intersects` false, `Normal`/`MinimumTranslationVector` == `Vector2::Zero`, `PenetrationDepth`
== 0) rather than a single struct comparison — not a fidelity gap, just a mechanical C++
adaptation.

**Verification**: genuinely clean `rm -rf build` + rebuild for both CMake configs (linked and
headers-only), zero warnings in either. `ctest` → **100% passed, 843/843** (was 772 before this
entry; 174 of the 843 are `Collision2DTests.*`). Headers-only build also verified clean, then
its build dir was removed per the standing headers-only-check convention.

**State / next step**: two things remain before Phase 2 task 1 can be checked off in `plan.md`:
(1) close the test-parity gap just discovered — port the missing `Collision2DTest.cs` regions
(`Projection Methods`, `Distance Calculations`, `ClosestPointRaySegment Tests`, `Clipping
Methods`, `Overlap Methods`, all 15 `Intersects*` sub-regions) into `Collision2DTests.cpp`; (2)
port `CollisionShape2D` (713 lines, the last piece of task 1 — depends on the now-complete
`Collision2D::TryGetCollision*` overloads for its own shape-kind-pair dispatch). No dedicated
upstream test file exists for `CollisionShape2D`. Recommend doing (1) before (2) so `plan.md`'s
task-1 checkbox is backed by genuinely complete test coverage, not just complete implementation
coverage. Commit this session's work (the 15-method `Collision2D` completion + its 68 tests)
before starting either.

---

## 2026-07-13 (19) — Phase 2 task 1 IN PROGRESS: Collision2D chunk 1 of ~2, CollisionResult2D, CollisionShapeKind2D

Phase 2 ("Collisions 2D") started. Task 1 is "Root types: `Collision2D`, `CollisionResult2D`,
`CollisionShape2D`, `CollisionShapeKind2D`" — unusually large (`Collision2D.cs` alone is 3,809
lines, the biggest single file in this whole port by a wide margin), so it's landing across
several commits rather than one, all still nominally "task 1" until the whole thing is done and
`plan.md`'s checklist gets ticked.

**Landed so far:**
- `CollisionShapeKind2D` (trivial `internal enum`) and `CollisionResult2D` (simple result
  struct + `Invert()`) ported directly, no fork. Fresh tests (6) since no upstream test file
  covers `Invert` and only 4 of upstream's `CollisionResult2DTest.cs` facts translate 1:1 (the
  5th, `Default_ReturnsNonIntersectingResult`, folds into the default-constructor check — C++
  has no separate `default`-keyword-expression distinct from default construction for a struct).
- `Collision2D` — first of ~2 forked chunks. Originally scoped to just Constants/Helpers/
  Containment (lines 1–1487 of the upstream file), but the fork discovered and reported a real,
  verified cross-region dependency chain (confirmed myself by reading upstream directly, e.g.
  `ContainsConvexPolygonConvexPolygon` genuinely calls `IntersectsConvexPolygonConvexPolygon`)
  that made the originally-planned narrow scope impossible to port in isolation. Ended up
  covering: Constants, Helpers, all 5 Containment sub-regions, the full Projection region, the
  full Distance/ClosestPoint region, 2 of 3 Clipping-region methods (`ClipLineToAabb`/
  `ClipLineToConvexPolygon`, needed by Distance, not by the still-pending Ray Interval methods),
  and all 15 plain-`bool` `Intersects*` methods (but none of the `CollisionResult2D`-producing
  `TryGetCollision*` overloads, which nothing ported so far depends on) — 64 methods, 103 tests
  ported 1:1 from `Collision2DTest.cs`'s matching regions.
  **Independently re-verified before committing** (given the significant unrequested scope
  expansion, this got more scrutiny than a typical fork report): confirmed the
  `ContainsConvexPolygonConvexPolygon`/`IntersectsConvexPolygonConvexPolygon` dependency
  directly against upstream; spot-checked `DistanceSquaredSegmentSegment` (Ericson's classic
  closest-point-between-segments algorithm) line-by-line against upstream, exact match; ran
  `grep -oP` diffs of upstream method names vs. what's now in `Collision2D.hpp` to get the
  definitive remaining-method list myself (matches the fork's own count); did a genuinely clean
  `rm -rf build` + full rebuild of both CMake configs before trusting the reported pass count.
  **Zero upstream bugs found in this chunk** — unlike several Phase 1 Collections tasks, this
  algorithmic code translated cleanly.

**Remaining for `Collision2D` (verified via the diff above, not guessed)**: 5 methods —
`SolveParametricIntersectionWithImplicitLine`, `SolveParametricIntersection2D` (Parametric
Solvers region), `ClosestPointRaySegment`, `RayCircleIntersectionInterval`,
`RayCapsuleIntersectionInterval` (Ray Interval methods + their shared helper) — plus all 10
`TryGetCollision*(..., out CollisionResult2D)` overloads (`TryGetCollisionAabbAabb`,
`AabbConvexPolygon`, `AabbObb`, `ObbObb`, `ObbConvexPolygon`, `CircleCircle`, `CircleAabb`,
`CircleObb`, `CircleCapsule`, `ConvexPolygonConvexPolygon`). This is a much smaller remaining
scope than originally planned (roughly 800–1000 lines, not the ~2300 lines two more forks were
originally sized for) — likely just ONE more fork, not two.

**After `Collision2D` is fully done**: `CollisionShape2D` (713 lines) is the last piece of task
1 — it depends on the whole of `Collision2D` including the `TryGetCollision*` overloads (its own
`TryGetCollision(CollisionShape2D other, out CollisionResult2D result)` method dispatches to
them by shape-kind pair). No dedicated upstream test file for `CollisionShape2D` exists.

**Verification so far**: both build modes clean from a genuinely clean rebuild, zero new
warnings, `ctest` → **100% passed, 772/772** (was 669 before this task — 103 new, all from the
`Collision2D` chunk; `CollisionResult2D`'s 6 tests are already included in that count too via an
earlier direct-port sub-step, giving 669→772 as the net delta for this whole entry).

**Also landed this session, unrelated to Phase 2**: the user asked for in-code comments (not
just test-file comments) at every previously-found upstream bug's exact location. Added inline
comments (in addition to already-present header-comment explanations) to `ObjectPool.hpp`'s
`Use()` method and `Deque.hpp`'s `RemoveAt`'s two buggy shift branches. Found and fixed a real
gap: `RectangleF.Extensions.Clip`'s bug (mutates X/Y before deriving Width/Height from the
already-mutated values) was previously documented ONLY in `RectangleFExtensionsTests.cpp`'s
comments, with nothing in `RectangleFExtensions.hpp`/`.cpp` themselves — added matching
header-comment and inline documentation there too. This was a legitimate blind spot: every
*other* found bug already had both header + inline documentation; this one had fallen through
the cracks by only ever being written up in the test file. Worth double-checking for this same
gap (test-file-only documentation without matching header/inline documentation) whenever a
future bug is found and documented, not just trusting that "documented somewhere" means
"documented in the right place."

**State / next step**: launch the next fork for `Collision2D`'s remaining 15 methods
(Parametric Solvers + Ray Interval + all 10 `TryGetCollision*` overloads), verify it the same
way, then fork `CollisionShape2D`, then finalize `plan.md`'s task-1 checklist entry and decisions
log, commit, and push. Continue without pausing for a status update, per the standing
correction, unless a genuine blocker requiring the user's judgment comes up.

---

## 2026-07-13 (18) — Phase 1 test-suite parity pass; PHASE 1 COMPLETE (Phase 1 task 30)

Continued straight through, no check-in pause. **This was the last Phase 1 task.**

**Started with a direct personal audit** (not delegated) of all 16 upstream files under
`tests/MonoGame.Extended.Tests/{Math,Primitives,Shapes,Collections}` — the plan's literal
task wording. Confirmed 100% already covered by tests ported during each type's own
earlier implementation task.

**Then deliberately widened scope past that literal wording**: the task's actual purpose
is full Phase-1 test parity, which is broader than 4 named subfolders. A cheap `grep -c
'\[Fact\]\|\[Theory\]'` count comparison between each upstream root-level test file and
its ported counterpart surfaced a real, substantial gap: `BoundingBox2D`,
`BoundingCapsule2D`, `BoundingCircle2D`, `BoundingPolygon2D`, `LineSegment2D`, and
`OrientedBoundingBox2D` each had noticeably fewer tests than upstream actually has
(`OrientedBoundingBox2D` was the largest: 13 ported vs. 33 active upstream). Worth
remembering: when a task's literal scope and its evident purpose diverge, checking the
purpose-implied scope first is cheap and worth doing before declaring a task done on the
literal wording alone.

**Delegated the confirmed gap to a forked sub-agent** (7 files to check for real gaps, 8
more for a lighter verification pass), with explicit instructions to distinguish
"genuinely un-ported, portable now" tests from "correctly deferred to Phase 2 pending
`Collision2D`" tests by reading each type's own header-comment-documented deferral list
— not by guessing from test names — and to flag prominently (not paper over) any case
where a test needed functionality a header comment claimed was already ported but which
turned out not to actually exist.

**Result: +43 tests, no genuine discrepancies found.** Every header's documented
deferred-vs-ported split held up exactly. `BoundingBox2D` +4, `BoundingCapsule2D` +5,
`BoundingCircle2D` +8, `BoundingPolygon2D` +11, `LineSegment2D` +6,
`OrientedBoundingBox2D` +9. `HslColor` had no real gap (a `[Theory]`-vs-`TEST_P` counting
artifact, not a coverage gap). Every remaining un-ported test in these files genuinely
depends on `Collision2D` (not yet ported) or has no C++-translatable concept. The lighter
pass on 8 more files (`Angle`, `ColorExtensions`, `ColorHelper`, `Line2D`,
`MathExtended`, `Ray2D`, `RectangleExtensions`, `Vector2Extensions`) confirmed all
already at parity or better. `OrthographicCameraTests.cs` (53 upstream tests)
intentionally excluded — `OrthographicCamera` itself remains correctly deferred to
Phase 3.

**Independently re-verified before landing** (not just trusting the fork's self-report):
re-read 2 of the 6 modified files' diffs against the actual upstream `.cs` source
line-by-line, including the most calculation-heavy new tests (`OrientedBoundingBox2D`'s
`CreateFromRotation90Degrees`/`TransformNonUniformScale`) — all matched exactly. Then did
a genuinely clean `rm -rf build` + full rebuild of both CMake configurations myself
before trusting the reported pass count.

**Verification**: both build modes clean from a genuinely clean rebuild, zero new
warnings, `ctest` → **100% passed, 663/663** (was 624 — 39 net new tests).

## PHASE 1 IS COMPLETE

All 30 tasks landed, from `Version` through this test-parity pass — spanning ~50
forked-and-direct porting tasks, roughly 130 ported/created source files, and a test
suite that grew from 0 to 663 passing tests. Four genuine upstream bugs were found and
faithfully preserved along the way, never silently fixed:
1. `Segment2.SquaredDistanceTo`'s missing `return` for points beyond the segment's End.
2. `RectangleF.Extensions.Clip`'s mutate-X/Y-before-deriving-Width/Height ordering quirk.
3. `FramesPerSecondCounter.UpdateOrder`'s setter raising the wrong event (`EnabledChanged`
   instead of `UpdateOrderChanged`).
4. `ObjectPool<T>`'s severe self-referencing-node infinite-loop bug in `GetEnumerator()`.

Plus two confirmed-and-preserved `Deque<T>` correctness bugs (`IndexOf`'s not-found
handling, `RemoveAt`'s middle-index shift logic) — found by a fork and independently
re-verified by hand-tracing before being trusted.

**State / next step:** Phase 2 ("Collisions 2D") is next per `plan.md` §5 — first task:
root types `Collision2D`, `CollisionResult2D`, `CollisionShape2D`, `CollisionShapeKind2D`.
This phase unblocks the large cluster of `Intersects`/`Contains`/`TryGetCollision`
methods deferred throughout Phase 1 across nearly every bounding-volume and primitive
type — expect a "follow-up sweep" pattern similar to the `PrimitivesHelper` task (22) once
`Collision2D` itself lands: grep the tree for "Collision2D" in header deferral comments
and land every genuinely-unblocked follow-up in the same pass, verifying each by reading
the actual dependency (not assuming from a type's name). Continue without pausing for a
status update, per the standing correction, unless a genuine blocker requiring the user's
judgment comes up. **Commit AND push to `develop`** after landing work, as always.

---

## 2026-07-13 (17) — Collections: KeyedCollection, ListExtensions ported; DictionaryExtensions intentionally skipped (Phase 1 task 29)

Continued straight through, no check-in pause.

**Ported directly** (no fork; small, 106 lines of C# total across 3 files).
`KeyedCollection<TKey, TValue>` and `ListExtensions.Shuffle` fully ported as header-only
templates.

**`DictionaryExtensions.GetValueOrDefault` was deliberately NOT ported.** Verified (not
assumed) that `sharp-runtime`'s own `Dictionary<TKey, TValue>::GetValueOrDefault(key,
defaultValue = TValue{})` already implements byte-for-byte identical semantics to
upstream's extension method. This is the mirror image of the `RandomExtensions::
NextSingle` situation from task 21 — there, a same-named `sharp-runtime` method turned
out to implement a *different* algorithm and had to be re-ported to preserve fidelity;
here, the check came back a true match, so "reuse, don't re-roll" applies cleanly and no
port was needed. Worth remembering: a same-named `sharp-runtime` method is never
automatically reusable OR automatically to-be-avoided — check the actual algorithm each
time.

`KeyedCollection<TKey, TValue>` is backed by that same `Dictionary<TKey, TValue>` (whose
`operator[](key) const` already throws `KeyNotFoundException` for a missing key, exactly
matching C#'s `Dictionary` indexer). `CopyTo` matches upstream: always throws.
`ListExtensions.Shuffle`'s `IList<T>` parameter maps to `std::vector<T>&` and correctly
returns a reference to the same, now-shuffled vector, preserving upstream's fluent
return.

**Test coverage**: no upstream tests exist for any of the 3 files — wrote fresh tests for
`KeyedCollection`/`Shuffle` (determinism, element-preservation, edge cases).

**Verification**: both build modes clean, `ctest` → **100% passed, 624/624** (was 608 —
16 net new tests).

**State / next step:** Phase 1 is 29 of 30 tasks in — only task 30 left
("Port `tests/MonoGame.Extended.Tests/{Math,Primitives,Shapes,Collections}` as
GoogleTest suites", i.e. a Phase 1 test-suite consolidation/parity pass). No known
blockers. Continue without pausing for a status update, per the standing correction,
unless a genuine blocker requiring the user's judgment comes up. **Commit AND push to
`develop`** after this task.

---

## 2026-07-13 (16) — Collections: ObjectPool<T>, Pool<T>, IPoolable, ItemEventArgs ported (Phase 1 task 28)

Continued straight through, no check-in pause.

**Ported directly** (no fork; small, 244 lines of C# total). All four fully ported, no
deferrals. `IPoolable::NextNode`/`PreviousNode` → `IPoolable*` (`ObjectPool<T>` casts to
`T*` wherever upstream does an unchecked `(T)node.NextNode` cast). `event Action<T>
ItemUsed`/`ItemReturned` → `System::MulticastAction<T*>` (the multicast-delegate type
extended in `sharp-runtime` earlier in this session, exactly for this kind of need).

**Found and preserved this session's fourth confirmed upstream bug, by far the most
severe — a genuine infinite loop, not just a wrong value.** Found this one directly
while hand-tracing `CreateObject()`/`Use()` myself during the port (not delegated, not a
fork's claim I had to re-verify): `CreateObject()` unconditionally sets `_tailNode = item`
as its last step, right before `New()` calls `Use(item)` on that same item. `Use()`'s
`if (_tailNode is null)` check — meant to detect "is this the pool's very first node" —
is therefore always false for every freshly-created item, so `Use()` always links the
item to point to **itself** (`item.PreviousNode == item`, `item.NextNode == item`). For a
pool's very first item, `GetEnumerator()`'s `while (node != null) { yield return node;
node = node.NextNode; }` then never terminates. Confirmed byte-for-byte against the C#
source (not assumed) before trusting it.

Given the bug is a literal infinite loop, took explicit precautions so the regression
tests demonstrating it can never accidentally hang the test suite: they assert on
`getNextNodeProperty()`/`getPreviousNodeProperty()` directly, never by iterating the pool
in the exact state that triggers the bug — with a prominent warning in both
`ObjectPool.hpp` and `ObjectPoolTests.cpp` so a future session doesn't "simplify" a test
into iteration and reintroduce a hang. Also re-ran the entire suite under `timeout 60`
as an explicit extra safety net before trusting a clean pass.

**Caught and fixed one of my own test-authoring mistakes**: initially assumed the
full-pool policy applies as soon as `TotalCount > Capacity` at the very next `New()`
call, but upstream's actual guard is `TotalCount <= Capacity` (allowing `Capacity + 1`
total creations before the policy ever triggers) — traced and corrected before
committing.

**Verification**: both build modes clean, `ctest` (under `timeout 60`) → **100% passed,
608/608, no hang** (was 589 — 19 net new tests).

**State / next step:** Phase 1 is 28 of ~30 tasks in. Next per `plan.md` §5 Phase 1: task
29, Collections: `KeyedCollection`, `DictionaryExtensions`, `ListExtensions`. No known
blockers. Continue without pausing for a status update, per the standing correction,
unless a genuine blocker requiring the user's judgment comes up. **Commit AND push to
`develop`** after this task.

---

## 2026-07-13 (15) — Collections: Bag<T>, Deque<T> ported (Phase 1 task 27)

Continued straight through, no check-in pause.

**Ported via a forked sub-agent** (~1053 lines of C# source + 456 lines of upstream
tests). First use of a new `CNA::Extended::Collections` sub-namespace. Both are
header-only templates.

**`Bag<T>` has yet another different license**: unlike everything ported so far,
`Bag.cs`'s own header credits a BSD-2-clause-style license from GAMADU.COM's C# port of
thelinuxlich's `artemis_CSharp` project — not MonoGame.Extended's usual MIT header.
Handled directly (not delegated): `artemis_CSharp` is still live (unlike
`nickgravelyn/Triangulator`), so fetched its actual source and confirmed the license text
matches word-for-word. Added a NEW `NOTICE.md` section for this ("Code directly derived
from other permissively-licensed (non-MIT) projects"), separate from the existing
MIT-project section, since BSD-2-clause is a genuinely different license family.

**`Deque<T>` implements `sharp-runtime`'s `IList<T>`**, matching how `sharp-runtime`'s
own `List<T>` does the same.

**The fork reported finding three genuine correctness bugs in upstream `Deque<T>`** —
not just fidelity/GC-hygiene quirks like everything found before this session. Given how
much bigger a claim "upstream has silent data-corruption bugs" is than anything found so
far, did NOT just trust the self-report: independently re-derived all three from scratch
with my own fresh concrete examples (not reusing the fork's), hand-computing physical-
array and logical-view state at every step against the actual upstream `.cs` source.
1. `IndexOf`'s formula doesn't check for "not found" before applying modulo arithmetic —
   for a genuinely-absent item, this can make `Remove` either throw unexpectedly or
   **silently remove an unrelated real element while reporting success**, depending on
   the buffer's wraparound offset. A never-grown `Deque` also hits a literal
   divide-by-zero here (C#: catchable exception; C++: UB, so explicitly guarded instead).
2. & 3. `RemoveAt`'s middle-index removal is only reliable in one of three practical
   cases: front-half-shift on a non-wrapped buffer works; **back-half-shift is broken
   even unwrapped**; **front-half-shift is ALSO broken once wrapped** (an element is
   silently lost, replaced by a stale default value). Not caught by upstream's own tests,
   which only check `Count`, never the resulting values.

All three independently confirmed exactly as claimed — my hand-computed expected values
matched the fork's test assertions precisely, down to the specific post-removal element
sequences. Preserved exactly (not fixed), each with a dedicated regression test.

**Iteration needed real care**: upstream's enumerator re-reads live state on every
loop-condition check (there's an upstream test specifically for removing-from-front
during iteration) — a naive one-time-snapshot C++ translation wouldn't reproduce this.
Solved via a physical-index formula re-derived from live state every step.

**Test coverage**: all 17 active upstream `DequeTests.cs` tests ported 1:1, plus 24 fresh
tests (including the 3 bug regressions and live-iteration behavior). `Bag<T>`'s one
upstream test is a C#-GC-boxing-allocation benchmark with no C++ equivalent — not
ported; 19 fresh tests added instead.

**Verification**: did a genuinely clean `rm -rf build` + full reconfigure/rebuild myself
(not reusing the fork's already-built directory) for both CMake configurations before
trusting the reported pass count. Both clean, zero new warnings, `ctest` → **100%
passed, 589/589** (was 529 — 60 net new tests).

**State / next step:** Phase 1 is 27 of ~30 tasks in. Next per `plan.md` §5 Phase 1: task
28, Collections: `ObjectPool<T>`, `Pool<T>`, `IPoolable`, `ItemEventArgs`. No known
blockers. Continue without pausing for a status update, per the standing correction,
unless a genuine blocker requiring the user's judgment comes up. **Commit AND push to
`develop`** after this task.

---

## 2026-07-13 (14) — SimpleGameComponent, SimpleDrawableGameComponent ported (Phase 1 task 26)

Continued straight through, no check-in pause.

**Ported directly** (no fork; ~127 lines of C# total). Lighter-weight abstract bases that
don't need a `Game&` (unlike CNA's own `GameComponent`/`DrawableGameComponent`), each
implementing multiple CNA interfaces directly (`IGameComponent`, `IUpdateable`,
`System::IDisposable`, `System::IComparable<GameComponent>`,
`System::IComparable<SimpleGameComponent>` for the first; `+IDrawable` for the second).

**First time this port had to handle C#'s explicit interface implementation.** Upstream
uses it twice: `bool IUpdateable.Enabled => _isEnabled;` alongside a public `IsEnabled`
property, and `bool IDrawable.Visible => _isVisible;` alongside a public `Visible`
property. Decided the translation case-by-case rather than one blanket rule:
- `IsEnabled`/`Enabled` genuinely differ in name upstream → ported as two distinct C++
  members: a public `getIsEnabledProperty()`, plus a **private** override of
  `IUpdateable::getEnabledProperty()` — legal C++, since access specifiers gate name
  lookup, not virtual dispatch, so the private override still gets called correctly
  through an `IUpdateable&` reference. This is the closest C++ analog to "accessible
  only through the interface."
- `Visible`/`IDrawable.Visible` share the *identical* name upstream — nothing distinct
  left to preserve — so collapsed to one public `getVisibleProperty()` override
  satisfying `IDrawable` directly. Both C# members always read the same backing field in
  both cases regardless, so this naming/visibility collapse changes no observable
  behavior.

**Worth remembering for any future explicit-interface-implementation case**: check
whether upstream gave the two members different names before deciding between a
two-member (name-preserving) or one-member (collapsed) C++ translation.

**Test coverage**: no upstream tests exist for either file — wrote fresh tests via
minimal concrete test subclasses (both types are abstract), covering
enabled/visible/update-order/draw-order change events (raised only when the value
actually changes), `Initialize()`/`Dispose()` idempotency, and `CompareTo` ordering.

**Verification**: both build modes clean, `ctest` → **100% passed, 529/529** (was 516).

**State / next step:** Phase 1 is 26 of ~30 tasks in. Next per `plan.md` §5 Phase 1: task
27, Collections: `Bag<T>`, `Deque<T>`. No known blockers. Continue without pausing for a
status update, per the standing correction, unless a genuine blocker requiring the
user's judgment comes up. **Commit AND push to `develop`** after this task.

---

## 2026-07-13 (13) — FramesPerSecondCounter, FramesPerSecondCounterComponent ported (Phase 1 task 25)

Continued straight through, no check-in pause.

**Ported directly** (no fork; ~100 lines of C# total). First `cna-extended` type to
implement CNA's `IUpdateable` interface and derive from `System::Object` — closely
followed CNA's own `GameComponent.hpp`/`.cpp` pattern (public `EventHandler<EventArgs>`
members, `getXChangedEvent()` accessor overrides, `Raise(this, EventArgs::Empty)`).

**Third confirmed upstream bug this session, preserved not fixed**:
`FramesPerSecondCounter.UpdateOrder`'s setter raises `EnabledChanged` instead of
`UpdateOrderChanged` — a copy-paste error from the `Enabled` setter directly above it in
`FramesPerSecondCounter.cs`. Reproduced exactly, documented prominently in
`FramesPerSecondCounter.hpp`'s header comment, covered by an explicit regression test
that names the discrepancy (`UpdateOrderSetterRaisesEnabledChangedNotUpdateOrderChangedReproducesKnownUpstreamBug`).

**`FramesPerSecondCounterComponent`** (a `DrawableGameComponent` subclass, just forwards
`Update`/`Draw` to an internal `FramesPerSecondCounter`) needs a live `Game&` to
construct — no dedicated test file, matching a precedent CNA itself already set:
`cna`'s own `tests/Microsoft/Xna/Framework/DrawableGameComponentTests.cpp` literally says
"No tests: DrawableGameComponent requires a live Game and GraphicsDevice (SDL/GPU)." All
the independently-testable logic lives in `FramesPerSecondCounter` itself, which is fully
unit tested.

**Caught my own test-authoring bug before landing**: an initial test assumed the internal
timer starts at zero seconds, but it actually starts pre-loaded at a full second (matches
upstream's `_timer = _oneSecondTimeSpan;` field initializer) — so the very first
`Update()` call always latches a `FramesPerSecond` reading immediately, no matter how
little time elapsed. Hand-traced the arithmetic to confirm this is genuine upstream
behavior, not a fidelity bug in the port, then rewrote the test to assert the real
behavior.

**Verification**: both build modes clean, `ctest` → **100% passed, 516/516** (was 507).

**State / next step:** Phase 1 is 25 of ~30 tasks in. Next per `plan.md` §5 Phase 1: task
26, `SimpleGameComponent` + `SimpleDrawableGameComponent`. No known blockers. Continue
without pausing for a status update, per the standing correction, unless a genuine
blocker requiring the user's judgment comes up. **Commit AND push to `develop`** after
this task.

---

## 2026-07-13 (12) — GameTimeExtensions, GameComponentCollectionExtensions ported (Phase 1 task 24)

Continued straight through, no check-in pause.

**Ported directly** (no fork; tiny, ~36 lines of C# total). `GameTimeExtensions.GetElapsedSeconds`
→ a free function taking `const GameTime&`. `GameComponentCollectionExtensions`'s two
`Add<T>` overloads → free function templates taking `GameComponentCollection&`,
returning `T*` — matches `GameComponentCollection`'s own design in CNA (confirmed by
reading its `.cpp`: it never `delete`s its stored `IGameComponent*` items, a non-owning
collection; caller owns component lifetime, the natural translation of C#'s GC-owned
reference-type semantics into explicit pointers). `Func<T>` → `std::function<T*()>`,
matching this project's established convention (`HslColor.hpp`'s `Match`/`Map`).

**Test coverage**: no upstream tests for either file — wrote fresh tests (elapsed-seconds
conversion + zero case; both `Add<T>` overloads, checking collection membership and
count).

**Verification**: both build modes clean, `ctest` → **100% passed, 507/507** (was 503).

**State / next step:** Phase 1 is 24 of ~30 tasks in. Next per `plan.md` §5 Phase 1: task
25, `FramesPerSecondCounter` + `FramesPerSecondCounterComponent`. No known blockers.
Continue without pausing for a status update, per the standing correction, unless a
genuine blocker requiring the user's judgment comes up. **Commit AND push to `develop`**
after this task.

---

## 2026-07-13 (11) — Math/Triangulation ported (Phase 1 task 23)

Continued straight through, no check-in pause.

**Ported via a forked sub-agent** (6 upstream files, ~870 lines, an ear-clipping
triangulation algorithm) — `Vertex`, `LineSegment`, `Triangle`, `CyclicalList<T>`,
`IndexableCyclicalLinkedList<T>`, `Triangulator` + `WindingOrder` enum, all in a new
`CNA::Extended::Triangulation` sub-namespace (mirrors the `Shapes` sub-namespace
precedent from the prior task). No deferrals.

**License note — handled directly, not by the fork**: unlike almost every other file
ported so far (which just cite an algorithm source, e.g. "Real-Time Collision
Detection"), all 6 of these files carry `MIT Licensed:
https://github.com/nickgravelyn/Triangulator` in their own headers — the same "code
actually derived from a different MIT project" situation as `Angle.cs`/SlimMath. Did the
license research myself before delegating the port: the original repo 404s on GitHub
(both web UI and API), no mirror/archive/renamed-account found. Added a `NOTICE.md` entry
using the standard MIT template with an explicit provenance caveat (documented what's
confirmed — MIT, per 6 consistent upstream file headers plus web search corroboration —
vs. what couldn't be verified — the exact original copyright line). Gave the fork the
exact SPDX header text to use so it didn't have to make that call itself.

**Design highlight**: `CyclicalList<T>`/`IndexableCyclicalLinkedList<T>` are internal-only
(upstream's own doc comment: `Triangulator` is "the sole public class in the entire
library"). Upstream implements them via C#'s `new`-keyword method-hiding on `List<T>`/
`LinkedList<T>` subclasses — the fork correctly judged that mechanism not worth
replicating in C++ for a type with zero public API surface, and instead composed (not
inherited) `sharp-runtime`'s `List<T>`/`LinkedList<T>`, adding only the cyclical
`operator[]`/`RemoveAt`/`IndexOf`. Good precedent for any future C#-collection-subclass
type.

**Critical fidelity point preserved**: upstream's `Triangulator` has 5 `static readonly`
mutable buffers shared across every call (explicit upstream design tradeoff for reduced
GC pressure, not an accident) — ported as genuine `static inline` C++ class members, not
locals/`thread_local`, keeping `Triangulate`/`CutHoleInShape` non-reentrant/not-
thread-safe exactly like upstream. Documented prominently in `Triangulator.hpp`.

**Independent verification before committing** (per this session's standing rule —
never just trust a fork's self-report): re-grepped all 6 upstream `.cs` files for
`public |internal ` members against the ported headers (nothing dropped — confirmed
`Equals`/`GetHashCode`/`ToString`/operators present where upstream has them, absent
where it doesn't, e.g. `Triangle` genuinely has no `ToString` upstream); hand-traced
`CutHoleInShape` (the trickiest method — nullable-comparison logic, cyclical-index
injection) line-by-line against the C# source and found it faithful; did a clean
`rm -rf build` + full reconfigure/rebuild myself (not trusting the fork's already-built
`build/`), confirming zero new compiler warnings and 503/503 `ctest` from scratch.

**Test coverage**: upstream's one active test file (`TriangulatorTests.cs` — 4
`DetermineWindingOrder` tests, including a shoelace-formula regression test for issue
#791) ported 1:1. Fresh tests for everything else (no upstream coverage exists for
`Triangulate`, `CutHoleInShape`, `EnsureWindingOrder`, `ReverseWindingOrder`, or any of
`Vertex`/`LineSegment`/`Triangle`/the two cyclical collection types).

**Verification**: both build modes clean, zero warnings, `ctest` → **100% passed,
503/503** (was 464 before this task — 39 net new tests).

**State / next step:** Phase 1 is 23 of ~30 tasks in. Next per `plan.md` §5 Phase 1: task
24, `GameTimeExtensions`, `GameComponentCollectionExtensions`. No known blockers.
Continue without pausing for a status update, per the standing correction, unless a
genuine blocker requiring the user's judgment comes up. **Commit AND push to `develop`**
after this task.

---

## 2026-07-13 (10) — PrimitivesHelper, ShapeExtensions ported; big follow-up sweep (Phase 1 task 22)

Continued straight through, no check-in pause.

**Ported directly** (no fork; ~145 lines, fully self-contained) — `PrimitivesHelper` (an
upstream-`internal` static utility class: `IntersectsSlab`, `CreateRectangleFromPoints`,
`TransformRectangle`, `TransformOrientedRectangle`, `SquaredDistanceToPointFromRectangle`,
`ClosestPointToPointFromRectangle`). No deferrals.

**`ShapeExtensions` deferred in full to Phase 5** — it's pure `SpriteBatch` debug-drawing
code (`DrawPolygon`/`DrawLine`/`DrawCircle`/etc.), not a math utility, despite living in
upstream's `Math/` folder.

**Gap found and closed while scoping `ShapeExtensions`**: it depends on
`MonoGame.Extended.Shapes.Polygon`/`Polyline` (`source/MonoGame.Extended/Shapes/`), an
entire upstream folder never tracked anywhere in `plan.md`. Both types were small and
self-contained (only need `Vector2`/`RectangleF`, already ported), so ported them here
rather than leaving the gap open — first use of a `CNA::Extended::Shapes` sub-namespace.

**The big part of this task: a verified follow-up sweep**, not assumption-based. Landing
`PrimitivesHelper` was flagged across several earlier sessions as *the* recurring blocker.
Rather than trusting those earlier notes, re-verified each flagged type by reading its
current header's deferral comment plus the actual upstream `.cs` source before touching
any code:
- **Confirmed still blocked (left deferred)**: `Line2D`/`LineSegment2D`'s `Intersects`/
  `DistanceSquared*` overloads — these need `Collision2D` (a different, larger Phase-2
  type), not `PrimitivesHelper`, despite both citing overlapping "Real-Time Collision
  Detection" algorithm sources. Don't assume these are unblocked next time either, without
  re-checking against `Collision2D` landing specifically.
- **Confirmed unblocked and landed**: `RectangleF` (`Transform` x2, `CreateFrom(points)`
  x2, `UpdateFromPoints`, `SquaredDistanceTo`, `DistanceTo`, `ClosestPointTo`);
  `BoundingRectangle` (the equivalent set, `Transform` faithfully preserving upstream's
  mutate-input-in-place semantics); `OrientedRectangle` (`getBoundingRectangleProperty()`,
  `static Transform`, `operator RectangleF()` — upstream's `private` ref-taking Transform
  overload isn't public API, inlined into the public one instead).
- **Widened the sweep further** by grepping the whole tree for "PrimitivesHelper" in
  deferral comments (not just the types named in prior notes) and found 2 more:
  `CircleF::Intersects(CircleF, BoundingRectangle)` and
  `Segment2::Intersects(RectangleF|BoundingRectangle, out Vector2)`. Landed both too.

**Testing nuance found and preserved (not a code bug)**: porting `OrientedRectangle`'s
upstream `Transform` test class 1:1, 2 of the 9 tests failed even though the underlying
math was correct. Root cause: those 2 upstream tests use upstream's own
`CollectionAssert.Equal` test helper, which does an order-*insensitive* containment check,
not a sequence comparison like gtest's `EXPECT_EQ` on a `std::vector` — upstream's own
hand-written expected point order doesn't actually match its own `Points` getter's
algorithmic order either; it only passes upstream because the helper ignores order. Fixed
by adding a small `ExpectUnorderedPointsEqual` helper replicating that exact upstream
semantics for just those 2 tests, rather than silently reordering the expected values.
**Lesson for future test ports**: a literal `EXPECT_EQ` translation of a C# assertion can
be *stricter* than what the C# test actually checks if the C# side used a custom
assert-helper — check the helper's actual semantics, not just its call syntax, before
assuming a failing ported test means the ported code is wrong.

**Test coverage**: `RectangleFTests.cpp` gained upstream's 2 constructor tests + 5
`Transform` tests from the previously-unported `Primitives/RectangleFTests.cs` (that file's
`Rectangle_Intersects_Test` intentionally NOT ported — it exercises base XNA/FNA
`Rectangle.Intersects`, not a MonoGame.Extended addition), plus fresh
`CreateFrom(points)`/`UpdateFromPoints`/distance tests. `BoundingRectangleTests.cpp` and
`CircleFTests.cpp`/`Segment2Tests.cpp` gained fresh tests (upstream's own coverage for
`BoundingRectangle`, `CircleF`-vs-`BoundingRectangle`, and `Segment2` is entirely commented
out in its own test suite). `OrientedRectangleTests.cpp` gained upstream's full 9-test
`Transform` class. `PrimitivesHelperTests.cpp`/`PolygonTests.cpp`/`PolylineTests.cpp` are
new files.

**Verification**: both build modes clean, `ctest` → **100% passed, 464/464** (was 429
before this task — 35 net new tests).

**State / next step:** Phase 1 is 22 of ~30 tasks in. Next per `plan.md` §5 Phase 1: task
23, `Math/Triangulation/*` (polygon triangulation helpers). No known blockers for it.
Continue without pausing for a status update, per the standing correction, unless a genuine
blocker requiring the user's judgment comes up. **Commit AND push to `develop`** after this
task.

---

## 2026-07-13 (9) — FastRandom, RandomExtensions ported (Phase 1 task 12)

Continued straight through, no check-in pause.

**Ported directly** (~410 lines across 6 files, no fork needed) — `FastRandom` (a
linear-congruential PRNG class with a bridge/strategy-pattern internal design: private
`IFastRandomImpl` interface, `LinearCongruentialGeneratorImpl`, and a `[ThreadStatic]`-
backed `ThreadSafeFastRandomImpl` behind the `Shared` static instance) and
`RandomExtensions` (free functions extending `System::Random`). No deferrals — fully
self-contained.

**Design decisions worth remembering**:
- `Shared`'s lazy-initialized C# static property became a function-local static (Meyer's
  singleton) rather than a namespace-scope `static const`/`static` object — a deliberate
  choice to avoid a repeat of the `Matrix3x2::Identity` cross-translation-unit
  static-init-order bug from two tasks ago.
- **Deliberately did NOT reuse `sharp-runtime`'s own `System::Random::NextSingle()`** for
  `RandomExtensions::NextSingle`, even though "reuse sharp-runtime, don't re-roll" is the
  usual rule here. Checked and confirmed: `sharp-runtime`'s `NextSingle()` already exists
  and is used elsewhere, but implements a *different* (more modern, real-.NET-6+-matching)
  algorithm than what `RandomExtensions.cs` itself actually does (a plain
  `(float)NextDouble()` cast). Using the "better" existing method would have silently
  changed behavior from what this specific upstream file does — the fidelity mandate
  wins over the reuse-don't-re-roll convenience rule when they conflict like this.
  **Worth checking for this same conflict pattern in any future task**: before reusing an
  existing sharp-runtime method just because the name matches, verify its actual
  algorithm matches what upstream MonoGame.Extended does, not just that it does something
  broadly similar.

**Test coverage**: no upstream tests exist for either file — wrote fresh tests covering
deterministic seeding (same seed → same sequence), range bounds for every `Next`/
`NextSingle` overload, `NextAngle`'s `[-pi, pi]` range, `NextUnitVector`'s unit-length
guarantee, and the `Shared` singleton property (same instance returned every call).

**Verification**: both build modes clean on the first try, `ctest` → **100% passed,
410/410** (was 389 before this task).

**State / next step:** Phase 1 is 12 of ~20 tasks in (13 including the `OrientedRectangle`
follow-up). Next per `plan.md` §5 Phase 1: `PrimitivesHelper`, `ShapeExtensions`. **This
one matters a lot** — `PrimitivesHelper` specifically has been the recurring blocker
behind nearly every deferral left in Phase 1 so far (`RectangleF`/`BoundingRectangle`'s
`Transform`/`CreateFrom(points)`/`SquaredDistanceTo`, `OrientedRectangle`'s `Transform`/
`BoundingRectangle`/`RectangleF` conversion, `CircleF`'s `Intersects(BoundingRectangle)`,
`Segment2`'s `Intersects`/some `Distance*` overloads). **After this task lands, do a full
sweep** (grep the tree for "PrimitivesHelper" in header comments — there should be a
concentrated cluster) and land every genuinely-unblocked follow-up, likely across several
files in one pass, the same way the `SizeF` and `Matrix3x2` tasks did. This could be a
substantial cleanup task on its own. Continue without pausing for a status update, per
the standing correction, unless a genuine blocker requiring the user's judgment comes up.
**Commit AND push to `develop`** after this task.

---

## 2026-07-13 (8) — OrientedRectangle follow-up picked up immediately; corrected a prior report

Continued straight through from task 11 without a check-in pause — picked up the
`OrientedRectangle` follow-up flagged at the end of the last entry, since it was now
small and well-scoped. Ported directly (no fork needed at this point — I already had
full context on the file from scoping it).

**Correction found while re-scoping** (worth remembering as its own lesson, separate from
the fork-report-verification lessons already documented): the previous task's fork report
said only `OrientedRectangle::Transform` was blocked on `PrimitivesHelper`. Reading the
actual C# source myself before porting found that's incomplete — `BoundingRectangle` (the
property, `=> (RectangleF)this`) and `explicit operator RectangleF(OrientedRectangle)`
are **also** transitively blocked, since both call `RectangleF::Transform` internally,
which is itself still blocked. All three deferred together now, documented in
`OrientedRectangle.hpp`'s header comment. **Lesson**: re-verify "only X is blocked"
claims by reading the dependency chain yourself before starting a follow-up task, even
when the claim comes from this session's own prior notes, not just from a fresh fork
report — carried-forward summaries can be incomplete too.

**Ported**: `Center`/`Radii`/`Orientation` fields, constructor, `Points`, `Position`
(getter works; setter throws `std::logic_error`, matching upstream's
`NotImplementedException` — a real "always throws" API preserved faithfully, not
softened), `Equals`/`GetHashCode`/`ToString`/operators, the `OrientedRectangle(RectangleF)`
converting constructor (doesn't need `Transform`), and the self-contained SAT
`Intersects(OrientedRectangle, OrientedRectangle)`. C#'s named-tuple return
`(bool Intersects, Vector2 MinimumTranslationVector)` became a small named
`OrientedRectangleIntersection` struct — closer to upstream's actual named-field
semantics than an unnamed `std::pair`.

**Test coverage**: upstream's `Initializes_oriented_rectangle` and `Equals_comparison`
tests ported 1:1. The entire nested `Transform` test class (11 tests) is inapplicable —
every one of them exercises the deferred `Transform` method. Wrote fresh tests for
`Position`, the `RectangleF` conversion, and `Intersects` (no upstream coverage of that
SAT algorithm specifically either way).

**Verification**: both build modes clean on the first try (no build/test iteration needed
this time), `ctest` → **100% passed, 389/389** (was 380 before this task).

**State / next step:** Phase 1 is now 11 of ~20 tasks complete per the numbered task list,
plus this OrientedRectangle follow-up. Next per `plan.md` §5 Phase 1: `FastRandom`,
`RandomExtensions`. Continue without pausing for a status update, per the standing
correction, unless a genuine blocker requiring the user's judgment comes up. **Commit AND
push to `develop`** after this task, same as every task since the workflow change.

---

## 2026-07-13 (7) — Matrix3x2 ported (Phase 1 task 11); Transform2 landed as a bonus; second bug found

Continued straight through, no check-in pause.

**Forked** (~1375 lines of C# — Matrix3x2 alone is 1037). **Ported**: `Matrix3x2` (fully
self-contained, no deferrals), `MatrixExtensions`, `Vector2Extensions` — ~1300 lines
across 9 new files.

**Bonus follow-up landed in the same pass: `Transform2` is now fully implemented**
(`Transform.hpp`/`.cpp`) — this was deferred all the way back at task 2 of this phase,
specifically waiting on `Matrix3x2::CreateScale/CreateRotationZ/CreateTranslation/
Multiply/Decompose`. Both constructors, `IMovable`/`IRotatable`/`IScalable`
implementation (an asymmetry vs. `Transform3`, which implements none of those — preserved
faithfully, not "fixed" for consistency), the two `RecalculateLocalMatrix`/
`RecalculateWorldMatrix` overrides, `ToString`. 8 new tests in `TransformTests.cpp`
(`Transform2Tests.*`), including one specifically exercising the `IMovable`/`IRotatable`/
`IScalable` interface implementation.

**Other follow-up spots — precise findings, not guesses**:
- `RectangleF`/`BoundingRectangle`'s `Transform(...)`: confirmed **still blocked** —
  needs `PrimitivesHelper.TransformRectangle`, Matrix3x2 alone isn't enough.
- `CircleF`/`Segment2`'s remaining deferrals: confirmed unrelated to Matrix3x2
  (`PrimitivesHelper`-only).
- **`OrientedRectangle` — blocker status changed, worth flagging clearly**: its
  constructor and `Orientation` field only need `SizeF`+`Matrix3x2`, both now available.
  Only its `Transform` method still needs `PrimitivesHelper.TransformOrientedRectangle`.
  **Good candidate for a small near-term follow-up task**: port everything except
  `Transform` (narrow deferral, matching the pattern used everywhere else in this phase),
  rather than leaving it as a whole-type deferral with zero files. Not done this pass —
  explicitly flagged for the next session/task rather than attempted under time pressure.

**Second confirmed bug this session — this one is cna-extended's own, not upstream's**:
`Matrix3x2::Identity` was built as `Matrix3x2(Vector2::UnitX, Vector2::UnitY,
Vector2::Zero)` — a classic C++ static-initialization-order fiasco, since those are
static objects in a *different* translation unit (`Vector2.cpp`) with unspecified
init-order relative to `Matrix3x2.cpp`. `Identity` was silently all-zero at runtime.
Caught by 2 failing tests after an otherwise-green build (`IdentityHasNoEffectOnTransform`,
`MultiplyByIdentityIsUnchanged`) — a good reminder that "the build is green" and "the
tests pass" are different checkpoints, both matter. Fixed with literal float values
(`1,0,0,1,0,0`) instead, removing the cross-TU dependency entirely. The fork proactively
checked the other 4 similar `static const X::Empty`-style members already in the tree
(`RectangleF`, `BoundingRectangle`, `SizeF`, `Size`) and confirmed none has this pattern —
worth re-checking this specific hazard on any *future* `static const Type X = Type(other
statics...)` declaration, not just this one.

**Verification**: both build modes clean, `ctest` → **100% passed, 380/380** (was 343
before this task).

**State / next step:** Phase 1 is 11 of ~20 tasks in. Next per `plan.md` §5 Phase 1:
`FastRandom`, `RandomExtensions`. Before that, or as its own quick task, **consider
picking up the `OrientedRectangle` (minus `Transform`) follow-up flagged above** — it's
now small and well-scoped, and closes out a loose end from several tasks ago rather than
letting it linger. Continue without pausing for a status update, per the standing
correction, unless a genuine blocker requiring the user's judgment comes up. **Commit AND
push to `develop`** after every task (per the user's mid-session instruction — this is
now the standing workflow, not just for this task).

---

## 2026-07-13 (6) — Size/SizeF/Interval/Thickness ported (Phase 1 task 10); pushing to GitHub, new `develop` branch

Continued straight through, no check-in pause.

**Repo workflow change mid-session, from the user directly**: asked to push the work so
far to GitHub (`git push origin master` — done, `master` now exists on `origin`), then
asked to create and push a new `develop` branch (`git checkout -b develop && git push -u
origin develop` — done, tracking set up). **Going forward: commit AND push after every
task, to `develop`, not just commit.** This supersedes the earlier "one task = one local
commit" note — now it's "one task = one commit + push to origin/develop."

**Ported (~1450 lines across 16 files)**: `Size`, `SizeF`, `Interval<T>` (header-only
template), `Thickness` — **no deferrals**, nothing in these 4 files depends on anything
still unported. `Interval<T>`'s upstream `where T : IComparable<T>` constraint has no C++
analogue for primitives; translated to plain `<`/`==` comparisons.

**All three `SizeF`-blocked follow-ups (flagged explicitly in the fork prompt from the
previous session's NEXT.md note) verified genuinely unblocked and landed**:
1. `ISizable.hpp` — forward-declare → real `#include`, round-trip test added.
2. `RectangleF` — `Size` property + `RectangleF(Vector2, SizeF)` constructor added.
3. `BoundingRectangle` — `(Vector2, SizeF)` constructor + both implicit conversions from
   `Rectangle`/`RectangleF` added.
`RectangleF`'s/`BoundingRectangle`'s *other* deferrals (`Transform`, `CreateFrom(points)`,
`SquaredDistanceTo`) are still blocked on `Matrix3x2`/`PrimitivesHelper` — untouched,
unrelated to this task.

No upstream bugs found this time (unlike the previous task's confirmed `Segment2` bug).
Two of the fork's own draft bugs caught and fixed before the build even ran: duplicate/
ambiguous constructor overloads in both `Interval<T>` and `Thickness` (C#'s separate
constructor + implicit-conversion-operator pair collapses into a single C++ converting
constructor — declaring both separately is an ambiguous-overload compile error, not just
redundant) and a `Rectangle::Width`/`Height` field-vs-property mixup (only Left/Right/Top/
Bottom are properties on CNA's `Rectangle`; Width/Height are plain fields).

**Test coverage**: `Math/IntervalTests.cs` fully active, ported 1:1 (34 tests — first time
in several tasks a Math-folder test file had *no* `Collision2D` dependency at all).
`Primitives/Size2Tests.cs` entirely commented out upstream; no upstream `ThicknessTests.cs`
exists — wrote fresh tests for both.

**Verification**: both build modes clean, `ctest` → **100% passed, 343/343** (was 274
before this task).

**State / next step:** Phase 1 is 10 of ~20 tasks in — halfway through the phase's task
list. Next per `plan.md` §5 Phase 1: `Matrix3x2`, `MatrixExtensions`, `Vector2Extensions`.
**This is a big one and matters a lot**: `Matrix3x2` alone is ~1037 lines of C# and is the
single most-referenced blocker so far this phase — `Transform2` (deferred since task 2),
`OrientedRectangle` (deferred whole), and several remaining `RectangleF`/
`BoundingRectangle`/`CircleF`/`Segment2` members are all waiting on it. **After this task
lands, do a deliberate sweep** (grep the tree for "Matrix3x2" in header comments) and
land every genuinely-unblocked follow-up in the same pass, the same way the last two
tasks did for `SizeF`. This could reasonably be its own follow-up task if the sweep turns
up a lot — use judgment on whether to fold it into the same commit or split it.
**Commit AND push to `develop`** after this task (see the workflow change above).
Continue without pausing for a status update, per the standing correction, unless a
genuine blocker requiring the user's judgment comes up.

---

## 2026-07-13 (5) — CircleF, EllipseF, Segment2 ported (Phase 1 task 9); found a real upstream bug

Continued straight through, no check-in pause.

**Forked** (~930 lines of C# across 3 files). **Ported**: `CircleF`, `EllipseF` (fully
self-contained, zero deferrals), `Segment2` — ~860 lines across 9 new files. Small
deferrals in `CircleF` (4 `Intersects(CircleF, BoundingRectangle)` overloads) and
`Segment2` (2 `Intersects(RectangleF|BoundingRectangle, ...)` overloads), both blocked on
`PrimitivesHelper` (inherited blocker, not new this task). No whole-type deferral needed
this time.

**Found a genuine upstream bug, independently re-verified before trusting it**:
`Segment2.SquaredDistanceTo` (upstream `Segment2.cs:96`) has
```
if (dot >= startToEndDistanceSquared)
    endToPoint.Dot(endToPoint);   // <-- missing `return`!
```
For a point projecting beyond the segment's `End`, this silently falls through to the
perpendicular-distance formula below it instead of returning the (correct) end-point
distance — confirmed wrong (400 instead of the correct 800 for a documented test case).
Read the upstream line myself to confirm the fork's finding rather than just trusting the
report. **Ported this exactly as-is** — the project's explicit no-simplification mandate
means preserving upstream bugs, not silently correcting them. Documented prominently in
`Segment2.hpp`'s header comment, with a regression test that names both the actual
(bug-preserving) value and what the mathematically-correct value would be. **If this is
ever noticed again in a future session, do not "fix" it** — it's a deliberate, documented
fidelity choice. This is the first confirmed upstream bug found during this port; the same
handling rule applies to any future ones.

**Attribution check**: `CircleF.cs`/`Segment2.cs` cite "Real-Time Collision Detection,
Christer Ericson, 2005" in code comments — an academic algorithm citation (like citing a
textbook for the algorithm's origin), not a third-party code/license dependency the way
`Angle.cs`'s SlimMath credit was. No `NOTICE.md` change needed for this one.

**Test coverage**: same pattern as recent tasks — upstream's own test suite here is
unusually thin (`EllipseFTest.cs` fully active, ported 1:1; `CircleFTests.cs` has exactly
1 active test with the rest commented out; `Segment2DTests.cs` is entirely commented out
upstream). Ported what's actually active, wrote fresh tests for the rest, including the
bug-regression test above.

**Bugs the fork found and fixed via building/running (not review alone)**: CNA's
`Vector2::Dot` is `static`, not an instance method (`a.Dot(b)` doesn't exist, needed
`Vector2::Dot(a, b)`) — ~7 call sites fixed. Also caught two of its own fresh test bugs
(a "contains" test using a point that was actually outside the circle; the bug-regression
test's assertion initially backwards) — both are real "the test was wrong, not the port"
cases, same category as the RectangleF task's `Clip` test bug.

**Verification**: both build modes clean, `ctest` → **100% passed, 274/274** (was 230
before this task).

**State / next step:** Phase 1 is 9 of ~20 tasks in. Next per `plan.md` §5 Phase 1: `Size`,
`SizeF`, `Interval`, `Thickness`. This one matters more than most remaining tasks —
`SizeF` specifically is what's been blocking `ISizable`'s test, parts of `RectangleF`/
`BoundingRectangle`, and probably more once you check. **After this task lands, do a
sweep for "needs SizeF" deferral comments across the tree** (grep for "SizeF" in header
comments) and land whichever follow-ups are genuinely unblocked, the same way the
RectangleF task did for `IRectangularF`/`Camera<T>`. Continue without pausing for a
status update, per the standing correction, unless a genuine blocker requiring the user's
judgment comes up.

---

## 2026-07-13 (4) — RectangleF family ported (Phase 1 task 8), largest task since bounding volumes

Continued straight through, no check-in pause (per the standing correction).

**Forked again** (1858 lines of C# across 5 files — `RectangleF.cs`, `Rectangle.
Extensions.cs`, `RectangleF.Extensions.cs`, `BoundingRectangle.cs`, `OrientedRectangle.cs`)
with the accumulated lessons baked into the prompt: mandatory per-type `Equals`/
`GetHashCode`/`ToString`/operators/`Deconstruct` checklist, verify-don't-assume for
dependency unblocking, explicit ask to check two specific deferred follow-ups.

**Ported**: `RectangleF`, `RectangleExtensions`, `RectangleFExtensions`, `BoundingRectangle`
— 1313 lines (4 header/source pairs + 4 test files). **`OrientedRectangle` fully
deferred, no file created** — its `Orientation` field *is* `Matrix3x2` (not just used in a
method), a deep structural dependency like `OrthographicCamera`→`ViewportAdapter` from two
tasks ago, not a narrow one. `RectangleF`/`BoundingRectangle` both have several members
deferred on `SizeF`/`Matrix3x2`/`PrimitivesHelper` (documented per-header;
`plan.md`'s Phase 1 checklist has the full breakdown, don't duplicate it here).

**Both explicitly-requested follow-ups verified genuinely unblocked and landed**:
1. `IRectangularF` (`IRectangular.hpp`) — test added to `InterfaceTests.cpp`.
2. `Camera<T>` (`Camera.hpp`) — forward-declared `RectangleF` replaced with a real
   `#include`; `CameraTests.cpp`'s compile-only placeholder replaced with a full concrete
   `Camera<Vector2>` test double (8 real tests: position/zoom/move/look-at/bounding-
   rectangle/contains/world-screen round trip).
`ISizable` is still blocked (needs `SizeF`, next-but-one task) — not touched.

**Bugs the fork found and fixed via actually building/running (not just review)**:
1. CNA's `Rectangle` has no `.Left`/`.Right`/`.Top`/`.Bottom` fields, only
   `getLeftProperty()` etc. — fixed in `RectangleExtensions.cpp`.
2. `BoundingRectangle` ended up with zero declared constructors (only the SizeF-taking one
   was deferred) — resolved by deliberately keeping it a C++ aggregate (no user-declared
   ctor at all) rather than adding one upstream doesn't have; C++20 parenthesized-
   aggregate-init covers `BoundingRectangle()` and `BoundingRectangle(center, halfExtents)`
   both. Worth remembering as a pattern: when every upstream constructor is deferred, an
   aggregate can be the right (not just convenient) answer, if the fields are public and
   there's no invariant a constructor would need to enforce.
3. **A genuine test-authoring bug, not an implementation bug**: the fork's first-draft
   `RectangleFExtensionsTests` assumed `Clip` does proper min/max rectangle intersection.
   Upstream's actual `RectangleF.Extensions.Clip` mutates X/Y first, then derives Width/
   Height from the *already-mutated* X/Y — a real quirk (it doesn't even detect true
   non-overlap correctly) that the C++ port had faithfully replicated; only the test's
   assumptions were wrong. Fixed the test, documented the quirk, did **not** "fix" the
   port to be geometrically correct. Good reminder: when a test fails, check which side
   (implementation or test) is actually unfaithful to upstream before "fixing" anything.

**Test coverage**: upstream's own test suite for this area is unusually thin —
`Primitives/RectangleFTests.cs` is entirely blocked (every test needs the deferred `SizeF`
ctor or `Transform`), and `Primitives/BoundingRectangleTests.cs` is **commented out in
upstream itself**. Ported what's actually active and portable 1:1 (7 tests total, from
`Math/RectangleFTests.cs` + `RectangleExtensionsTests.cs`); wrote ~30 fresh tests for
everything else that's ported but untested/disabled upstream.

**Verification**: both build modes clean, `ctest` → **100% passed, 230/230** (was 189
before this task). Independently re-verified (not just trusted the fork's report):
`grep -n "public "` against `RectangleF.cs` confirmed the full member checklist,
`GetHashCode` present in both `RectangleF.hpp` and `BoundingRectangle.hpp`, spot-read
`BoundingRectangle.hpp` for the aggregate-vs-constructor judgment call.

**State / next step:** Phase 1 is 8 of ~20 tasks in. Next per `plan.md` §5 Phase 1:
`CircleF`, `EllipseF`, `Segment2`. **Check dependencies first as always** — given the
pattern so far, watch specifically for anything needing `SizeF`/`Matrix3x2`/
`PrimitivesHelper` (all three are recurring blockers this phase; `SizeF` is 2 tasks away,
`Matrix3x2` is 3 tasks away — `CircleF`/`EllipseF`/`Segment2` may hit the same wall
`OrientedRectangle` did). Continue without pausing for a status update, per the standing
correction, unless a genuine blocker requiring the user's judgment comes up.

---

## 2026-07-13 (3) — MathExtended, FloatHelper, Angle ported (Phase 1 task 7)

Continued straight through per the correction in session (2) — no check-in pause this
time.

**Ported directly (~315 lines)**: `MathExtended` (`MachineEpsilon` constant +
`CalculateMinimum/MaximumVector2`), `FloatHelper` (`Swap`, header-only), `Angle` (full
radian/degree/gradian/revolution angle type).

**Attribution finding**: `Angle.cs`'s own file header credits the **SlimMath** project
(Copyright (c) 2007-2010 SlimDX Group, MIT License) as the origin of this code — not just
Craftwork Games. Added a new "Code directly derived from other MIT-licensed projects"
section to `NOTICE.md` with SlimMath's full license text, distinct from the existing
inspiration-only "courtesy attribution" list (Mercury Particle Engine, 2D XNA Primitives,
LibGDX) — those aren't code actually carried into this repo, SlimMath's is. **If a future
ported file's upstream header credits another project by name the same way, add it to
that same NOTICE.md section, not the courtesy list.**

**Fidelity subtlety worth remembering**: `Angle::Equals`/`CompareTo` in upstream are NOT
`readonly` — they call `WrapPositive()` on `this` (mutating in place) and on a by-value
copy of the `other` parameter, so calling `Equals`/`CompareTo`/`==`/`!=` on an `Angle` has
the side effect of wrapping it into `[0, tau)`. Ported this exactly: `Equals`/`CompareTo`
are non-const, take `other` by value (not `const&`), and `operator==`/`!=` take both
operands by value for the same reason. `GetHashCode()` deliberately does *not* wrap first
— an apparent equality/hashing contract inconsistency in upstream itself — preserved as-is
rather than "fixed", since a faithful port isn't the place to correct upstream's own bugs
silently. This is exactly the kind of easy-to-miss-by-skimming detail worth specifically
grep'ing for in future tasks: check whether C# methods are `readonly` before assuming a
C++ `const` method is a safe translation.

**Small follow-up applied to a previous task's file**: `HslColor.cpp` referenced
`std::numeric_limits<float>::epsilon()` as a stand-in for `MathExtended.MachineEpsilon`
(not yet ported when that task ran). Now that `MathExtended` is real, swapped it in
directly — same value, but now the actual named constant matching upstream, not a
workaround. Worth checking for this pattern going forward: when a task lands, grep for
prior "not yet ported, using X as a stand-in" comments elsewhere in the tree and follow up.

**Test coverage**: both upstream test files ported 1:1 (`MathExtendedTests.cs`,
`AngleTest.cs`) — no `Collision2D` dependency to work around this time either.

**Verification**: both build modes clean, `ctest` → **100% passed, 189/189** (was 180
before this task).

**State / next step:** Phase 1 is 7 of ~20 tasks in. Next per `plan.md` §5 Phase 1:
`RectangleF`, `Rectangle.Extensions`, `RectangleF.Extensions`, `BoundingRectangle`,
`OrientedRectangle` — a bigger task (this is the family that `ISizable`, `IRectangular.
IRectangularF`, `Camera<T>`, and several bounding-volume factory methods have all been
forward-declaring/deferring against). Landing this should let several earlier deferred
pieces get finished as natural follow-ups — check `plan.md`'s decisions log and each
affected file's header comment for what's waiting on `RectangleF` specifically before
starting, and fold in whichever of those follow-ups make sense in the same pass rather
than opening a new task for each. **Continue without pausing for a status update per the
standing correction from session (2), unless a genuine blocker requiring the user's
judgment comes up.**

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
