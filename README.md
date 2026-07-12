# cna-extended

`cna-extended` is a C++23 port of [MonoGame.Extended](https://github.com/craftworkgames/MonoGame.Extended)
for [`cna`](../cna) (a C++23 port of XNA 4.0 / FNA), built on top of
[`sharp-runtime`](../sharp-runtime) (a C++23 reimplementation of the relevant .NET BCL
surface). It is a sibling library in the same family as [`easy-3d`](../easy-3d).

MonoGame.Extended is "a set of utilities... that makes it easier to make games" on top of
MonoGame — collision detection, tilemaps, particles, sprite sheets, an entity-component
system, tweening, input helpers, and more. `cna-extended` ports the runtime-usable parts
of that library to C++, faithfully — the goal is a 1:1 translation as far as the
differences between C# and C++ allow, not a reimagining.

## Status

Early bootstrap. See [`plan.md`](plan.md) for the full phase-by-phase task list and
scope decisions, and [`NEXT.md`](NEXT.md) for the current session-to-session status.

## Scope

Ported: math/shapes/collections foundation, 2D collision detection, input listeners,
timers, tweening, viewport adapters, debug vector drawing, screens, sprite/graphics
extensions, bitmap fonts, sprite animation, particles, an Artemis-style ECS, and Tiled/
LDtk/Ogmo tilemap support.

Not ported (see `plan.md` §2 for the full rationale): the design-time
`MonoGame.Extended.Content.Pipeline` (MGCB tooling has no C++ runtime counterpart), and
the runtime `.xnb`-based `ContentTypeReader` classes that only exist to consume that
pipeline's output. Direct-format loading (Tiled TMX/JSON, LDtk JSON, Ogmo JSON,
TexturePacker JSON, BMFont `.fnt`) covers the same ground without needing MGCB.

## Building

Depends on `../cna` and `../sharp-runtime` as sibling checkouts, following the same
CMake dependency pattern as `easy-3d` (see `plan.md` §4): if a `CNA` target already
exists in the parent build it's reused, otherwise it can be built standalone via an
opt-in flag, or a headers-only fallback is used for compile-checking. Exact CMake usage
will be documented here once Phase 0 scaffolding lands.

## License

MIT — see [`LICENSE`](LICENSE). See [`NOTICE.md`](NOTICE.md) for MonoGame.Extended's
own MIT attribution.
