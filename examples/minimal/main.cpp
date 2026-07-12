// SPDX-License-Identifier: MIT
//
// Minimal cna-extended example.
//
// As of Phase 0, cna-extended only carries version info — this just proves the
// library builds and links. As phases land, extend this to show off a small
// slice of each (see easy-3d's minimal example for the target shape once CNA
// linkage is required).

#include <CNA/Extended.hpp>

#include <iostream>

int main()
{
    std::cout << "cna-extended " << CNA::Extended::VersionString() << '\n';
    return 0;
}
