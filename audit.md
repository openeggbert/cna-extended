# cna-extended Port and World3DEXT Audit

**Audit date:** 2026-07-14  
**Audited revision:** cna-extended bf71945 (develop)  
**Upstream reference inspected:** MonoGame.Extended 78c00977

## Executive assessment

cna-extended is a strong, substantive C++ port of the runtime-relevant part of MonoGame.Extended, not an API-shaped skeleton. It has coherent C++/CNA/sharp-runtime integration, broad runtime module coverage, both linked and headers-only builds, and a large translated test suite. Excluding the .NET/MGCB content pipeline and XNB readers is a sound deliberate boundary: direct-format loaders replace their runtime use case in this C++ project.

World3DEXT is not a MonoGame.Extended port: upstream is fundamentally 2D-only. It is a clearly marked extension over the existing ECS, transforms, screens, graphics and CNA 3D API. Its EXT suffix correctly distinguishes it from ported API. It already contains a camera, hierarchy bridge, rigid/skinned rendering, billboards, text, debug drawing, particles, voxel tilemaps, collisions and a screen/demo integration.

| Area | Assessment | Rationale |
|---|---:|---|
| Runtime port fidelity | **8/10** | Broad, source-shaped implementation; exclusions are documented and rational. |
| CNA / sharp-runtime integration | **8/10** | Clean target reuse/build modes and documented sibling-runtime workarounds. |
| Test breadth | **8/10** | 2,159 registered CTest cases, including 80 World3DEXT cases. |
| Local rendering confidence | **5/10** | Builds succeed, but this environment cannot initialise SDL video, so rendering assertions could not run. |
| World3DEXT design maturity | **6/10** | Strong reuse and useful breadth, but correctness, validation and scalability gaps remain. |

**Bottom line:** the 2D port is in good shape and appears close to its stated runtime scope. The 3D extension is a capable first version, but should be treated as experimental/early-production until the high-priority findings are fixed.

## Method and evidence

The audit reviewed the repository documentation and plans, CMake integration, the upstream checkout, all World3DEXT headers/implementations/tests, and targeted 3D algorithms and rendering paths. This is a source, build, test-registration and targeted behavioural audit; it is not a byte-for-byte proof for every upstream method.

| Check | Result | Meaning |
|---|---|---|
| Linked build: cmake --build build -j2 | Passed | Built sharp-runtime, CNA/EasyGL, CNA_EXTENDED, all examples and CnaExtendedTests. |
| Headers-only build: cmake --build build-headers -j2 | Passed | Public headers, tests and 2D/3D examples compile without linking CNA. |
| ctest --test-dir build -N | **2,159** tests | Current authoritative registration count. |
| World3DEXT registration | **80** tests | Covers camera, transforms, collision, grid broadphase, animation, particles, voxel data and rendering paths. |
| ctest --test-dir build -I 1887,1966 -j2 | 58 passed, 22 failed | Each failure occurred before graphics work: SDL_InitSubSystem(SDL_INIT_VIDEO) failed: No available video device. |

The 22 graphics failures do not prove a renderer algorithm failure. They do mean that render-target pixel assertions, depth tests, culling tests and the demo could not be independently reproduced in this environment. The installed xvfb-run also did not supply a usable display. This is therefore a serious validation/CI confidence issue.

## Strengths

- The expected runtime families are present: math/shapes/collections, 2D collisions, input, timers, tweening, viewport adapters, vector drawing, screens, graphics/sprites, bitmap fonts, direct-format tilemaps, particles, serialization and Artemis-style ECS.
- The library uses CNA XNA-shaped types directly. It does not invent a parallel math or graphics layer.
- CMake correctly supports a parent-provided CNA target, standalone sibling build, and headers-only compile checking. Both modes passed in this audit.
- MISSING.md records CNA semantic deviations instead of hiding them. That is especially valuable for render-target readback tests.
- World3DEXT reuses existing Transform3, ECS World, Screen, Model, BasicEffect, SkinnedEffect, buffers and BoundingFrustum rather than duplicating those systems.
- The 3D layer has real implementation depth: 80 dedicated registered tests and a complete example exist, rather than only public declarations.

