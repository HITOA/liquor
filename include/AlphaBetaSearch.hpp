//
// Created by HITO on 30/08/2026.
//

#pragma once

#include <chrono>
#include <SearchEngine.hpp>
#include <Types.hpp>

#include <StringHash.hpp>
#include <TranspositionTable.hpp>
#include <PVTable.hpp>


namespace LiquorChess
{

    struct AlphaBetaSearchInfo : public SearchInfo
    {
        uint32_t seldepth{};
        uint32_t nodes{};
        uint32_t nps{};
        std::vector<chess::Move> pv{};
    };

    template<Evaluator EvalT>
    class AlphaBetaSearch : public SearchEngine
    {
    public:
        chess::Move Search(const std::stop_token& stop, const SearchParameters& parameters) override
        {
            ClearKillers();

            tt.NewGeneration();

            chess::Board board = parameters.board;

            if constexpr (IncrementalEvaluator<EvalT>)
                evaluator.Update(board);

            int64_t remaining = parameters.timeLimit;

            chess::Move bestMove{};
            uint32_t depth = 2;

            while (!stop.stop_requested())
            {
                std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

                ClearNodeCount();
                ClearMaxPly();

                const Centipawn score = Negamax(stop, board, depth, 0, 0, -CENTIPAWN_INFINITE, CENTIPAWN_INFINITE);

                if (!stop.stop_requested())
                    bestMove = pvTable.Get(0, 0);

                std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
                std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                if (duration.count() <= 0)
                    duration = std::chrono::milliseconds{ 1 };

                if (parameters.observer && !stop.stop_requested())
                {
                    std::unique_ptr<AlphaBetaSearchInfo> info = std::make_unique<AlphaBetaSearchInfo>();
                    info->hash = CTFNV1A("AlphaBetaSearchInfo");
                    info->elapsed = duration;
                    info->score = score;
                    info->depth = depth;
                    info->seldepth = GetMaxPly();
                    info->nodes = GetNodeCount();
                    info->nps = GetNodeCount() * 1000 / duration.count();
                    UpdatePrincipalVariation(board, info->pv);
                    PushSearchInfo(parameters.observer, std::move(info));
                }

                remaining -= duration.count();
                const int64_t estimate = static_cast<int64_t>(duration.count() * 2.5L);
                if (estimate > remaining && parameters.timeLimit > 0)
                    break;
                if ((depth >= parameters.depthLimit && parameters.depthLimit > 0) || depth >= MAX_DEPTH)
                    break;

                ++depth;
            }

            return bestMove;
        }

        void Clear() override
        {
            tt.Clear();
            pvTable.Clear();
            ClearKillers();
            ClearMaxPly();
            ClearNodeCount();
        }

    private:
        void UpdatePrincipalVariation(chess::Board board, std::vector<chess::Move>& pv)
        {
            pv.clear();
            for (uint32_t i = 0; i < pvTable.Size(0); ++i)
                pv.push_back(pvTable.Get(0, i));
        }

