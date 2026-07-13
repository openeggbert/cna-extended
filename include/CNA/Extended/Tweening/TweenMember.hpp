// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// *** GENUINE C++ REDESIGN, NOT A LITERAL PORT -- approved by the user for this exact tradeoff ***
// Replaces upstream's entire reflection-based member-access hierarchy: TweenMember.cs's abstract
// `TweenMember`/`TweenMember<T>`, TweenFieldMember.cs's `TweenFieldMember<T>` (expression-tree-
// compiled FieldInfo access), and TweenPropertyMember.cs's `TweenPropertyMember<T>` (expression-
// tree-compiled PropertyInfo access). C++ has neither runtime reflection nor expression trees, so
// a literal port is impossible; the C# lambda member-selector (`x => x.Position`) is replaced with
// C++ pointer-to-member syntax (`&TargetType::Position`) -- syntactically similar at the call
// site, fully compile-time type-checked, and needs no reflection machinery at all. This single
// TweenMember<TTarget, TMember> class replaces the whole 4-class hierarchy: a C++ pointer-to-data-
// member already works uniformly for what upstream needed two separate reflection paths
// (FieldInfo vs PropertyInfo) to unify.
//
// LIMITATION, not present upstream: pointer-to-member syntax addresses public DATA MEMBERS
// (fields) only. This project's own types often expose "C# properties" via getXProperty()/
// setXProperty() method pairs instead of public fields (see CLAUDE.md's established convention).
// Animating a getXProperty()/setXProperty() pair via this mechanism is out of scope -- only types
// with an actual public field for the animated value are tweenable. Most simple animatable
// values in this project (Vector2::X/Y, Color, float fields, etc.) are already public fields, so
// this covers the common case; it does not cover types that only expose computed or validated
// properties through accessor methods.
#pragma once

namespace CNA::Extended::Tweening
{
    /**
     * @brief Binds a target object and a pointer-to-member, giving typed get/set access to that
     * member's value. Replaces upstream's reflection-based TweenMember/TweenFieldMember/
     * TweenPropertyMember hierarchy -- see this file's header comment for why.
     * @tparam TTarget The type of the target object.
     * @tparam TMember The value type of the member being animated.
     */
    template <typename TTarget, typename TMember>
    class TweenMember
    {
    public:
        /**
         * @brief Binds the specified target object and member.
         * @param target The object whose member will be animated. Must outlive this TweenMember.
         * @param member Pointer to the public data member to animate.
         */
        TweenMember(TTarget* target, TMember TTarget::*member) : target_(target), member_(member) {}

        /** @brief Gets the current value of the bound member on the target object. */
        [[nodiscard]] TMember getValueProperty() const { return target_->*member_; }

        /**
         * @brief Sets the value of the bound member on the target object.
         * @remark Marked const: this mutates the TARGET's member through the stored pointer, not
         * this TweenMember object itself (a const TweenMember still owns a mutable TTarget*, the
         * same way a const-qualified pointer variable doesn't make its pointee const). Needed so
         * TypedTween-derived classes can call this through a const TweenMember& (see
         * TypedTween::getMemberProperty()'s const-reference return).
         */
        void setValueProperty(TMember value) const { target_->*member_ = value; }

        /** @brief Gets the target object whose member is being animated. */
        [[nodiscard]] TTarget* getTargetProperty() const { return target_; }

        /** @brief Gets the bound pointer-to-member, usable to identify which member is targeted (e.g. to find/cancel an existing tween on the same member). */
        [[nodiscard]] TMember TTarget::*getMemberPointerProperty() const { return member_; }

    private:
        TTarget* target_;
        TMember TTarget::*member_;
    };
}
