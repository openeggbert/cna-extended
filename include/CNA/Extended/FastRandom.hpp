// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/FastRandom.cs. Upstream's private nested
// IFastRandomImpl/LinearCongruentialGeneratorImpl/ThreadSafeFastRandomImpl (a bridge/strategy
// pattern) are ported 1:1 as private nested types owned via std::unique_ptr. C#'s `[ThreadStatic]`
// becomes `thread_local` storage duration. `Shared`'s lazy-initialized static property becomes a
// function-local static (a deliberate choice after this session's earlier
// static-initialization-order bug in Matrix3x2::Identity -- see plan.md's decisions log -- to
// avoid the same cross-translation-unit init-order hazard).
#pragma once

#include "CNA/Extended/Interval.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <memory>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    /**
     * @brief A pseudo-random number generator using a linear congruential generator algorithm.
     *
     * This implementation uses the same constants as Microsoft Visual C++'s rand() function
     * (a=214013, c=2531011, m=2^31). It provides high performance and speed, but comes at the
     * price of having lower statistical quality, or true "randomness", compared to modern
     * algorithms. The algorithm is deterministic based on the initial seed value, making it
     * suitable for reproducible sequences.
     *
     * Note: This pseudo-random number generator exhibits noticeable patterns and should not be
     * used for cryptographic purposes or when a high-quality random distribution is critical.
     * Consider using System::Random for better statistical properties.
     */
    class FastRandom
    {
    public:
        /** @brief Gets a thread-safe FastRandom instance that may be used concurrently from any thread. */
        static FastRandom& getSharedProperty();

        /** @brief Initializes a new FastRandom using the default seed value. */
        FastRandom();

        /**
         * @brief Initializes a new FastRandom using the specified seed value.
         * @param seed A number used to calculate a starting value for the pseudo-random number
         * sequence. Must be positive.
         */
        explicit FastRandom(int seed);

        ~FastRandom();
        FastRandom(FastRandom&&) noexcept;
        FastRandom& operator=(FastRandom&&) noexcept;
        FastRandom(const FastRandom&) = delete;
        FastRandom& operator=(const FastRandom&) = delete;

        /**
         * @brief Returns a non-negative random integer.
         * @return A 32-bit signed integer that is greater than or equal to 0 and less than 32768.
         */
        [[nodiscard]] int Next();

        /**
         * @brief Returns a non-negative random integer that is less than or equal to the
         * specified maximum.
         * @param max The inclusive upper bound of the random number to be generated.
         */
        [[nodiscard]] int Next(int max);

        /**
         * @brief Returns a random integer that is within a specified range.
         * @param min The inclusive lower bound of the random number returned.
         * @param max The inclusive upper bound of the random number returned.
         */
        [[nodiscard]] int Next(int min, int max);

        /**
         * @brief Returns a random integer that is within a closed interval.
         * @param interval A closed interval representing the lower and upper bounds of the
         * random number to return.
         */
        [[nodiscard]] int Next(const Interval<int>& interval);

        /**
         * @brief Returns a random floating-point number that is greater than or equal to 0.0 and
         * less than 1.0.
         */
        [[nodiscard]] float NextSingle();

        /**
         * @brief Returns a random floating-point number that is greater than or equal to 0.0 and
         * less than the specified maximum.
         * @param max The exclusive upper bound of the random number generated.
         */
        [[nodiscard]] float NextSingle(float max);

        /**
         * @brief Returns a random floating-point number that is within a specified range.
         * @param min The inclusive lower bound of the random number returned.
         * @param max The exclusive upper bound of the random number returned.
         */
        [[nodiscard]] float NextSingle(float min, float max);

        /**
         * @brief Returns a random floating-point number that is within a closed interval.
         * @param interval A closed interval representing the lower and upper bounds of the
         * random number to return.
         */
        [[nodiscard]] float NextSingle(const Interval<float>& interval);

        /** @brief Returns a random angle between -pi and pi, in radians. */
        [[nodiscard]] float NextAngle();

        /**
         * @brief Gets a random unit vector.
         * @param vector Receives a unit vector with a random direction.
         */
        void NextUnitVector(Vector2& vector);

        /**
         * @brief Gets a random unit vector.
         * @param vector A pointer to the Vector2 where the random unit vector will be stored.
         */
        void NextUnitVector(Vector2* vector);

    private:
        class IFastRandomImpl
        {
        public:
            virtual ~IFastRandomImpl() = default;
            [[nodiscard]] virtual int Next() = 0;
            [[nodiscard]] virtual int Next(int max) = 0;
            [[nodiscard]] virtual int Next(int min, int max) = 0;
            [[nodiscard]] virtual int Next(const Interval<int>& interval) = 0;
            [[nodiscard]] virtual float NextSingle() = 0;
            [[nodiscard]] virtual float NextSingle(float max) = 0;
            [[nodiscard]] virtual float NextSingle(float min, float max) = 0;
            [[nodiscard]] virtual float NextSingle(const Interval<float>& interval) = 0;
            [[nodiscard]] virtual float NextAngle() = 0;
            virtual void NextUnitVector(Vector2& vector) = 0;
            virtual void NextUnitVector(Vector2* vector) = 0;
        };

        class LinearCongruentialGeneratorImpl final : public IFastRandomImpl
        {
        public:
            LinearCongruentialGeneratorImpl();
            explicit LinearCongruentialGeneratorImpl(int seed);

            [[nodiscard]] int Next() override;
            [[nodiscard]] int Next(int max) override;
            [[nodiscard]] int Next(int min, int max) override;
            [[nodiscard]] int Next(const Interval<int>& interval) override;
            [[nodiscard]] float NextSingle() override;
            [[nodiscard]] float NextSingle(float max) override;
            [[nodiscard]] float NextSingle(float min, float max) override;
            [[nodiscard]] float NextSingle(const Interval<float>& interval) override;
            [[nodiscard]] float NextAngle() override;
            void NextUnitVector(Vector2& vector) override;
            void NextUnitVector(Vector2* vector) override;

        private:
            static constexpr int kMultiplier = 214013;
            static constexpr int kIncrement = 2531011;

            int state_;
        };

        class ThreadSafeFastRandomImpl final : public IFastRandomImpl
        {
        public:
            [[nodiscard]] int Next() override;
            [[nodiscard]] int Next(int max) override;
            [[nodiscard]] int Next(int min, int max) override;
            [[nodiscard]] int Next(const Interval<int>& interval) override;
            [[nodiscard]] float NextSingle() override;
            [[nodiscard]] float NextSingle(float max) override;
            [[nodiscard]] float NextSingle(float min, float max) override;
            [[nodiscard]] float NextSingle(const Interval<float>& interval) override;
            [[nodiscard]] float NextAngle() override;
            void NextUnitVector(Vector2& vector) override;
            void NextUnitVector(Vector2* vector) override;

        private:
            static LinearCongruentialGeneratorImpl& LocalRandom();
        };

        explicit FastRandom(std::unique_ptr<IFastRandomImpl> impl);

        std::unique_ptr<IFastRandomImpl> impl_;
    };
}
