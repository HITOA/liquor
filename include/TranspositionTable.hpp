//
// Created by HITO on 30/08/2026.
//

#pragma once
#include "Chess.hpp"
#include "Types.hpp"


namespace LiquorChess
{

    class TranspositionTable
    {
    public:
        struct TTEntry
        {
            uint64_t key = 0;
            chess::Move bestMove{};
            Centipawn score = -CENTIPAWN_INFINITE;
            uint32_t depth = 0;
            uint8_t generation = 0;
            enum class Flag
            {
                EXACT,
                LOWERBOUND,
                UPPERBOUND
            } flag;
        };

    public:
        TranspositionTable();
        ~TranspositionTable() = default;

        TTEntry* Probe(uint64_t key);
        void Store(uint64_t key, chess::Move bestMove, Centipawn score, uint32_t depth, TTEntry::Flag flag);

        void Resize(size_t size);

        void NewGeneration();
        void Clear();

        static Centipawn ScoreToTT(Centipawn score, uint32_t ply);
        static Centipawn ScoreFromTT(Centipawn score, uint32_t ply);

    private:
        uint8_t generation = 0;
        std::vector<TTEntry> entries;
    };

}
