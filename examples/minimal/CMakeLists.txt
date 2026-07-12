# SPDX-License-Identifier: MIT

# As of Phase 0, the only ported functionality is Version — it does not touch
# CNA, so this example always builds as a real executable. Once later phases
# add functionality that needs a linked CNA (Graphics, Tilemaps, ...), extend
# this the way easy-3d's examples/minimal/CMakeLists.txt does: gate the
# add_executable() on CNA_EXTENDED_CNA_LINKED and fall back to an OBJECT
# library compile-check otherwise.
add_executable(cna_extended_minimal main.cpp)
target_link_libraries(cna_extended_minimal PRIVATE CNA_EXTENDED)
