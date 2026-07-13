// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's GameComponentCollectionExtensions.cs. C# extension methods on
// Microsoft.Xna.Framework.GameComponentCollection -> free function templates taking
// GameComponentCollection& in this namespace, matching the convention used throughout this
// project (see RandomExtensions.hpp). C#'s `Func<T>` factory parameter maps to
// std::function<T*()> (matching this project's established Func<T> convention, see
// HslColor.hpp), returning T* rather than T since GameComponentCollection stores raw
// IGameComponent* pointers (CNA's own translation of C#'s reference-type semantics for this
// type -- GameComponentCollection does not take ownership of/delete its items, matching upstream
// XNA/FNA's own non-owning-collection design; the caller is responsible for the component's
// lifetime, same as calling `new T()` directly in C++). Header-only: a template, like
// Interval<T>.
#pragma once

#include "Microsoft/Xna/Framework/GameComponentCollection.hpp"
#include "Microsoft/Xna/Framework/IGameComponent.hpp"

#include <functional>
#include <type_traits>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::GameComponentCollection;
    using Microsoft::Xna::Framework::IGameComponent;

    /**
     * @brief Constructs a new T via its default constructor and adds it to the collection.
     * @tparam T The IGameComponent-implementing type to construct and add.
     * @return A pointer to the newly constructed and added component.
     */
    template <typename T>
    [[nodiscard]] T* Add(GameComponentCollection& collection)
    {
        static_assert(std::is_base_of_v<IGameComponent, T>, "T must implement IGameComponent");
        static_assert(std::is_default_constructible_v<T>, "T must be default-constructible");

        T* gameComponent = new T();
        collection.Add(gameComponent);
        return gameComponent;
    }

    /**
     * @brief Constructs a new T via the specified factory function and adds it to the collection.
     * @tparam T The IGameComponent-implementing type to construct and add.
     * @param createGameComponent A factory function that constructs and returns the new component.
     * @return A pointer to the newly constructed and added component.
     */
    template <typename T>
    [[nodiscard]] T* Add(GameComponentCollection& collection, const std::function<T*()>& createGameComponent)
    {
        static_assert(std::is_base_of_v<IGameComponent, T>, "T must implement IGameComponent");

        T* gameComponent = createGameComponent();
        collection.Add(gameComponent);
        return gameComponent;
    }
}