## Findings

Severity reflects runtime impact and likelihood, including documented limitations when they contradict the public contract.

### A-01 — High: box/sphere minimum translation is invalid when a sphere centre is inside a box

**Evidence:** src/CNA/Extended/World3DEXT/CollisionShape3DEXT.cpp, TryGetBoxSphereCollision, especially lines 134-161.

The algorithm clamps the sphere centre to the box. For a centre inside the box, the closest point is the centre, so distance is zero. The fallback selects a direction but returns penetration depth equal only to sphere radius. In general this does not move the box out of the sphere or the sphere out of the box. The required distance also includes the distance from the centre to the selected nearest box face.

The source calls this a deliberate simplification, but the API promises an MTV that moves the first shape out of the second. Embedded objects occur in spawns, teleports and fast movement, so this is not a rare theoretical state.

**Recommendation:** select the nearest of six box faces for the inside case, return its outward normal, and return the exact separating depth. Add tests for the centre, all six near-face cases, non-uniform boxes and inverted sphere/box calls.

### A-02 — High: transform parent cycles are accepted

**Evidence:** src/CNA/Extended/World3DEXT/TransformHierarchySystemEXT.cpp, lines 31-55.

The system resolves any non-negative ParentEntityIdEXT to a pointer and calls setParentProperty. It neither rejects self-parenting nor detects indirect cycles. Transform3 is a parent-recursive hierarchy, so A -> B -> A can cause unbounded matrix/dirty propagation recursion, potentially a stack overflow.

Tests cover a valid chain, parent movement and detachment, but not self-parenting, two-node/longer cycles, a missing parent, or a parent without a transform.

**Recommendation:** validate the graph before wiring it. Reject self-parenting and walk ancestors with a visited set. Produce an error that identifies the entities. Add negative tests and define whether an invalid edit preserves the old relationship or detaches it.

### A-03 — High: graphical validation is not reproducible in this environment

**Evidence:** 22 of the 80 World3DEXT tests fail while constructing GraphicsDevice, with SDL_InitSubSystem(SDL_INIT_VIDEO) failed: No available video device. The audited build uses CNA EASY_GL.

The source compiles, but none of the important rendering assertions could be independently executed here. README.md, NEXT.md and world3d_demo describe headless rendering as normal; that claim is not universal with the current configuration.

**Recommendation:** make a known graphics configuration required in CI. Use a supported CNA headless/software backend if it can provide the required readback, or provision a proven software GL/X server setup. Add a preflight log/test for selected backend, SDL video driver and GL renderer. Label graphics tests in CTest, but require that labelled job before release. Document actual platform requirements.

### A-04 — Medium: billboard frustum bounds are too small

**Evidence:** src/CNA/Extended/World3DEXT/BillboardRenderSystemEXT.cpp, lines 63-67.

The radius is max(width, height) / 2. A billboard quad requires sqrt(width² + height²) / 2 for its enclosing sphere. The present sphere can falsely cull visible billboard corners near a frustum plane.

**Recommendation:** use half of SizeEXT.Length(), or another conservative bound. Add a boundary test with a large billboard whose centre is just outside a frustum plane while a corner remains visible.

### A-05 — Medium: OctreeEXT is a spatial hash with unbounded cubic work

**Evidence:** its own public header documents a fixed-cell spatial hash; src/CNA/Extended/World3DEXT/OctreeEXT.cpp, lines 65-125, iterates every cell covered by a query or actor AABB.

The honest documentation is good, but the name suggests a hierarchical octree. Large actors/queries can allocate or scan every covered cell in three dimensions. Candidate deduplication also repeatedly performs linear std::find.

**Recommendation:** rename it SpatialHash3DEXT and make that expectation explicit, or implement a real loose octree/BVH. At minimum add an oversized-object policy, a maximum-cell guard, and hash/query-stamp candidate deduplication. Benchmark large-world queries before presenting it as a general broadphase.

### A-06 — Medium: particle public inputs are insufficiently validated

**Evidence:** ParticleEmitter3DEXT normalizes the mutable ConeDirectionEXT without checking for zero. ParticleRenderSystem3DEXT directly converts opacity * 255.0f to uint8_t without clamping.

