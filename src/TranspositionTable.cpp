#include <cmath>
#include <TranspositionTable.hpp>

LiquorChess::TranspositionTable::TranspositionTable() : entries{}
{
    Resize(1 << 22);
}

LiquorChess::TranspositionTable::TTEntry* LiquorChess::TranspositionTable::Probe(uint64_t key)
{
    uint64_t index = key & (entries.size() - 1);
    TTEntry* entry = &entries[index];
    if (entry->key != key)
        return nullptr;
    entry->generation = generation;
    return entry;
}

void LiquorChess::TranspositionTable::Store(uint64_t key, chess::Move bestMove, Centipawn score, uint32_t depth, TTEntry::Flag flag)
{
    assert(IsValid(score));
    uint64_t index = key & (entries.size() - 1);
    TTEntry* entry = &entries[index];
    if (entry->key == 0 || entry->generation != generation || entry->depth < depth)
    {
        entry->key = key;
        entry->depth = depth;
        entry->score = score;
        entry->flag = flag;
        entry->bestMove = bestMove;
    }
}

void LiquorChess::TranspositionTable::Resize(size_t size)
{
    size = std::pow(2, ceil(log(size - 1) / log(2)));
    entries.resize(size);
    Clear();
}

void LiquorChess::TranspositionTable::NewGeneration()
{
    generation += 1;
}

void LiquorChess::TranspositionTable::Clear()
{
    std::fill(entries.begin(), entries.end(), TTEntry{});
    generation = 0;
}

LiquorChess::Centipawn LiquorChess::TranspositionTable::ScoreToTT(Centipawn score, uint32_t ply)
{
    if (score >= CENTIPAWN_MATE_IN_MAX_PLY) return score + ply;
    if (score <= CENTIPAWN_MATED_IN_MAX_PLY) return score - ply;
    return score;
}

LiquorChess::Centipawn LiquorChess::TranspositionTable::ScoreFromTT(Centipawn score, uint32_t ply)
{
    if (score >= CENTIPAWN_MATE_IN_MAX_PLY) return score - ply;
    if (score <= CENTIPAWN_MATED_IN_MAX_PLY) return score + ply;
    return score;
}