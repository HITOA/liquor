//
// Created by HITO on 30/08/2026.
//

#pragma once

#include <Chess.hpp>
#include <Types.hpp>
#include <Heuristic/Material.hpp>
#include <Heuristic/PST.hpp>
#include <Heuristic/PawnStructure.hpp>

#include <MantaRay/Activation/ClippedReLU.h>
#include <MantaRay/Perspective/PerspectiveNNUE.h>

#include <concepts>


namespace LiquorChess
{

    template<typename T>
    concept Evaluator = requires(T& evaluator, const chess::Board& board)
    {
        { evaluator.Evaluate(board) } -> std::same_as<Centipawn>;
    };

    template<typename T>
    concept IncrementalEvaluator = Evaluator<T> && requires(T& evaluator, const chess::Board& board, const chess::Move& move)
    {
        { evaluator.Update(board) } -> std::same_as<void>;
        { evaluator.MakeMove(board, move) } -> std::same_as<void>;
        { evaluator.UnmakeMove(board, move) } -> std::same_as<void>;
        { evaluator.MakeNullMove(board) } -> std::same_as<void>;
        { evaluator.UnmakeNullMove(board) } -> std::same_as<void>;
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

    class HandCraftedEvaluator
    {
    public:
        HandCraftedEvaluator() = default;
        ~HandCraftedEvaluator() = default;

        static Centipawn Evaluate(const chess::Board& board)
        {
            const chess::Color us = board.sideToMove();
            const chess::Color them = ~us;

            return (Heuristic::Material::Score(board, us) +
                    Heuristic::Presence::Score(board, us) +
                    Heuristic::PawnStructure::Score(board, us)) -

                   (Heuristic::Material::Score(board, them) +
                    Heuristic::Presence::Score(board, them) +
                    Heuristic::PawnStructure::Score(board, them));
        }
    };

    class NNUEEvaluator
    {
        using Activation = MantaRay::ClippedReLU<int16_t, 0, 255>;
        using NeuralNetwork = MantaRay::PerspectiveNetwork<int16_t, int32_t, Activation, 768, 512, 1, 512, 400, 255, 64>;

    public:
        NNUEEvaluator();
        ~NNUEEvaluator();

        Centipawn Evaluate(const chess::Board& board);
        void Update(const chess::Board& board);
        void MakeMove(const chess::Board& board, const chess::Move& move);
        void UnmakeMove(const chess::Board& board, const chess::Move& move);
        static void MakeNullMove(const chess::Board& board) {}
        static void UnmakeNullMove(const chess::Board& board) {}

    private:
        NeuralNetwork network;
    };

}
