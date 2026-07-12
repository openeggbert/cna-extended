# CLAUDE.md — rules for automated contributions to cna-extended

This file is for future Claude Code (or any automated) sessions working in the
**cna-extended** repository. Read it, `NEXT.md`, and `plan.md` — in that order — before
making any change.

## What this project is

`cna-extended` is a C++23 port of [MonoGame.Extended](https://github.com/craftworkgames/MonoGame.Extended)
for [`cna`](../cna) (a C++23 port of XNA 4.0 / FNA), built on
[`sharp-runtime`](../sharp-runtime) (a C++23 .NET BCL reimplementation). It is a sibling
library in the same family as [`easy-3d`](../easy-3d), following the same conventions.

Full scope, module-by-module in/out decisions, and the phase-ordered task list live in
`plan.md`. Do not re-derive that scope from first principles — it was explicitly
negotiated with the user; read it.

Reference source for the C# original (read-only, never edit):
`/rv/data/library/github.com/craftworkgames/MonoGame.Extended`. Refresh it with
`/rv/data/library/github.com/github.sh` (no-arg mode updates every reference clone in
that tree, not just this one).

## Read first, every session

1. `NEXT.md` — what happened last session, what's blocked, what to do next. Update it
   throughout this session, not just at the end.
2. `plan.md` — the authoritative task list and scope. Check tasks off as they land; add
   newly discovered tasks; do not silently reorder phases without noting why in `NEXT.md`.
3. `README.md` (once it exists) — public-facing description.

## The one hard gate

**Do not start Phase 0 (or any later phase) of `plan.md` until the user has explicitly
approved the plan.** This was an explicit, repeated instruction from the user when this
project was bootstrapped — do not treat a long gap in the conversation, or an
"autonomous work" instruction to keep going, as implicit approval. If `plan.md` still
says `Status: DRAFT` at the top and there is no clear approval in the conversation, your
job is to refine the plan and ask questions, not write C++. Once approved, remove the
`DRAFT` marker from `plan.md` and record the approval in `NEXT.md` before starting Phase 0.

## Working rules

- **Follow `plan.md`'s phase order.** Phases are dependency-ordered (Math/Collections
  before Collisions/Graphics before Tilemaps/Particles/ECS) for a reason — don't jump
  ahead to a module whose dependencies aren't ported yet.
- **Port tests alongside implementation**, translated to GoogleTest, in the same phase —
  do not defer "add tests later."
- **Every ported file gets an SPDX header** (`// SPDX-License-Identifier: MIT`) plus a
  one-line "ported from MonoGame.Extended" note, matching `sharp-runtime`'s convention
  for its own upstream attributions. Do not drop this even for small files.
- **Reuse `sharp-runtime` types, don't re-roll them.** Before porting a MonoGame.Extended
  utility type or collection, check whether `sharp-runtime`'s `System::*` tree already
  has the real BCL equivalent (this already happened once: MonoGame.Extended's own
  `ObservableCollection` is explicitly skipped in favor of
  `System::Collections::ObjectModel::ObservableCollection` — see `plan.md` §2). The same
  check applies to `Serialization/*` vs `sharp-runtime`'s `System::Text::Json` (Phase 6).
- **Namespace is `CNA::Extended::`**, sub-namespaced per module
  (`CNA::Extended::Tilemaps`, `CNA::Extended::Particles`, etc.) — not a 1:1
  `MonoGame::Extended::` mirror. See `plan.md` §4 for the reasoning; don't relitigate it
  without the user.
- **File layout mirrors the namespace path**: `include/CNA/Extended/<Module>/<Type>.hpp`
  ↔ `src/CNA/Extended/<Module>/<Type>.cpp`, matching `cna`'s and `sharp-runtime`'s own
  convention.
- **C# properties → `getXProperty()`/`setXProperty()`**, matching `cna`/`sharp-runtime`.
- **Keep the CMake sibling-dependency pattern from `easy-3d`** (`if(TARGET CNA)` check,
  then an opt-in `add_subdirectory(../cna)` fallback, then a headers-only fallback) — see
  `plan.md` §4. Don't switch to `FetchContent`/submodules for `cna`/`sharp-runtime`
  without asking; that's a different dependency model than the rest of this ecosystem
  uses.
- **Zero-warning policy**: `-Wall -Wextra -Werror` (`/W4 /WX` on MSVC), matching
  `sharp-runtime`. Don't merge work that introduces new warnings.

## Explicitly excluded — do not port these even if you find them convenient

- `MonoGame.Extended.Content.Pipeline` (the whole design-time MGCB assembly).
- Any `Content/ContentReaders/*`, `Tilemaps/Content/*Reader`, `Content/BitmapFonts/`
  (the xnb-side helper — not the runtime `BitmapFonts/` module, which **is** ported), or
  other class whose sole purpose is reading a `.xnb` binary asset produced by the
  excluded Content Pipeline. See `plan.md` §2 for the full rationale and the (short) list
  of `Content/*` files that are direct-format (not xnb) and therefore **are** in scope
  (`TexturePacker/*`, `ExternalResourceResolver(s)`).

If you find a class during implementation that's ambiguous (looks xnb-adjacent but isn't
obviously so), default to excluding it and note the ambiguity in `NEXT.md` rather than
guessing either way.

## Do NOT modify sibling repositories

Do not edit, build into, or "fix" these unless the user explicitly asks:

- `../cna`
- `../sharp-runtime`
- `../easy-3d`
- any other sibling repo under `/rv/data/development/github.com/openeggbert/`

You may *read* `../cna` and `../sharp-runtime` to find existing types, namespaces, and
CMake target names to build against.

## Features that require explicit approval

Do not add any of the following without the user explicitly approving it first — none of
these are in `plan.md`'s current scope:

- Anything from `MonoGame.Extended.Content.Pipeline` or a home-grown replacement for it.
- New third-party dependencies beyond what `plan.md` already lists (GoogleTest). If a
  module seems to need one (e.g. a JSON/XML library beyond what `sharp-runtime` already
  vendors), stop and ask rather than adding it.
- Support for a MonoGame.Extended map/atlas format not listed in `plan.md` §2.
- Switching the tracking mechanism from `plan.md` checkboxes to something else (e.g. a
  `plan.sqlite3` DB like `sharp-runtime` uses) — the user explicitly chose markdown.

## After meaningful changes

- Check off completed tasks in `plan.md`; add newly discovered tasks under the right
  phase.
- Update `NEXT.md` with what changed, what's next, and any new open questions —
  granularly, not just at the end of a long session.
- Keep the build green: whatever the finalized Phase 0 build command turns out to be,
  record it in `NEXT.md` and run it before ending a session.
