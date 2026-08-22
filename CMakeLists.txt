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

# Most of cna-extended needs compiled CNA symbols. A parent project may provide
# CNA's modular targets first; otherwise a standalone build adds cnanext and
# selects only the dependency modules listed below. The headers-only mode is
# retained for API compile checks, but the linked modular build is the useful
# default.
option(CNA_EXTENDED_LINK_CNA
    "When building standalone, add cnanext and link the CNA modules cna-extended uses"
    ON)

# cnanext requires one renderer, platform and audio implementation even when a
# consumer links a subset of the framework modules. The dependency defaults are
# deliberately host-independent and do not pull a windowing, GPU, or sound
# backend into a cna-extended build. The CPU software renderer is used instead
# of the no-op headless renderer because rendering tests inspect backbuffer
# pixels. Applications that need a visible window can select e.g.
# OPENGLES3/SDL3/SDL3.
set(CNA_EXTENDED_CNA_RENDERER "SOFTWARE"
    CACHE STRING "cnanext renderer used by a standalone cna-extended build")
set_property(CACHE CNA_EXTENDED_CNA_RENDERER PROPERTY STRINGS
    SOFTWARE HEADLESS SDL_RENDERER OPENGLES2 OPENGLES3 OPENGL33 BGFX VULKAN)
set(CNA_EXTENDED_CNA_PLATFORM "HEADLESS"
    CACHE STRING "cnanext platform used by a standalone cna-extended build")
set_property(CACHE CNA_EXTENDED_CNA_PLATFORM PROPERTY STRINGS HEADLESS SDL3 SDL2 TERMINAL)
set(CNA_EXTENDED_CNA_AUDIO_PLATFORM "NULL"
    CACHE STRING "cnanext audio platform used by a standalone cna-extended build")
set_property(CACHE CNA_EXTENDED_CNA_AUDIO_PLATFORM PROPERTY STRINGS NULL SDL3 SDL2)

# Direct CNA module ownership, derived from every Microsoft/Xna and CNA header
# included by include/ and src/. Their PUBLIC dependency closure supplies the
# platform, audio, and media implementation modules required by Runtime and
# Content; the CNA umbrella would additionally pull Devices, CNAEXT, Storage,
# GamerServices, and Net, none of which cna-extended uses.
set(CNA_EXTENDED_CNA_MODULE_TARGETS
    CNA::Core
    CNA::Math
    CNA::GraphicsCore
    CNA::Input
    CNA::Content
    CNA::Runtime
)

# Direct Sharp Runtime ownership, derived from every System and SharpRuntime
# header included by include/ and src/. These are the only Sharp Runtime targets
# linked to CNA_EXTENDED itself.
set(CNA_EXTENDED_SHARP_RUNTIME_TARGETS
    SharpRuntime::Core.Base
    SharpRuntime::Collections.Core
    SharpRuntime::IO
    SharpRuntime::IO.Compression
    SharpRuntime::Text
    SharpRuntime::Text.Json
    SharpRuntime::Threading.Tasks
    SharpRuntime::Xml
)

# cnanext's selected module closure additionally declares the components below
# (notably Runtime for Audio and the narrow renderer contract). Setting the
# complete union before adding sharp-runtimenext prevents its standalone `All`
# default from enabling unrelated networking, HTTP, XML LINQ, timers, etc.
set(CNA_EXTENDED_SHARP_RUNTIME_COMPONENTS
    Core.Base
    IO
    Collections.Core
    Collections.ObjectModel
    Runtime
    Threading
    Text
    Globalization
    Storage
    Security.Cryptography
    IO.Compression
    Text.Json
    Threading.Tasks
    Xml
)

# ---------------------------------------------------------------------------
# Locate CNA (modular companion runtime, expected at ../cnanext)
# ---------------------------------------------------------------------------
set(CNA_EXTENDED_CNA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cnanext"
    CACHE PATH "Path to the modular cnanext repository")

