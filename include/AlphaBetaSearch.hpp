//
// Created by HITO on 30/08/2026.
//

#pragma once

#include <chrono>
#include <SearchEngine.hpp>
#include <Types.hpp>

#include "StringHash.hpp"
#include "TranspositionTable.hpp"


namespace LiquorChess
{

    struct AlphaBetaSearchInfo : public SearchInfo
    {
        uint32_t depth;
        uint32_t seldepth;
        uint32_t nodes;
        uint32_t nps;
        std::vector<chess::Move> pv{};
    };

    template<Evaluator EvalT>
    class AlphaBetaSearch : public SearchEngine
    {
    public:
        chess::Move Search(const std::stop_token stop, const SearchParameters& parameters) override
        {
            ClearKillers();

            chess::Board board = parameters.board;

            if constexpr (IncrementalEvaluator<EvalT>)
                evaluator.Update(board);

            int64_t remaining = parameters.limit.count();

            chess::Move bestMove{};
            uint32_t depth = 2;

            chess::Movelist legalMoves;
            chess::movegen::legalmoves(legalMoves, board);

            size_t lastNodeCount = 0;

            while (!stop.stop_requested())
            {
                std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

                ClearNodeCount();
                ClearMaxPly();

                Centipawn score = Negamax(board, depth, 0, 0, -CENTIPAWN_INFINITE, CENTIPAWN_INFINITE);

                if (TranspositionTable::TTEntry* entry = tt.Probe(board.hash()))
                    bestMove = entry->bestMove;

                std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
                std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                if (duration.count() <= 0)
                    duration = std::chrono::milliseconds{ 1 };

                if (parameters.observer)
                {
                    std::unique_ptr<AlphaBetaSearchInfo> info = std::make_unique<AlphaBetaSearchInfo>();
                    info->hash = CTFNV1A("AlphaBetaSearchInfo");
                    info->elapsed = duration;
                    info->score = score;
                    info->depth = depth;
                    info->seldepth = GetMaxPly();
                    info->nodes = GetNodeCount();
                    info->nps = GetNodeCount() * 1000 / duration.count();
                    BuildPrincipalVariation(board, info->pv);
                    PushSearchInfo(parameters.observer, std::move(info));
                }

                remaining -= duration.count();
                lastNodeCount = lastNodeCount > 0 ? lastNodeCount : GetNodeCount();
                double mpn = static_cast<double>(duration.count()) / static_cast<double>(GetNodeCount());
                double growth = static_cast<double>(GetNodeCount()) / static_cast<double>(lastNodeCount);
                int64_t estimate = static_cast<int64_t>((GetNodeCount() * growth) * mpn);
                if (estimate > remaining)
                    break;

                lastNodeCount = GetNodeCount();

                ++depth;
            }

            return bestMove;
        }

    private:
        void BuildPrincipalVariation(chess::Board board, std::vector<chess::Move>& pv)
        {
            for (size_t i = 0; i < MAX_PV_LENGTH; ++i)
            {
                TranspositionTable::TTEntry* entry = tt.Probe(board.hash());
                if (entry == nullptr || entry->flag != TranspositionTable::TTEntry::Flag::EXACT)
                    break;

                chess::Move bestMove = entry->bestMove;
                if (!board.isLegal(bestMove))
                    break;
                pv.push_back(bestMove);
                board.makeMove(bestMove);
            }
        }

        Centipawn Negamax(chess::Board& board, uint32_t limit, uint32_t ply, uint32_t extension, Centipawn alpha, Centipawn beta)
        {
            IncrementNodeCount();

            if (board.isRepetition() || board.isHalfMoveDraw())
                return CENTIPAWN_DRAW;

            if (board.inCheck() && extension < MAX_EXTENSION)
            {
                limit += 1;
                extension += 1;
            }

            if (limit == 0)
                return Quiescence(board, ply, alpha, beta);

            Centipawn originalAlpha = alpha;

            chess::Move ttMove{};
            if (TranspositionTable::TTEntry* entry = tt.Probe(board.hash()))
            {
                ttMove = entry->bestMove;
                Centipawn entryScore = TranspositionTable::ScoreFromTT(entry->score, ply);
                if (entry->depth >= limit)
                {
                    if (entry->flag == TranspositionTable::TTEntry::Flag::EXACT) return entryScore;
                    if (entry->flag == TranspositionTable::TTEntry::Flag::LOWERBOUND) alpha = std::max(alpha, entryScore);
                    if (entry->flag == TranspositionTable::TTEntry::Flag::UPPERBOUND) beta = std::min(beta, entryScore);
                    if (alpha >= beta) return entryScore;
                }
            }

            chess::Movelist legalMoves;
            chess::movegen::legalmoves(legalMoves, board);

            if (legalMoves.empty() && board.inCheck())
                return -CENTIPAWN_MATE + ply;
            if (legalMoves.empty())
                return CENTIPAWN_DRAW;

            OrderMoveList(board, legalMoves, ttMove, ply);

            chess::Move bestMove{};
            Centipawn bestScore = -CENTIPAWN_INFINITE;

            for (size_t i = 0; i < legalMoves.size(); ++i)
            {
                chess::Move& move = legalMoves[i];
                MakeMove(board, move);
                int32_t score = 0;
                if (i > 0)
                    score = -Negamax(board, limit - 1, ply + 1, extension, -alpha - 1, -alpha);
                if (i == 0 || (score > alpha && score < beta))
                    score = -Negamax(board, limit - 1, ply + 1, extension, -beta, -alpha);
                UnmakeMove(board, move);

                if (score > bestScore)
                {
                    bestMove = move;
                    bestScore = score;
                }
                alpha = std::max(alpha, bestScore);
                if (alpha >= beta)
                {
                    if (!board.isCapture(move) && move != killers[ply][0])
                    {
                        killers[ply][1] = killers[ply][0];
                        killers[ply][0] = move;
                    }
                    break;
                }
            }

            TranspositionTable::TTEntry::Flag flag =
                bestScore <= originalAlpha ? TranspositionTable::TTEntry::Flag::UPPERBOUND
              : bestScore >= beta          ? TranspositionTable::TTEntry::Flag::LOWERBOUND
              : TranspositionTable::TTEntry::Flag::EXACT;
            tt.Store(board.hash(), bestMove, TranspositionTable::ScoreToTT(bestScore, ply), limit, flag);

            return bestScore;
        }

