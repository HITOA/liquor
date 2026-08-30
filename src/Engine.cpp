#include <Engine.hpp>

#include <Interface.hpp>
#include <PST.hpp>
#include <PiecesValue.hpp>

#include <chrono>


LiquorChess::Engine::Engine(Interface* interface) : interface{ interface }, killers{}
{
    ClearKillers();
}

void LiquorChess::Engine::SetBoardInternal(const std::string& fen)
{
    assert(searching.load() == false);

    board.setFen(fen);
}

void LiquorChess::Engine::MakeMove(const std::string& move)
{
    assert(searching.load() == false);

    board.makeMove(chess::uci::uciToMove(board, move));
}

void LiquorChess::Engine::Run()
{
    assert(searching.load() == false);

    searching.store(true);
    searchThread = std::jthread{ [this]{ Search(); } };
}

void LiquorChess::Engine::Stop()
{
    assert(searching.load() == true);

    searching.store(false);
    searchThread.join();
}

void LiquorChess::Engine::Search()
{
    constexpr uint32_t bf = 6; //branching factor

    ClearKillers();

    int16_t depth = 1;

    chess::Board copiedBoard = board;

    chess::Movelist moves;
    chess::movegen::legalmoves(moves, copiedBoard);

    chess::Move bestMove{};

    while (searching.load(std::memory_order_relaxed) == true)
    {
        std::chrono::system_clock::time_point beg = std::chrono::high_resolution_clock::now();

        uint32_t nodes = 0;

        chess::Move currentBestMove{};
        currentBestMove.setScore(-20000);

        int32_t v = -20000;
        int32_t a = -20000;
        int32_t b = 20000;

        for (auto& move : moves)
        {
            copiedBoard.makeMove(move);
            int32_t s = -Negamax(copiedBoard, depth - 1, -b, -a, nodes);
            v = std::max(v, s);
            move.setScore(static_cast<int16_t>(s));
            copiedBoard.unmakeMove(move);
            if (currentBestMove.score() < move.score())
                currentBestMove = move;
            a = std::max(v, a);
            if (a >= b)
                break;
        }

        std::sort(moves.begin(), moves.end(), [](chess::Move& v1, chess::Move& v2) {
            return v1.score() > v2.score();
        });

        if (searching.load(std::memory_order_relaxed) != false)
            bestMove = currentBestMove;

        std::chrono::system_clock::time_point end = std::chrono::high_resolution_clock::now();

        std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - beg);

        if (duration.count() * bf > 10000)
            searching.store(false);

        interface->PushToGUI<InfoEvent>(depth, depth, duration.count(), nodes, bestMove.score());
        ++depth;
    }

    std::string move = chess::uci::moveToUci(bestMove);
    interface->PushToGUI<BestMoveEvent>(move);

    searching.store(false);
}

int32_t LiquorChess::Engine::Negamax(chess::Board& board, int32_t depth, int32_t a, int32_t b, uint32_t& nodes, uint32_t ply)
{
    constexpr int32_t MATE_SCORE = 10000;

    ++nodes;

    if (board.isRepetition() || board.isHalfMoveDraw())
        return 0;

    if (depth <= 0 || searching.load() == false)
        return Quiescence(board, a, b, nodes);

    int32_t bestScore = -20000;

    chess::Movelist moves{};
    chess::movegen::legalmoves(moves, board);

    if (moves.empty())
    {
        if (board.inCheck())
            return -MATE_SCORE + ply;
        return 0;
    }

    for (chess::Move& move : moves)
    {
        if (board.isCapture(move))
            move.setScore(static_cast<int16_t>(MvvLvaScore(board, move)));
        else if (int16_t score = IsKiller(move, ply))
            move.setScore(score);
    }
    std::sort(moves.begin(), moves.end(), [](chess::Move& v1, chess::Move& v2)
    {
        return v1.score() > v2.score();
    });

    for (chess::Move& move : moves)
    {
        board.makeMove(move);
        int32_t score = -Negamax(board, depth - 1, -b, -a, nodes, ply + 1);
        board.unmakeMove(move);
        bestScore = std::max(score, bestScore);
        a = std::max(bestScore, a);
        if (a >= b)
        {
            if (!board.isCapture(move))
            {
                move.setScore(static_cast<int16_t>(score));
                TryInsertKiller(move, ply);
            }
            break;
        }
    }

    return bestScore;
}

