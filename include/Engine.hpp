#pragma once

#include <CommandBuffer.hpp>

#include <atomic>
#include <Chess.hpp>

#include <thread>


namespace LiquorChess {
    class Interface;

    class Engine {
    private:
        constexpr static int32_t MAX_PLY = 32;
        constexpr static int32_t KILLER_COUNT = 3;

    public:
        Engine(Interface* interface);
        ~Engine() = default;

        void SetBoardInternal(const std::string& fen);
        void MakeMove(const std::string& move);

        void Run();
        void Stop();

        [[nodiscard]] inline bool IsRunning() const
        {
            return searching.load(std::memory_order_relaxed);
        }

    protected:
        void Search();

        int32_t Negamax(chess::Board& board, int32_t depth, int32_t a, int32_t b, uint32_t& nodes, uint32_t ply = 0);
        int32_t Quiescence(chess::Board& board, int32_t a, int32_t b, uint32_t& nodes);
        int32_t Evaluate(chess::Board& board);
        int32_t MaterialScore(chess::Board& board, chess::Color color);
        int32_t PresenceScore(chess::Board& board, chess::Color color);
        int32_t MvvLvaScore(chess::Board& board, chess::Move& move);

        int16_t IsKiller(chess::Move& move, uint32_t ply);
        void TryInsertKiller(chess::Move& move, uint32_t ply);
        void ClearKillers();

    private:
        Interface* interface;

        chess::Board board;

        std::atomic<bool> searching;
        std::jthread searchThread;

        chess::Move killers[MAX_PLY][KILLER_COUNT];
    };

}