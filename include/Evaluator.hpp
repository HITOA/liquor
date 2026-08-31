//
// Created by HITO on 30/08/2026.
//

#pragma once

#include <Chess.hpp>
#include <Types.hpp>
#include <PiecesValue.hpp>
#include <PST.hpp>

#include <concepts>
#include <cstdint>


namespace LiquorChess
{

    template<typename T>
    concept Evaluator = requires(const T& evaluator, const chess::Board& board)
    {
        { evaluator.Evaluate(board) } -> std::same_as<Centipawn>;
    };

    class MinimalistHeuristic
    {
    public:
        MinimalistHeuristic() = default;
        ~MinimalistHeuristic() = default;

        static Centipawn Evaluate(const chess::Board& board)
        {
            const chess::Color us = board.sideToMove();
            const chess::Color them = ~us;
            return  (Heuristic::Material::Score(board, us) + Heuristic::Presence::Score(board, us)) -
                    (Heuristic::Material::Score(board, them) + Heuristic::Presence::Score(board, them));
        }
    };

}
