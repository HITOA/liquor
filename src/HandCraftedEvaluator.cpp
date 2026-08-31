#include <Evaluator.hpp>

#include <../include/Heuristic/Material.hpp>


LiquorChess::Centipawn LiquorChess::HandCraftedEvaluator::Evaluate(const chess::Board& board)
{
    chess::Color us = board.sideToMove();
    chess::Color them = ~us;

    Centipawn materialScore = Heuristic::Material::Score(board, us) - Heuristic::Material::Score(board, them);
    Centipawn pstScore = Heuristic::Presence::Score(board, us) - Heuristic::Material::Score(board, them);

    return materialScore + pstScore;
}