// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#pragma once

/// @file Extended.hpp
/// @brief Umbrella header for cna-extended.
///
/// cna-extended is a C++ port of MonoGame.Extended for CNA. Its API uses CNA / XNA-style
/// types (Vector2, Rectangle, SpriteBatch, ...) directly and lives in the CNA::Extended
/// namespace (and per-module sub-namespaces, e.g. CNA::Extended::Tilemaps). See
/// README.md, plan.md, and CLAUDE.md.
///
/// Like MonoGame.Extended itself, most consumers should prefer including only the
/// per-module headers/umbrellas they need (e.g. "CNA/Extended/Tilemaps.hpp") rather than
/// this file — it will end up pulling in every ported module as phases land.

#include "CNA/Extended/Version.hpp"