        Centipawn Quiescence(chess::Board& board, uint32_t ply, Centipawn alpha, Centipawn beta)
        {
            IncrementNodeCount();
            MaxPly(ply);

            Centipawn originalAlpha = alpha;

            chess::Move ttMove{};
            if (TranspositionTable::TTEntry* entry = tt.Probe(board.hash()))
            {
                ttMove = entry->bestMove;
                Centipawn entryScore = TranspositionTable::ScoreFromTT(entry->score, ply);
                if (entry->flag == TranspositionTable::TTEntry::Flag::EXACT) return entryScore;
                if (entry->flag == TranspositionTable::TTEntry::Flag::LOWERBOUND) alpha = std::max(alpha, entryScore);
                if (entry->flag == TranspositionTable::TTEntry::Flag::UPPERBOUND) beta = std::min(beta, entryScore);
                if (alpha >= beta) return entryScore;
            }

            Centipawn bestScore = -CENTIPAWN_INFINITE;
            if (!board.inCheck())
            {
                bestScore = evaluator.Evaluate(board);
                if (bestScore >= beta)
                    return bestScore;
                alpha = std::max(alpha, bestScore);
            }

            chess::Movelist legalMoves;

            if (board.inCheck())
                chess::movegen::legalmoves(legalMoves, board);
            else
                chess::movegen::legalmoves<chess::movegen::MoveGenType::CAPTURE>(legalMoves, board);

            if (legalMoves.empty())
                return board.inCheck() ? CENTIPAWN_MATE + ply : bestScore;

            OrderMoveList(board, legalMoves, ttMove, ply);

            chess::Move bestMove{};

            for (size_t i = 0; i < legalMoves.size(); ++i)
            {
                chess::Move& move = legalMoves[i];
                MakeMove(board, move);
                int32_t score = 0;
                if (i > 0)
                    score = -Quiescence(board, ply + 1, -alpha - 1, -alpha);
                if (i == 0 || (score > alpha && score < beta))
                    score = -Quiescence(board, ply + 1, -beta, -alpha);
                UnmakeMove(board, move);

                if (score > bestScore)
                {
                    bestMove = move;
                    bestScore = score;
                }
                alpha = std::max(alpha, bestScore);
                if (alpha >= beta)
                    break;
            }

            if (board.isLegal(bestMove))
            {
                TranspositionTable::TTEntry::Flag flag =
                    bestScore <= originalAlpha ? TranspositionTable::TTEntry::Flag::UPPERBOUND
                  : bestScore >= beta          ? TranspositionTable::TTEntry::Flag::LOWERBOUND
                  : TranspositionTable::TTEntry::Flag::EXACT;
                tt.Store(board.hash(), bestMove, TranspositionTable::ScoreToTT(bestScore, ply), 0, flag);
            }

            return bestScore;
        }

        void OrderMoveList(chess::Board& board, chess::Movelist& moves, chess::Move ttMove, uint32_t ply)
        {
            for (auto& move : moves)
            {
                if (move == ttMove) move.setScore(10000);
                else if (board.isCapture(move)) move.setScore(MvvLvaScore(board, move));
                else if (move == killers[ply][0]) move.setScore(10);
                else if (move == killers[ply][1]) move.setScore(9);
                else move.setScore(0);
            }
            std::sort(moves.begin(), moves.end(), [ttMove](chess::Move& v1, chess::Move& v2)
            {
                if (v1 == ttMove) return true;
                if (v2 == ttMove) return false;
                return v1.score() > v2.score();
            });
        }

        Centipawn MvvLvaScore(chess::Board& board, chess::Move& move)
        {
            Centipawn victimScore = Heuristic::Material::PIECES_VALUE[board.at<chess::PieceType>(move.to())];
            Centipawn attackerScore = Heuristic::Material::PIECES_VALUE[board.at<chess::PieceType>(move.from())];
            return victimScore * 100 - attackerScore;
        }

        void MakeMove(chess::Board& board, chess::Move& move)
        {
            if constexpr (IncrementalEvaluator<EvalT>)
                evaluator.MakeMove(board, move);
            board.makeMove(move);
        }

        void UnmakeMove(chess::Board& board, chess::Move& move)
        {
            if constexpr (IncrementalEvaluator<EvalT>)
                evaluator.UnmakeMove(board, move);
            board.unmakeMove(move);
        }

        void ClearNodeCount() { nodeCount = 0; }
        void IncrementNodeCount() { ++nodeCount; }
        size_t GetNodeCount() { return nodeCount; }

        void ClearMaxPly() { maxPly = 0; }
        void MaxPly(uint32_t ply) { maxPly = std::max(maxPly, ply); }
        uint32_t GetMaxPly() { return maxPly; }

        void ClearKillers()
        {
            std::fill(&killers[0][0], &killers[0][0] + MAX_PLY * 2, chess::Move{});
        }

    private:
        EvalT evaluator;
        size_t nodeCount = 0;
        uint32_t maxPly = 0;

        TranspositionTable tt{};
        chess::Move killers[MAX_PLY][2]{};
    };

}
