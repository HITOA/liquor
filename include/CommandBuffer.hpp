//
// Created by HITO on 27/08/2026.
//

#pragma once

#include <atomic>
#include <cassert>
#include <cstdlib>


namespace LiquorChess
{
    /**
     * Minimal MPSC ring buffer
     */
    template<typename T, size_t N>
    class CommandBuffer
    {
    public:
        CommandBuffer() : buffer{ nullptr }, writeIndex{ 0 }, readIndex{ 0 }
        {
            buffer = static_cast<T*>(malloc(N * sizeof(T)));
        }
        ~CommandBuffer()
        {
            if (buffer != nullptr)
                free(buffer);
        }

        [[nodiscard]] bool Empty() const
        {
            return !ready[readIndex];
        }

        void Push(const T& value)
        {
            size_t index = writeIndex.fetch_add(1, std::memory_order_relaxed) % N;
            buffer[index] = value;
            ready[index].store(true, std::memory_order_release);
        }

        [[nodiscard]] T Pop()
        {
            assert(!Empty());
            while (!ready[readIndex].load(std::memory_order_acquire)) {}
            T value = buffer[readIndex];
            ready[readIndex].store(false, std::memory_order_release);
            readIndex = (readIndex + 1) % N;
            return value;
        }

    private:
        T* buffer;

        std::atomic_size_t writeIndex;
        size_t readIndex;

        std::atomic_bool ready[N];
    };

}
