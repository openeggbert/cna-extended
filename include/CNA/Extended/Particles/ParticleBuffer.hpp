// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/ParticleBuffer.cs: a circular buffer of raw,
// unconstructed Particle memory. Upstream's `Marshal.AllocHGlobal(SizeInBytes)` (raw unmanaged
// bytes, no constructor calls) -> `std::malloc`/`std::free` here, not `new Particle[]` (which
// would default-construct every slot -- a real behavioral difference upstream deliberately avoids
// via unmanaged allocation). Since `Particle` is a fixed-size POD, byte-pointer arithmetic on
// upstream's `IntPtr NativePointer` is equivalent to `Particle*` pointer arithmetic here (which
// auto-scales by `sizeof(Particle)`), so `NativePointer`/`Head`/`Tail`/`BufferEnd` are all typed
// directly as `Data::Particle*` rather than mirroring the IntPtr-then-cast pattern. `GC
// .Add/RemoveMemoryPressure` (informs the GC of unmanaged allocation size) has no C++ equivalent
// (no GC) and is dropped.
#pragma once

#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/ParticleIterator.hpp"
#include "System/IDisposable.hpp"

namespace CNA::Extended::Particles
{
    /** @brief A circular buffer of Particle slots in contiguous, unconstructed heap memory, with head/tail-based allocation. */
    class ParticleBuffer : public System::IDisposable
    {
    public:
        /** @param size The maximum number of particles this buffer can hold. */
        explicit ParticleBuffer(int size);

        ~ParticleBuffer() override;

        ParticleBuffer(const ParticleBuffer&) = delete;
        ParticleBuffer& operator=(const ParticleBuffer&) = delete;

        /** @brief Gets the raw pointer to the start of the buffer's unconstructed memory. Invalid after Dispose(). */
        [[nodiscard]] Data::Particle* getNativePointerProperty() const { return nativePointer_; }

        /** @brief Gets the current tail position (where the next Release() allocates). */
        [[nodiscard]] Data::Particle* getTailProperty() const { return tail_; }

        /** @brief Gets the pointer just past the end of the allocated buffer memory, used for wraparound bounds checking. */
        [[nodiscard]] Data::Particle* getBufferEndProperty() const { return bufferEnd_; }

        /** @brief Gets the maximum number of particles this buffer can store. */
        [[nodiscard]] int getSizeProperty() const { return size_; }

        /** @brief Gets an iterator over the currently active particles, reset to the current head. */
        [[nodiscard]] ParticleIterator& getIteratorProperty() { return iterator_.Reset(); }

        /** @brief Gets the current head position (the oldest active particle). */
        [[nodiscard]] Data::Particle* getHeadProperty() const { return head_; }

        /** @brief Gets the number of additional particles that can be released before the buffer is full. */
        [[nodiscard]] int getAvailableProperty() const { return size_ - count_; }

        /** @brief Gets the current number of active particles. */
        [[nodiscard]] int getCountProperty() const { return count_; }

        /** @brief Gets the total buffer size in bytes (capacity + 1 slot, to distinguish full from empty in the circular layout). */
        [[nodiscard]] int getSizeInBytesProperty() const;

        /** @brief Gets the size in bytes of the currently active portion of the buffer. */
        [[nodiscard]] int getActiveSizeInBytesProperty() const;

        [[nodiscard]] bool getIsDisposedProperty() const { return isDisposed_; }

        /**
         * @brief Allocates space at the tail for up to @p releaseQuantity new particles (fewer if the buffer doesn't have room).
         * @return An iterator positioned at the start of the newly allocated slots.
         * @throws System::ObjectDisposedException this buffer was already disposed.
         */
        ParticleIterator& Release(int releaseQuantity);

        /**
         * @brief Removes @p number particles from the head (oldest first).
         * @throws System::ObjectDisposedException this buffer was already disposed.
         */
        void Reclaim(int number);

        void Dispose() override;

    private:
        int size_;
        Data::Particle* nativePointer_;
        Data::Particle* bufferEnd_;
        Data::Particle* head_;
        Data::Particle* tail_;
        int count_ = 0;
        bool isDisposed_ = false;
        ParticleIterator iterator_;
    };
}
