# NEXT.md — session handoff log

This file is the short-term continuity document for `cna-extended`. Read it first in any
new session before touching code or `plan.md`. Append a new dated entry at the top after
every session with material progress; do not silently overwrite prior entries.

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
