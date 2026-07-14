# SPDX-License-Identifier: MIT

# tiled_demo constructs a real GraphicsDevice/SpriteBatch/Texture2D/RenderTarget2D and parses a
# real Tiled TMX map, so *running* it needs CNA's compiled symbols, not just its headers -- same
# situation easy-3d's own examples/minimal/CMakeLists.txt documents. Only build it as a real
# executable when CNA is linkable (either the parent project provided the CNA target, or
# -DCNA_EXTENDED_LINK_CNA=ON built it). Otherwise, compile-check main.cpp as an OBJECT library
# (CNA/sharp-runtime headers alone are enough to compile it) so the default, no-CNA-link build
# still catches header/API drift here too.
if(CNA_EXTENDED_CNA_LINKED)
    add_executable(cna_extended_tiled_demo main.cpp)
    target_link_libraries(cna_extended_tiled_demo PRIVATE CNA_EXTENDED)
    if(MSVC)
        target_compile_options(cna_extended_tiled_demo PRIVATE /W4 /WX)
    else()
        target_compile_options(cna_extended_tiled_demo PRIVATE -Wall -Wextra -Werror)
    endif()
else()
    add_library(cna_extended_tiled_demo_compilecheck OBJECT main.cpp)
    target_link_libraries(cna_extended_tiled_demo_compilecheck PRIVATE CNA_EXTENDED)
    if(MSVC)
        target_compile_options(cna_extended_tiled_demo_compilecheck PRIVATE /W4 /WX)
    else()
        target_compile_options(cna_extended_tiled_demo_compilecheck PRIVATE -Wall -Wextra -Werror)
    endif()
    message(STATUS
        "cna-extended: 'tiled_demo' example compile-checked only, not built as an executable "
        "(needs CNA linked to run: -DCNA_EXTENDED_LINK_CNA=ON, or a parent project that defines the CNA target).")
endif()