int32_t LiquorChess::Engine::Quiescence(chess::Board& board, int32_t a, int32_t b, uint32_t& nodes)
{
    ++nodes;

    int32_t bestScore = Evaluate(board);
    if (bestScore >= b)
        return bestScore;
    if (bestScore > a)
        a = bestScore;

    chess::Movelist moves{};
    chess::movegen::legalmoves(moves, board);

    for (chess::Move& move : moves)
        if (board.isCapture(move))
            move.setScore(static_cast<int16_t>(MvvLvaScore(board, move)));
    std::sort(moves.begin(), moves.end(), [](chess::Move& v1, chess::Move& v2)
    {
        return v1.score() > v2.score();
    });

    for (chess::Move& move : moves)
    {
        if (!board.isCapture(move))
            continue;
        board.makeMove(move);
        int32_t score = -Quiescence(board, -b, -a, nodes);
        board.unmakeMove(move);

        if (score >= b)
            return score;
        if (score > bestScore)
            bestScore = score;
        if (score > a)
            a = score;
    }

    return bestScore;
}

int32_t LiquorChess::Engine::Evaluate(chess::Board& board)
{
    chess::Color us = board.sideToMove();
    chess::Color them = ~us;

    int32_t score = (MaterialScore(board, us) + PresenceScore(board, us)) -
                    (MaterialScore(board, them) + PresenceScore(board, them));

    return score;
}

int32_t LiquorChess::Engine::MaterialScore(chess::Board& board, chess::Color color)
{
    int32_t score = 0;
    score += board.pieces(chess::PieceType::PAWN, color).count() * PIECES_VALUE[static_cast<int>(chess::PieceType::PAWN)];
    score += board.pieces(chess::PieceType::KNIGHT, color).count() * PIECES_VALUE[static_cast<int>(chess::PieceType::KNIGHT)];
    score += board.pieces(chess::PieceType::BISHOP, color).count() * PIECES_VALUE[static_cast<int>(chess::PieceType::BISHOP)];
    score += board.pieces(chess::PieceType::ROOK, color).count() * PIECES_VALUE[static_cast<int>(chess::PieceType::ROOK)];
    score += board.pieces(chess::PieceType::QUEEN, color).count() * PIECES_VALUE[static_cast<int>(chess::PieceType::QUEEN)];
    return score;
}

int32_t LiquorChess::Engine::PresenceScore(chess::Board& board, chess::Color color)
{
    int32_t score = 0;

    chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN, color);
    chess::Bitboard knights = board.pieces(chess::PieceType::KNIGHT, color);
    chess::Bitboard bishops = board.pieces(chess::PieceType::BISHOP, color);
    chess::Bitboard rooks = board.pieces(chess::PieceType::ROOK, color);
    chess::Bitboard queens = board.pieces(chess::PieceType::QUEEN, color);

    while (pawns)
    {
        chess::Square square{ pawns.pop() };
        score += PAWN_PST[square.relative_square(color).index()];
    }

    while (knights)
    {
        chess::Square square{ knights.pop() };
        score += KNIGHT_PST[square.relative_square(color).index()];
    }

    while (bishops)
    {
        chess::Square square{ bishops.pop() };
        score += BISHOP_PST[square.relative_square(color).index()];
    }

    while (rooks)
    {
        chess::Square square{ rooks.pop() };
        score += ROOK_PST[square.relative_square(color).index()];
    }

    while (queens)
    {
        chess::Square square{ queens.pop() };
        score += QUEEN_PST[square.relative_square(color).index()];
    }

    return score;
}

int32_t LiquorChess::Engine::MvvLvaScore(chess::Board& board, chess::Move& move)
{
    int32_t victimScore = PIECES_VALUE[board.at<chess::PieceType>(move.to())];
    int32_t attackerScore = PIECES_VALUE[board.at<chess::PieceType>(move.from())];
    return victimScore * 100 - attackerScore;
}

int16_t LiquorChess::Engine::IsKiller(chess::Move& move, uint32_t ply)
{
    for (auto& killer : killers[ply])
    {
        if (killer == move)
            return killer.score();
    }
    return 0;
}

void LiquorChess::Engine::TryInsertKiller(chess::Move& move, uint32_t ply)
{
    for (auto& killer : killers[ply])
    {
        if (killer.score() < move.score())
        {
            std::swap(killer, move);
        }
    }
}

void LiquorChess::Engine::ClearKillers()
{
    for (auto& killer : killers)
    {
        for (auto& i : killer)
        {
            i = chess::Move{};
        }
    }
}