        Centipawn Negamax(
            const std::stop_token& stop,
            chess::Board& board,
            uint32_t limit,
            uint32_t ply,
            uint32_t extension,
            Centipawn alpha,
            Centipawn beta,
            bool isNull = false)
        {
            IncrementNodeCount();

            pvTable.Enter(ply);

            if (stop.stop_requested())
                return -CENTIPAWN_INFINITE;

            if (ply > 0 && board.isRepetition(1) || board.isHalfMoveDraw())
                return CENTIPAWN_DRAW;

            if (board.inCheck() && extension < MAX_EXTENSION)
            {
                limit += 1;
                extension += 1;
            }

            if (limit == 0)
                return Quiescence(stop, board, ply, alpha, beta);

            Centipawn originalAlpha = alpha;
            Centipawn originalBeta = beta;

            bool isPv = beta - alpha > 1;

            Centipawn staticEval = evaluator.Evaluate(board);

            if (!board.inCheck() && !isPv && limit <= 6 && std::abs(beta) < CENTIPAWN_MATE_IN_MAX_PLY)
            {
                Centipawn margin = static_cast<Centipawn>(150 * limit);
                if (staticEval >= beta + margin)
                    return staticEval;
            }

            if (!board.inCheck() && !isPv && limit >= 3 && staticEval >= beta
                && board.hasNonPawnMaterial(board.sideToMove()) && !isNull)
            {
                const uint32_t R = 2 + limit / 6;
                MakeNullMove(board);
                Centipawn score = -Negamax(stop, board, limit - 1 - R, ply + 1, extension, -beta, -beta + 1, true);
                UnmakeNullMove(board);

                if (score >= beta)
                    return std::abs(score) >= CENTIPAWN_MATE_IN_MAX_PLY ? beta : score;
            }

            chess::Move ttMove{};

            if (TranspositionTable::TTEntry* entry = tt.Probe(board.hash()))
            {
                ttMove = entry->bestMove;
                Centipawn entryScore = TranspositionTable::ScoreFromTT(entry->score, ply);
                if (!isPv && entry->depth >= limit)
                {
                    if (entry->flag == TranspositionTable::TTEntry::Flag::EXACT) return entryScore;
                    if (entry->flag == TranspositionTable::TTEntry::Flag::LOWERBOUND) alpha = std::max(alpha, entryScore);
                    if (entry->flag == TranspositionTable::TTEntry::Flag::UPPERBOUND) beta = std::min(beta, entryScore);
                    if (alpha >= beta) return entryScore;
                }
            }

            chess::Movelist legalMoves{};
            chess::movegen::legalmoves(legalMoves, board);

            if (legalMoves.empty() && board.inCheck())
                return -CENTIPAWN_MATE + static_cast<Centipawn>(ply);
            if (legalMoves.empty())
                return CENTIPAWN_DRAW;

            ScoreMoveList(board, legalMoves, ttMove, ply);

            chess::Move bestMove{};
            Centipawn bestScore = -CENTIPAWN_INFINITE;

            for (size_t i = 0; i < legalMoves.size(); ++i)
            {
                chess::Move& move = PickBest(legalMoves.begin() + i, legalMoves.end());

                MakeMove(board, move);

                int32_t score = 0;

                if (i > 0)
                {
                    uint32_t reducing = ReduceDepth(board, move, limit, ply, i);
                    score = -Negamax(stop, board, limit - 1 - reducing, ply + 1, extension, -alpha - 1, -alpha);
                }

                if (i == 0 || (score > alpha && score < beta))
                    score = -Negamax(stop, board, limit - 1, ply + 1, extension, -beta, -alpha);

                UnmakeMove(board, move);

                if (score > bestScore)
                {
                    bestMove = move;
                    bestScore = score;
                    if (isPv && score > alpha)
                    {
                        pvTable.Store(move, ply);
                        pvTable.AppendChild(ply);
                    }
                }
                alpha = std::max(alpha, bestScore);
                if (alpha >= beta)
                {
                    if (!board.isCapture(move))
                    {
                        UpdateHistory(board.sideToMove(), move.from(), move.to(), limit * limit);
                        if (move != killers[ply][0])
                        {
                            killers[ply][1] = killers[ply][0];
                            killers[ply][0] = move;
                        }
                    }
                    break;
                }
            }

            TranspositionTable::TTEntry::Flag flag =
                bestScore <= originalAlpha ? TranspositionTable::TTEntry::Flag::UPPERBOUND
              : bestScore >= originalBeta  ? TranspositionTable::TTEntry::Flag::LOWERBOUND
              : TranspositionTable::TTEntry::Flag::EXACT;
            tt.Store(board.hash(), bestMove, TranspositionTable::ScoreToTT(bestScore, ply), limit, flag);

            return bestScore;
        }

        Centipawn Quiescence(const std::stop_token& stop, chess::Board& board, uint32_t ply, Centipawn alpha, Centipawn beta)
        {
            IncrementNodeCount();
            MaxPly(ply);

            if (stop.stop_requested())
                return -CENTIPAWN_INFINITE;

            if (ply >= MAX_PLY)
                return Evaluate(board);

            Centipawn originalAlpha = alpha;
            Centipawn originalBeta = beta;

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
                bestScore = Evaluate(board);
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
                return board.inCheck() ? -CENTIPAWN_MATE + static_cast<Centipawn>(ply) : bestScore;

            ScoreMoveList(board, legalMoves, ttMove, ply);

            chess::Move bestMove{};

            for (size_t i = 0; i < legalMoves.size(); ++i)
            {
                chess::Move& move = PickBest(legalMoves.begin() + i, legalMoves.end());
                MakeMove(board, move);
                int32_t score = 0;
                if (i > 0)
                    score = -Quiescence(stop, board, ply + 1, -alpha - 1, -alpha);
                if (i == 0 || (score > alpha && score < beta))
                    score = -Quiescence(stop, board, ply + 1, -beta, -alpha);
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
                  : bestScore >= originalBeta  ? TranspositionTable::TTEntry::Flag::LOWERBOUND
                  : TranspositionTable::TTEntry::Flag::EXACT;
                tt.Store(board.hash(), bestMove, TranspositionTable::ScoreToTT(bestScore, ply), 0, flag);
            }

            return bestScore;
        }

