// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tweening/Tweener.hpp"

namespace CNA::Extended::Tweening
{
    void Tweener::Dispose()
    {
        CancelAll();
        activeTweens_.clear();
    }

    void Tweener::Update(float elapsedSeconds)
    {
        for (std::size_t i = activeTweens_.size(); i-- > 0;)
        {
            Tween* tween = activeTweens_[i].get();
            tween->Update(elapsedSeconds);

            if (!tween->getIsAliveProperty())
            {
                activeTweens_.erase(activeTweens_.begin() + static_cast<std::ptrdiff_t>(i));
            }
        }
    }

    void Tweener::CancelAll()
    {
        for (auto& tween : activeTweens_)
        {
            tween->Cancel();
        }
    }

    void Tweener::CancelAndCompleteAll()
    {
        for (auto& tween : activeTweens_)
        {
            tween->CancelAndComplete();
        }
    }
}
