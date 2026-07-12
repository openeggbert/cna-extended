# NEXT.md — session handoff log

This file is the short-term continuity document for `cna-extended`. Read it first in any
new session before touching code or `plan.md`. Append a new dated entry at the top after
every session with material progress; do not silently overwrite prior entries.

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
