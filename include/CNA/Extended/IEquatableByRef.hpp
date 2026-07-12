// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#pragma once

namespace CNA::Extended
{
    /**
     * @brief Defines a generalized method that a value type or class implements to create a
     * type-specific method for determining equality of instances by reference.
     * @tparam T The type of values or objects to compare.
     */
    template <typename T>
    class IEquatableByRef
    {
    public:
        virtual ~IEquatableByRef() = default;

        /**
         * @brief Indicates whether the current value or object is equal to another value or
         * object of the same type by reference.
         * @param other A value or object to compare with this value or object.
         * @return true if the current value or object is equal to @p other; otherwise, false.
         */
        virtual bool Equals(T& other) = 0;
    };
}
