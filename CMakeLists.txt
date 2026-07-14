# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20)

project(cna_extended
    VERSION 0.1.0
    DESCRIPTION "C++ port of MonoGame.Extended for CNA"
    LANGUAGES CXX)

# cna-extended follows CNA/sharp-runtime and uses C++23. Keep this in sync.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(CNA_EXTENDED_BUILD_EXAMPLES "Build cna-extended examples" ON)
option(CNA_EXTENDED_BUILD_TESTS "Build cna-extended tests" ON)

# Most of cna-extended (Graphics, Tilemaps, Particles, Screens, ECS drawing
# systems, ...) needs real CNA functionality (SpriteBatch, GraphicsDevice,
# Texture2D, ...), not just its headers. Building CNA ourselves is opt-in and
# OFF by default because it pulls in heavy deps (SHARP_RUNTIME, SDL3, a
# graphics backend, ...). A parent project that already defines the `CNA`
# target is detected and linked automatically regardless of this option.
option(CNA_EXTENDED_LINK_CNA "When building standalone, build CNA from CNA_EXTENDED_CNA_DIR and link it" OFF)

# CNA requires exactly one graphics backend. Ignored when the parent project
# provides the CNA target (it already chose a backend).
set(CNA_EXTENDED_CNA_BACKEND "EASY_GL"
    CACHE STRING "CNA backend to enable when cna-extended builds CNA itself: SDL_RENDERER | EASY_GL | BGFX | VULKAN")
set_property(CACHE CNA_EXTENDED_CNA_BACKEND PROPERTY STRINGS SDL_RENDERER EASY_GL BGFX VULKAN)

# ---------------------------------------------------------------------------
# Locate CNA (companion runtime, expected at ../cna)
# ---------------------------------------------------------------------------
set(CNA_EXTENDED_CNA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cna"
    CACHE PATH "Path to the CNA repository (provides headers under <dir>/include).")

if(EXISTS "${CNA_EXTENDED_CNA_DIR}/include")
    set(CNA_EXTENDED_CNA_INCLUDE_DIR "${CNA_EXTENDED_CNA_DIR}/include")
    message(STATUS "cna-extended: using CNA headers from ${CNA_EXTENDED_CNA_INCLUDE_DIR}")
else()
    message(WARNING
        "cna-extended: CNA headers not found at '${CNA_EXTENDED_CNA_DIR}/include'. "
        "Set -DCNA_EXTENDED_CNA_DIR=/path/to/cna. cna-extended will not compile "
        "until CNA's <Microsoft/Xna/Framework/*.hpp> headers are visible.")
endif()

# ---------------------------------------------------------------------------
# Locate sharp-runtime (companion BCL runtime, expected at ../sharp-runtime)
# ---------------------------------------------------------------------------
set(CNA_EXTENDED_SHARP_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sharp-runtime"
    CACHE PATH "Path to the sharp-runtime repository (provides headers under <dir>/include).")

if(EXISTS "${CNA_EXTENDED_SHARP_RUNTIME_DIR}/include")
    set(CNA_EXTENDED_SHARP_RUNTIME_INCLUDE_DIR "${CNA_EXTENDED_SHARP_RUNTIME_DIR}/include")
    message(STATUS "cna-extended: using sharp-runtime headers from ${CNA_EXTENDED_SHARP_RUNTIME_INCLUDE_DIR}")
else()
    message(WARNING
        "cna-extended: sharp-runtime headers not found at '${CNA_EXTENDED_SHARP_RUNTIME_DIR}/include'. "
        "Set -DCNA_EXTENDED_SHARP_RUNTIME_DIR=/path/to/sharp-runtime.")
endif()

