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

Porting is complete: all 9 module phases described in `plan.md` (Phases 1-9) are fully
ported and tested. On top of the port, `World3DEXT` (see `3d.md`/`plan3d.md`) adds a
non-upstream 3D scene extension — camera, transform hierarchy, model/skinned-model
rendering, 3D collisions, billboards/text/debug-draw, particles, voxel tilemaps, and an
end-to-end `world3d_demo` example — with all 9 of its own phases complete and an
independent audit's follow-up fixes (`plan3d.md`'s Phase 10) in progress. Current test
count: 2205/2205 passing (see `NEXT.md` for the most up-to-date figure, since this number
moves as work continues).

## Modules

- **Math, Shapes & Collections** (Phase 1) — `Vector`/`Angle` helpers, `RectangleF`,
  `CircleF`, `BoundingRectangle`, `Segment2`, 2D bounding volumes (`BoundingBox2D`,
  `BoundingCircle2D`, `BoundingCapsule2D`, `BoundingPolygon2D`, `OrientedBoundingBox2D`),
  ear-clipping triangulation, transform hierarchies (`Transform2`/`Transform3`), and
  extra collections (`Bag<T>`, `Deque<T>`) not already covered by `sharp-runtime`.
- **2D Collisions** (Phase 2) — shape-vs-shape intersection/containment tests
  (`Collision2D`), a broadphase-backed `CollisionWorld2D` with pluggable actors, a
  quadtree and spatial-hash broadphase, and layer-based collision filtering.
- **Input, Timers, Tweening, ViewportAdapters & VectorDraw** (Phase 3) — keyboard/mouse/
  gamepad/touch input listeners, frame-independent timers, a pointer-to-member-based
  `Tweener`/easing-function system, viewport scaling/boxing adapters, and debug
  primitive/vector drawing (`PrimitiveBatch`/`PrimitiveDrawing`).
- **Screens** (Phase 4) — a `Screen`/`ScreenManager` stack with `FadeTransition`/
  `ExpandTransition`.
- **Graphics, BitmapFonts & Animations** (Phase 5) — sprite/animation types (`Sprite`,
  `AnimatedSprite`, `SpriteSheet`), texture atlases (`Texture2DAtlas`,
  `Texture2DRegion`), nine-patch drawing, `SpriteBatch`/`GraphicsDevice` extension
  helpers, direct-format `TexturePacker` JSON atlas loading, AngelCode BMFont bitmap
  fonts (`BitmapFont`), and the sprite `AnimationController` system.
  Custom shader effects (`DefaultEffect`/`MatrixChainEffect`) are implemented on `cna`'s
  hand-written `ShaderEffect`/`BasicEffect`, not a literal MonoGame `Effect` bytecode
  port (`cna` has no `.mgfxo` bytecode loader).
- **Tilemaps** (Phase 7) — a format-agnostic tilemap data model (`Tilemap`,
  `TilemapLayers`, `TilemapObjects`, `TilemapTileset`) fed by direct-format parsers for
  Tiled (TMX/JSON), LDtk (JSON), and Ogmo (JSON) — no MGCB/`.xnb` step required — plus
  two independent rendering paths: `SpriteBatch`-based (`TilemapSpriteBatchRenderer`/
  `TilemapWorldSpriteBatchRenderer`) and direct `VertexBuffer`/`BasicEffect`-based
  (`TilemapRenderer`/`TilemapWorldRenderer`).
- **Particles** (Phase 8) — `ParticleEffect`/`ParticleEmitter` with a modifier/profile
  pipeline and XML effect serialization, matching MonoGame.Extended's Mercury Particle
  Engine-derived particle system.
- **ECS** (Phase 9) — an Artemis-style entity-component system: `World`/`WorldBuilder`,
  `Entity`/`EntityManager`, `Aspect`-based system filtering, and component mappers.

Not ported (see `plan.md` §2 for the full rationale): the design-time
`MonoGame.Extended.Content.Pipeline` (MGCB tooling has no C++ runtime counterpart), and
the runtime `.xnb`-based `ContentTypeReader` classes that only exist to consume that
pipeline's output. Direct-format loading (Tiled TMX/JSON, LDtk JSON, Ogmo JSON,
TexturePacker JSON, BMFont `.fnt`) covers the same ground without needing MGCB.

## Usage

`cna-extended` is a normal CMake static library consumed alongside `cna`; most modules
(anything touching `GraphicsDevice`/`SpriteBatch`) need a real, linked `cna` to run, not
just its headers. A small example that isn't graphics-dependent — tweening a game
object's position with an easing curve — looks like this:

```cpp
#include <CNA/Extended/Tweening/EasingFunctions.hpp>
#include <CNA/Extended/Tweening/Tweener.hpp>
#include <Microsoft/Xna/Framework/Vector2.hpp>

using CNA::Extended::Tweening::EasingFunctions;
using CNA::Extended::Tweening::Tweener;
using Microsoft::Xna::Framework::Vector2;

struct Player
{
    Vector2 Position{0.0f, 0.0f};
};

int main()
{
    Player player;
    Tweener tweener;

    // Animate player.Position to (100, 0) over half a second, eased out.
    tweener.TweenTo(&player, &Player::Position, Vector2(100.0f, 0.0f), 0.5f)
        ->Easing(EasingFunctions::QuadraticOut);

    // Call once per frame, e.g. from your game's Update():
    tweener.Update(1.0f / 60.0f);

    return 0;
}
```

See `examples/` for a fuller, graphics-backed sample (loading and rendering content via
`cna`'s `GraphicsDevice`/`SpriteBatch`).

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

Other relevant CMake options (see `CMakeLists.txt` for the full list): `CNA_EXTENDED_BUILD_EXAMPLES`
and `CNA_EXTENDED_BUILD_TESTS` (both `ON` by default) toggle `examples/`/`tests/`, and
`CNA_EXTENDED_CNA_BACKEND` (default `EASY_GL`; also `SDL_RENDERER`, `BGFX`, `VULKAN`)
selects which `cna` graphics backend gets built when `CNA_EXTENDED_LINK_CNA=ON`.

Both configurations are kept warning-free under `-Wall -Wextra -Werror`
(`/W4 /WX` on MSVC).

## Documentation

API reference docs are generated with Doxygen from the headers under `include/`:

```sh
doxygen Doxyfile
```

Output is written to `docs/generated/html` (git-ignored, not checked in).

## License

MIT — see [`LICENSE`](LICENSE). See `NOTICE.md` for MonoGame.Extended's own MIT
attribution.