if(EXISTS "${CNA_EXTENDED_CNA_DIR}/modules/CMakeLists.txt")
    message(STATUS "cna-extended: using modular CNA from ${CNA_EXTENDED_CNA_DIR}")
else()
    message(WARNING
        "cna-extended: modular CNA not found at '${CNA_EXTENDED_CNA_DIR}'. "
        "Set -DCNA_EXTENDED_CNA_DIR=/path/to/cnanext.")
endif()

# ---------------------------------------------------------------------------
# Locate sharp-runtime (modular companion BCL, expected at ../sharp-runtimenext)
# ---------------------------------------------------------------------------
set(CNA_EXTENDED_SHARP_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sharp-runtimenext"
    CACHE PATH "Path to the modular sharp-runtimenext repository")

if(EXISTS "${CNA_EXTENDED_SHARP_RUNTIME_DIR}/cmake/SharpRuntimeModules.cmake")
    message(STATUS "cna-extended: using modular Sharp Runtime from ${CNA_EXTENDED_SHARP_RUNTIME_DIR}")
else()
    message(WARNING
        "cna-extended: modular Sharp Runtime not found at '${CNA_EXTENDED_SHARP_RUNTIME_DIR}'. "
        "Set -DCNA_EXTENDED_SHARP_RUNTIME_DIR=/path/to/sharp-runtimenext.")
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
# Resolve modular CNA / Sharp Runtime linkage
# ---------------------------------------------------------------------------
set(_cna_extended_have_cna_target OFF)

# A source-tree parent may already have enabled only CNA's own Sharp Runtime
# closure. Extend that selection by exactly cna-extended's direct components
# before validating the modular targets.
if(TARGET CNA::Core AND COMMAND sharp_runtime_enable_components)
    sharp_runtime_enable_components(${CNA_EXTENDED_SHARP_RUNTIME_COMPONENTS})
endif()

if(TARGET CNA::Core)
    # Provided by a parent project. Do not link the parent's CNA umbrella: it
    # intentionally aggregates every framework extension module.
    set(_cna_extended_have_cna_target ON)
    message(STATUS "cna-extended: using modular CNA targets provided by the parent project")
