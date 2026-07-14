# MISSING.md — CNA vs XNA API-shape differences found while porting

This file tracks behavioral/API-shape differences between `cna` (this project's XNA 4.0/FNA
C++ dependency) and real XNA/FNA/MonoGame, discovered while porting MonoGame.Extended to
`cna-extended`. These are not bugs in `cna-extended` — they are places where `cna`'s
implementation of an XNA API diverges from upstream's real behavior in a way that affected a
literal 1:1 port and required a documented workaround here. Per `CLAUDE.md`'s "Do NOT modify
sibling repositories" rule, none of these are fixed in `cna` itself — only worked around
locally in `cna-extended`'s own code, and recorded here for whoever may want to align `cna`
more closely with upstream in the future.

## `Texture2D::GetData()` does not read back GPU-rendered content

**Where discovered**: `tests/CNA/Extended/Tilemaps/Rendering/TilemapIntegrationTests.cpp`
(ported from upstream's `TilemapIntegrationTests.cs`), while building this project's first
headless render-target pixel-readback test.

**Real XNA/FNA/MonoGame behavior**: `Texture2D.GetData()` — including when called on a
`RenderTarget2D`, which derives from `Texture2D` — returns the texture's actual current
GPU-side pixel contents, including content rendered into it via
`GraphicsDevice.SetRenderTarget()` plus draw calls, not just content uploaded via
`SetData()`. This is the standard, documented way to read back rendered output in
XNA/MonoGame (screenshot capture, procedural texture baking, pixel-level rendering tests,
etc.).

**`cna`'s current behavior**: `Texture2D::GetData(...)`
(`cna/include/Microsoft/Xna/Framework/Graphics/Texture2D.hpp`, implemented
`cna/src/Microsoft/Xna/Framework/Graphics/Texture2D.cpp:293-301`) only returns a CPU-side
pixel mirror populated exclusively by `SetData()` uploads. Rendering into a
`Texture2D`/`RenderTarget2D` via the GPU (`GraphicsDevice::SetRenderTarget()` + draw calls)
never touches that CPU-side mirror, so a literal `renderTarget->GetData(...)` call after
drawing returns stale/empty data instead of the actual rendered pixels.

**Confirmed workaround, already established in `cna` itself** (not invented for this port):
`GraphicsDevice::GetBackBufferData(Color* data, ...)` performs a real backend readback (e.g.
`glReadPixels` on the EasyGL backend) of whichever framebuffer is *currently bound* — see
`cna/examples/easygl_rt_roundtrip_test.cpp`'s own comment, "Read RT1 pixel while FBO is still
bound." The correct call sequence in `cna` for reading back rendered content is therefore:

```cpp
graphicsDevice.SetRenderTarget(&renderTarget);
// ... draw calls ...
graphicsDevice.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size())); // BEFORE unbinding
graphicsDevice.SetRenderTarget(nullptr);
```

**Impact on this port**: `TilemapIntegrationTests.cpp`'s `RenderToPixels()` helper uses this
sequence instead of a literal translation of upstream's C# (`renderTarget.GetData(pixels)`),
documented in that file's own header comment. Once fixed, all 28 ported pixel-verification
tests pass with the exact same assertions/tolerances as upstream — this is a pure API-shape
gap, not a rendering-correctness bug in `cna`, `cna-extended`, `TilemapRenderer`,
`TilemapSpriteBatchRenderer`, or `BasicEffect`.

**Not fixed in `cna`** (sibling-repo rule) — worked around locally in `cna-extended`'s own
test code only. Worth reporting to whoever maintains `cna`: a `Texture2D::GetData()` that
silently doesn't reflect real GPU-rendered content is a surprising divergence from real
XNA/MonoGame/FNA semantics that could affect any future consumer, not just this port's tests.
