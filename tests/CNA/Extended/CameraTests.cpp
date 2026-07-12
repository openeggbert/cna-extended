// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Camera<T>::getBoundingRectangleProperty() returns RectangleF, which is only forward-declared
// in Camera.hpp (not yet ported -- Phase 1, "RectangleF family"). A concrete override of that
// pure virtual method needs a complete RectangleF to construct/return one, so no concrete
// Camera<T> subclass can be instantiated here yet. This file only proves the header parses on
// its own; append real instantiation-based tests (a small concrete Camera<Vector2> test double,
// mirroring InterfaceTests.cpp's pattern) once RectangleF lands.
#include "CNA/Extended/Camera.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(CameraTests, HeaderCompilesStandalone)
    {
        SUCCEED();
    }
}