        chess::Move& PickBest(chess::Move* begin, chess::Move* end)
        {
            chess::Move* best = begin;

            for (chess::Move* it = begin + 1; it != end; ++it)
            {
                if (it->score() > best->score())
                    best = it;
            }
            std::swap(*best, *begin);
            return *begin;
        }

        [[nodiscard]] uint32_t ReduceDepth(
            const chess::Board& board,
            const chess::Move& move,
            const uint32_t depth,
            const uint32_t ply,
            const size_t moveIndex) const
        {
            if (depth < 3 || moveIndex < 3)
                return 0;
            if (board.inCheck())
                return 0;
            if (move == killers[ply][0] || move == killers[ply][1])
                return 0;
            return static_cast<uint32_t>(0.77 + std::log(depth) * std::log(moveIndex) / 2.36);
        }

        void ScoreMoveList(const chess::Board& board, chess::Movelist& moves, const chess::Move& ttMove, const uint32_t ply) const
        {
            for (auto& move : moves)
            {
                if (move == ttMove) move.setScore(20000);
                else if (board.isCapture(move)) move.setScore(MvvLvaScore(board, move)); // Max is 8900, min is 100
                else if (move == killers[ply][0]) move.setScore(99);
                else if (move == killers[ply][1]) move.setScore(98);
                else move.setScore(history[static_cast<size_t>(board.sideToMove().internal())][move.from().index()][move.to().index()]);
            }
        }

        static Centipawn MvvLvaScore(const chess::Board& board, const chess::Move& move)
        {
            chess::PieceType victimType = board.at<chess::PieceType>(move.to());
            if (move.typeOf() == chess::Move::ENPASSANT)
                victimType = chess::PieceType::PAWN;
            const Centipawn victimScore = Heuristic::Material::PIECES_VALUE[victimType];
            const Centipawn attackerScore = Heuristic::Material::PIECES_VALUE[board.at<chess::PieceType>(move.from())];
            return victimScore * 10 - attackerScore;
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

        void MakeNullMove(chess::Board& board)
        {
            if constexpr (IncrementalEvaluator<EvalT>)
                evaluator.MakeNullMove(board);
            board.makeNullMove();
        }

        void UnmakeNullMove(chess::Board& board)
        {
            if constexpr (IncrementalEvaluator<EvalT>)
                evaluator.UnmakeNullMove(board);
            board.unmakeNullMove();
        }

        Centipawn Evaluate(chess::Board& board)
        {
            const Centipawn score = evaluator.Evaluate(board);
#ifndef NDEBUG
            if (constexpr IncrementalEvaluator<EvalT>)
            {
                evaluator.Update(board);
                assert(score == evaluator.Evaluate(board));
            }
#endif
            return score;
        }

        void ClearNodeCount() { nodeCount = 0; }
        void IncrementNodeCount() { ++nodeCount; }
        [[nodiscard]] size_t GetNodeCount() const { return nodeCount; }

        void ClearMaxPly() { maxPly = 0; }
        void MaxPly(uint32_t ply) { maxPly = std::max(maxPly, ply); }
        [[nodiscard]] uint32_t GetMaxPly() const { return maxPly; }

        void ClearKillers()
        {
            std::fill_n(&killers[0][0], MAX_PLY * 2, chess::Move{});
        }

        void UpdateHistory(const chess::Color color, const chess::Square from, const chess::Square to, Centipawn bonus)
        {
            bonus = std::clamp(bonus, MIN_HISTORY, MAX_HISTORY);
            history[static_cast<size_t>(color.internal())][from.index()][to.index()] +=
                bonus - history[static_cast<size_t>(color.internal())][from.index()][to.index()] * std::abs(bonus) / MAX_HISTORY;
        }

    private:
        EvalT evaluator;
        size_t nodeCount = 0;
        uint32_t maxPly = 0;

        TranspositionTable tt{};
        PVTable pvTable{};

        chess::Move killers[MAX_PLY][2]{};
        Centipawn history[2][static_cast<size_t>(chess::Square::NO_SQ)][static_cast<size_t>(chess::Square::NO_SQ)] = {};
    };

}
