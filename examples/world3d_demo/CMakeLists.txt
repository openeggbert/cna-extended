# SPDX-License-Identifier: MIT

# world3d_demo constructs a real GraphicsDevice/RenderTarget2D/World3DEXT scene, so *running*
# it needs CNA's compiled symbols, not just its headers -- same situation as tiled_demo's own
# CMakeLists.txt (see that file's own comment). Only build it as a real executable when CNA is
# linkable; otherwise compile-check main.cpp as an OBJECT library so the default,
# no-CNA-link build still catches header/API drift here too.
if(CNA_EXTENDED_CNA_LINKED)
    add_executable(cna_extended_world3d_demo main.cpp)
    target_link_libraries(cna_extended_world3d_demo PRIVATE CNA_EXTENDED)
    if(MSVC)
        target_compile_options(cna_extended_world3d_demo PRIVATE /W4 /WX)
    else()
        target_compile_options(cna_extended_world3d_demo PRIVATE -Wall -Wextra -Werror)
    endif()
else()
    add_library(cna_extended_world3d_demo_compilecheck OBJECT main.cpp)
    target_link_libraries(cna_extended_world3d_demo_compilecheck PRIVATE CNA_EXTENDED)
    if(MSVC)
        target_compile_options(cna_extended_world3d_demo_compilecheck PRIVATE /W4 /WX)
    else()
        target_compile_options(cna_extended_world3d_demo_compilecheck PRIVATE -Wall -Wextra -Werror)
    endif()
    message(STATUS
        "cna-extended: 'world3d_demo' example compile-checked only, not built as an executable "
        "(needs CNA linked to run: -DCNA_EXTENDED_LINK_CNA=ON, or a parent project that defines the CNA target).")
endif()
