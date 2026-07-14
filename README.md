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

Nearly complete: 9 of the plan's 10 phases (see [`plan.md`](plan.md)) are fully done.
The one remaining item, `TilemapRenderer`/`TilemapWorldRenderer`, is blocked on a
genuine architectural decision (documented in `plan.md`'s Phase 7 entry and
[`NEXT.md`](NEXT.md) section 4) rather than unfinished work — everything else in the
tilemap-rendering pipeline (the `SpriteBatch`-based renderers, all three supported map
formats) is already ported and tested. See [`NEXT.md`](NEXT.md) for the current
session-to-session status.

## Scope

Ported: math/shapes/collections foundation, 2D collision detection, input listeners,
timers, tweening, viewport adapters, debug vector drawing, screens (including fade/expand
transitions), sprite/graphics extensions, custom shader effects, bitmap fonts, sprite
animation, an Artemis-style ECS, JSON/XML serialization helpers, particles (including
XML effect serialization), and tilemaps — a shared runtime data model plus direct-format
parsers for Tiled (TMX/JSON), LDtk (JSON), and Ogmo (JSON), with `SpriteBatch`-based
rendering for all three.

Not ported (see `plan.md` §2 for the full rationale): the design-time
`MonoGame.Extended.Content.Pipeline` (MGCB tooling has no C++ runtime counterpart), and
the runtime `.xnb`-based `ContentTypeReader` classes that only exist to consume that
pipeline's output. Direct-format loading (Tiled TMX/JSON, LDtk JSON, Ogmo JSON,
TexturePacker JSON, BMFont `.fnt`) covers the same ground without needing MGCB.

## Building

Depends on `../cna` and `../sharp-runtime` as sibling checkouts, following the same
CMake dependency pattern as `easy-3d` (see `plan.md` §4): if a `CNA` target already
exists in the parent build it's reused, otherwise it can be built standalone via an
opt-in flag (`CNA_EXTENDED_LINK_CNA=ON`, builds the real `cna`/`sharp-runtime` backend),
or a headers-only fallback is used for compile-checking (the default).

Linked config (needed for anything that actually touches `GraphicsDevice`/`SpriteBatch`,
including running the test suite's rendering-backed tests):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCNA_EXTENDED_LINK_CNA=ON
cmake --build build -j$(nproc)
ctest --test-dir build
```

Headers-only config (compile-checking only, no `cna`/`sharp-runtime` build, no test
execution against real `GraphicsDevice`-backed code):

```sh
cmake -S . -B build-headers -DCMAKE_BUILD_TYPE=Debug -DCNA_EXTENDED_LINK_CNA=OFF
cmake --build build-headers -j$(nproc)
```

Both configurations are kept warning-free under `-Wall -Wextra -Werror`
(`/W4 /WX` on MSVC).

## License

MIT — see [`LICENSE`](LICENSE). See [`NOTICE.md`](NOTICE.md) for MonoGame.Extended's
own MIT attribution.