elseif(CNA_EXTENDED_LINK_CNA)
    if(NOT EXISTS "${CNA_EXTENDED_CNA_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "cna-extended: CNA_EXTENDED_LINK_CNA=ON but no CNA CMake project at "
            "'${CNA_EXTENDED_CNA_DIR}'. Set -DCNA_EXTENDED_CNA_DIR=/path/to/cnanext.")
    endif()
    if(NOT EXISTS "${CNA_EXTENDED_SHARP_RUNTIME_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "cna-extended: CNA_EXTENDED_LINK_CNA=ON but no Sharp Runtime CMake project at "
            "'${CNA_EXTENDED_SHARP_RUNTIME_DIR}'. "
            "Set -DCNA_EXTENDED_SHARP_RUNTIME_DIR=/path/to/sharp-runtimenext.")
    endif()

    # Keep optional CNA surfaces out and select the smallest host-independent
    # implementation axes suitable for cna-extended's tests and consumers.
    set(CNA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CNA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(CNA_BUILD_C_API OFF CACHE BOOL "" FORCE)
    set(CNA_CNAEXT OFF CACHE BOOL "" FORCE)
    set(CNA_DEVICES OFF CACHE BOOL "" FORCE)
    set(CNA_ENABLE_NET OFF CACHE BOOL "" FORCE)
    set(CNA_ENABLE_DRACO OFF CACHE BOOL "" FORCE)
    set(CNA_GRAPHICS_RENDERER "${CNA_EXTENDED_CNA_RENDERER}" CACHE STRING "" FORCE)
    set(CNA_PLATFORM "${CNA_EXTENDED_CNA_PLATFORM}" CACHE STRING "" FORCE)
    set(CNA_AUDIO_PLATFORM "${CNA_EXTENDED_CNA_AUDIO_PLATFORM}" CACHE STRING "" FORCE)
    set(CNA_SHARP_RUNTIME_ROOT "${CNA_EXTENDED_SHARP_RUNTIME_DIR}" CACHE PATH "" FORCE)
    set(SHARP_RUNTIME_COMPONENTS "${CNA_EXTENDED_SHARP_RUNTIME_COMPONENTS}"
        CACHE STRING "Sharp Runtime components required by cna-extended and its CNA module closure" FORCE)
    set(SHARP_RUNTIME_BUILD_TESTS OFF CACHE BOOL "" FORCE)

    message(STATUS
        "cna-extended: adding modular CNA from '${CNA_EXTENDED_CNA_DIR}' "
        "(renderer: ${CNA_EXTENDED_CNA_RENDERER}, platform: ${CNA_EXTENDED_CNA_PLATFORM}, "
        "audio: ${CNA_EXTENDED_CNA_AUDIO_PLATFORM})")
    add_subdirectory("${CNA_EXTENDED_CNA_DIR}" "${CMAKE_BINARY_DIR}/_cna" EXCLUDE_FROM_ALL)
    set(_cna_extended_have_cna_target ON)
endif()

if(_cna_extended_have_cna_target)
    foreach(_required_target IN LISTS
            CNA_EXTENDED_CNA_MODULE_TARGETS
            CNA_EXTENDED_SHARP_RUNTIME_TARGETS)
        if(NOT TARGET ${_required_target})
            message(FATAL_ERROR
                "cna-extended: required modular dependency target '${_required_target}' is missing. "
                "When dependencies come from a parent, enable the cna-extended Sharp Runtime "
                "component selection before adding cna-extended.")
        endif()
    endforeach()

    target_link_libraries(CNA_EXTENDED PUBLIC
        ${CNA_EXTENDED_CNA_MODULE_TARGETS}
        ${CNA_EXTENDED_SHARP_RUNTIME_TARGETS}
    )
    target_compile_definitions(CNA_EXTENDED PUBLIC CNA_EXTENDED_HAS_CNA_LINK)
else()
    # Headers-only compile checking still follows the modular boundaries. This
    # is the public-header closure of the six direct CNA modules above.
    set(_cna_extended_cna_header_modules
        core math platform graphics input audio media content runtime)
    foreach(_module IN LISTS _cna_extended_cna_header_modules)
        target_include_directories(CNA_EXTENDED PUBLIC
            "${CNA_EXTENDED_CNA_DIR}/modules/${_module}/include")
    endforeach()

    # Public-header closure of the eight directly used Sharp Runtime targets.
    set(_cna_extended_sharp_header_modules
        core uri time-zone buffers collections io io-compression text text-json
        threading threading-tasks xml)
    foreach(_module IN LISTS _cna_extended_sharp_header_modules)
        target_include_directories(CNA_EXTENDED PUBLIC
            "${CNA_EXTENDED_SHARP_RUNTIME_DIR}/modules/${_module}/include")
    endforeach()
    target_include_directories(CNA_EXTENDED SYSTEM PUBLIC
        "${CNA_EXTENDED_SHARP_RUNTIME_DIR}/vendor")

    # CNA::BuildFlags normally publishes these definitions. Headers-only mode
    # deliberately does not create that target, so reproduce its API-selection
    # surface without pulling any implementation archive into the build.
    if(CNA_EXTENDED_CNA_RENDERER MATCHES "^(OPENGLES2|OPENGLES3|OPENGL33)$")
        target_compile_definitions(CNA_EXTENDED PUBLIC
            XNA5
            CNA_RENDERER_EASYGL
            "CNA_GL_PROFILE_${CNA_EXTENDED_CNA_RENDERER}")
    else()
        target_compile_definitions(CNA_EXTENDED PUBLIC
            XNA5
            "CNA_RENDERER_${CNA_EXTENDED_CNA_RENDERER}")
    endif()
    message(STATUS
        "cna-extended: modular headers-only compile check; CNA and Sharp Runtime archives are not linked")
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
    add_subdirectory(examples/world3d_demo)
endif()

if(CNA_EXTENDED_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