Zero direction and malformed ranges (negative/reversed/non-finite lifetimes, scales, rates and opacity) have undefined or surprising semantics. Out-of-range floating-point to byte conversion is not a safe colour policy.

**Recommendation:** validate at EmitEXT/UpdateEXT boundaries: reject or default a zero direction, clamp opacity to [0,1], require finite non-negative values, and define min/max inversion behaviour. Add property/fuzz tests for zero and extreme values.

### A-07 — Medium: rendering tests omit multi-object depth, transparency and frustum-edge regressions

Most 3D pixel tests draw one isolated object. The project documents that an initial RenderTarget2D choice lacked a depth buffer and that a multi-object demo exposed it. The test suite should now protect that specific failure class.

Opaque cubes, billboards, text and particles are submitted in ECS/container order; there is no opaque state sorting, transparent back-to-front ordering, or explicit transparency/depth-write policy. This is reasonable for a first version, but not a robust scene renderer.

**Recommendation:** add a depth-enabled scene with overlapping cubes drawn in two orders, frustum-edge billboards and semi-transparent particles/text. Define opaque and transparent passes before performance work.

### A-08 — Medium: model bounds are mandatory but easy to omit

ModelComponentEXT and SkinnedModelComponentEXT expose a default BoundingSphere and require the caller to populate BoundsEXT. Missing/wrong bounds silently cause incorrect culling, and there is no helper to derive local bounds from CNA model geometry.

**Recommendation:** add an asset-load helper that calculates bounds, or explicitly represent unset bounds and bypass culling until supplied. In debug builds, warn/assert instead of silently culling.

### A-09 — Medium: status documentation is stale and contradictory

At the audited revision:

- README.md states 2042/2042 tests.
- NEXT.md contains historical figures such as 2079/2079 and 2157/2159.
- CTest currently registers 2,159 tests.
- 3d.md still says the 3D plan needs approval/no code, while code and plan3d.md mark all phases complete.

This is not a runtime defect, but it weakens confidence in completion statements and confuses contributors.

**Recommendation:** establish one short current STATUS.md (or generated badge) as the authoritative status. Update README.md, NEXT.md and 3d.md together. Keep older numbers only in dated history.

### A-10 — Low: voxel tilemap rendering is intentionally simple and will not scale

TilemapRenderer3DEXT iterates an unordered map, tests every occupied tile and emits one complete cube draw per visible tile. It does not remove internal faces and submission order is non-deterministic.

**Recommendation:** retain this path for correctness/debugging, then add chunk meshing, hidden-face removal/greedy meshing, chunk-frustum culling, material batches and stable transparent ordering. Choose instancing versus combined meshes from measurements.

## Dependency observations

MISSING.md correctly documents two CNA differences that matter to this port:

1. Texture2D::GetData does not read GPU-rendered RenderTarget2D data, so render tests must read through GraphicsDevice while the target is still bound.
2. The no-rectangle GraphicsDevice::GetBackBufferData overload uses viewport size rather than the bound target's size, so robust callers must pass an explicit rectangle.

The local workarounds are good. These remain compatibility liabilities and should be raised upstream in CNA; a future CNA fix should be tested against both old and corrected semantics.

## Recommended order

1. Fix A-01 and A-02, including regression tests.
2. Resolve A-03: make a reproducible graphics CI path, then run the complete suite and world3d_demo there.
3. Fix conservative visibility (A-04), model-bound behaviour (A-08), and add depth/transparency/frustum-edge tests (A-07).
4. Harden public particle inputs (A-06).
5. Benchmark and rename/replace the grid broadphase; then optimise voxel rendering (A-05, A-10).
6. Synchronise status docs and supported headless configuration (A-09).

## Conclusion

The port is well implemented within its intentional runtime scope. It has enough real code, dependency integration and test coverage to support calling it a serious C++ reimplementation of MonoGame.Extended.

World3DEXT is valuable and architecturally promising, but it is new functionality rather than upstream parity. Its collision-contract defect, cycle safety gap and currently unrepeatable graphical verification are the main barriers to a stronger production-readiness claim. Addressing the first three recommendations would materially raise confidence.