# ---------------------------------------------------------------------------
# cna-extended library
# ---------------------------------------------------------------------------
file(GLOB_RECURSE CNA_EXTENDED_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

add_library(CNA_EXTENDED STATIC
    ${CNA_EXTENDED_SOURCES}
)

target_include_directories(CNA_EXTENDED
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(MSVC)
    target_compile_options(CNA_EXTENDED PRIVATE /W4 /WX)
else()
    target_compile_options(CNA_EXTENDED PRIVATE -Wall -Wextra -Werror)
endif()

# ---------------------------------------------------------------------------
# Resolve CNA linkage
# ---------------------------------------------------------------------------
# cna-extended ends up linked against CNA in one of three ways (same pattern
# easy-3d uses):
#   (1) A parent project (e.g. a game using both CNA and cna-extended) already
#       defined the `CNA` target before add_subdirectory(cna-extended). We
#       detect and link it.
#   (2) Standalone build with -DCNA_EXTENDED_LINK_CNA=ON: we add_subdirectory
#       (../cna) ourselves (selecting a backend, with CNA's own demos/tests
#       disabled).
#   (3) Otherwise: headers only. cna-extended still compiles for
#       compile-checking; the consuming application links CNA itself. Most
#       real functionality (anything touching GraphicsDevice/SpriteBatch/
#       Texture2D) will not *link* in this mode, only compile.
set(_cna_extended_have_cna_target OFF)

if(TARGET CNA)
    # (1) Provided by the parent project — the integration path for games.
    set(_cna_extended_have_cna_target ON)
    message(STATUS "cna-extended: linking the 'CNA' target provided by the parent project.")
elseif(CNA_EXTENDED_LINK_CNA)
    # (2) Build CNA ourselves from CNA_EXTENDED_CNA_DIR.
    if(NOT EXISTS "${CNA_EXTENDED_CNA_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "cna-extended: CNA_EXTENDED_LINK_CNA=ON but no CNA CMake project at "
            "'${CNA_EXTENDED_CNA_DIR}'. Set -DCNA_EXTENDED_CNA_DIR=/path/to/cna.")
    endif()
    # Keep CNA's own examples/tests out of our build.
    set(CNA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CNA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    # Select exactly one CNA graphics backend (CNA fails to configure otherwise).
    if(CNA_EXTENDED_CNA_BACKEND STREQUAL "SDL_RENDERER")
        set(CNA_BACKEND_SDL_RENDERER ON CACHE BOOL "" FORCE)
    elseif(CNA_EXTENDED_CNA_BACKEND STREQUAL "EASY_GL")
        set(CNA_BACKEND_EASY_GL ON CACHE BOOL "" FORCE)
    elseif(CNA_EXTENDED_CNA_BACKEND STREQUAL "BGFX")
        set(CNA_BACKEND_BGFX ON CACHE BOOL "" FORCE)
    elseif(CNA_EXTENDED_CNA_BACKEND STREQUAL "VULKAN")
        set(CNA_BACKEND_VULKAN ON CACHE BOOL "" FORCE)
    else()
        message(FATAL_ERROR
            "cna-extended: unknown CNA_EXTENDED_CNA_BACKEND '${CNA_EXTENDED_CNA_BACKEND}' "
            "(use SDL_RENDERER, EASY_GL, BGFX or VULKAN).")
    endif()
    message(STATUS "cna-extended: building CNA from '${CNA_EXTENDED_CNA_DIR}' (backend: ${CNA_EXTENDED_CNA_BACKEND}).")
    add_subdirectory("${CNA_EXTENDED_CNA_DIR}" "${CMAKE_BINARY_DIR}/_cna")
    set(_cna_extended_have_cna_target ON)
endif()

if(_cna_extended_have_cna_target)
    # CNA propagates its PUBLIC include dirs (the Microsoft/Xna headers) and,
    # transitively, SHARP_RUNTIME's.
    target_link_libraries(CNA_EXTENDED PUBLIC CNA)
    target_compile_definitions(CNA_EXTENDED PUBLIC CNA_EXTENDED_HAS_CNA_LINK)
else()
    # (3) Headers-only: compile against CNA + sharp-runtime headers; the
    # consumer links CNA (and therefore SHARP_RUNTIME) later.
    if(CNA_EXTENDED_CNA_INCLUDE_DIR)
        target_include_directories(CNA_EXTENDED PUBLIC "${CNA_EXTENDED_CNA_INCLUDE_DIR}")
    endif()
    if(CNA_EXTENDED_SHARP_RUNTIME_INCLUDE_DIR)
        target_include_directories(CNA_EXTENDED PUBLIC "${CNA_EXTENDED_SHARP_RUNTIME_INCLUDE_DIR}")
        # sharp-runtime's own CMakeLists.txt exposes its vendored single-header/two-file
        # libraries (nlohmann/json.hpp, tinyxml2/tinyxml2.h, ...) via
        # `target_include_directories(SHARP_RUNTIME SYSTEM PUBLIC .../vendor)` -- only
        # reachable when SHARP_RUNTIME is actually built as a target (linked mode). Headers-
        # only mode bypasses that target entirely, so it has to add the same vendor dir
        # itself or any sharp-runtime header pulling in a vendored dependency (e.g.
        # System/Xml/XmlDocument.hpp -> tinyxml2/tinyxml2.h) fails to compile here even
        # though it compiles fine in linked mode.
        if(EXISTS "${CNA_EXTENDED_SHARP_RUNTIME_DIR}/vendor")
            target_include_directories(CNA_EXTENDED SYSTEM PUBLIC "${CNA_EXTENDED_SHARP_RUNTIME_DIR}/vendor")
        endif()
    endif()
endif()

# Whether targets that actually run CNA-backed functionality (examples/tests
# exercising Graphics/Tilemaps/Particles/etc.) can link. Visible to the
# examples/ and tests/ subdirectories added below.
set(CNA_EXTENDED_CNA_LINKED ${_cna_extended_have_cna_target})

add_library(CNA::Extended ALIAS CNA_EXTENDED)

# ---------------------------------------------------------------------------
# Examples & tests
# ---------------------------------------------------------------------------
if(CNA_EXTENDED_BUILD_EXAMPLES)
    add_subdirectory(examples/minimal)
    add_subdirectory(examples/tiled_demo)
endif()

if(CNA_EXTENDED_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
