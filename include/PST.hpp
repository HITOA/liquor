//
// Created by HITO on 29/08/2026.
//

#pragma once

#include <Chess.hpp>
#include <cstdint>

#include "Types.hpp"


namespace LiquorChess::Heuristic::Presence
{

    constexpr Centipawn PAWN_PST[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-20, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    constexpr Centipawn KNIGHT_PST[64] = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    };

    constexpr Centipawn BISHOP_PST[64] = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    };

    constexpr Centipawn ROOK_PST[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    };

    constexpr Centipawn QUEEN_PST[64] = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
         -5,  0,  5,  5,  5,  5,  0, -5,
          0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };

    inline Centipawn Score(const chess::Board& board, chess::Color color)
    {
        Centipawn score = 0;

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

}
