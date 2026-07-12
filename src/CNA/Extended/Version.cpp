// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/Version.hpp"

#define CNA_EXTENDED_STR2(x) #x
#define CNA_EXTENDED_STR(x) CNA_EXTENDED_STR2(x)

namespace CNA::Extended
{
    const char* VersionString() noexcept
    {
        return CNA_EXTENDED_STR(CNA_EXTENDED_VERSION_MAJOR) "."
               CNA_EXTENDED_STR(CNA_EXTENDED_VERSION_MINOR) "."
               CNA_EXTENDED_STR(CNA_EXTENDED_VERSION_PATCH);
    }
}

#undef CNA_EXTENDED_STR
#undef CNA_EXTENDED_STR2
