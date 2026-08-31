//
// Created by HITO on 30/08/2026.
//

#pragma once

#include <Chess.hpp>
#include <Types.hpp>


namespace LiquorChess::Heuristic::Material
{

    constexpr Centipawn PIECES_VALUE[6] = {
        IDEAL_PAWN,
        IDEAL_KNIGHT,
        IDEAL_BISHOP,
        IDEAL_ROOK,
        IDEAL_QUEEN,
        IDEAL_KING
    };

    constexpr Centipawn BISHOP_PAIR_VALUE = 40;
    constexpr Centipawn REDUNDANCY_VALUE = -20;


    inline Centipawn Score(const chess::Board& board, chess::Color color)
    {
        Centipawn score = 0;

        const chess::Bitboard pawnBitboard = board.pieces(chess::PieceType::PAWN, color);
        const chess::Bitboard knightBitboard = board.pieces(chess::PieceType::KNIGHT, color);
        const chess::Bitboard bishopBitboard = board.pieces(chess::PieceType::BISHOP, color);
        const chess::Bitboard rookBitboard = board.pieces(chess::PieceType::ROOK, color);
        const chess::Bitboard queenBitboard = board.pieces(chess::PieceType::QUEEN, color);

        const int pawnCount = pawnBitboard.count();
        const int knightCount = knightBitboard.count();
        const int bishopCount = bishopBitboard.count();
        const int rookCount = rookBitboard.count();
        const int queenCount = queenBitboard.count();

        score += pawnCount * PIECES_VALUE[static_cast<int>(chess::PieceType::PAWN)];
        score += knightCount * PIECES_VALUE[static_cast<int>(chess::PieceType::KNIGHT)];
        score += bishopCount * PIECES_VALUE[static_cast<int>(chess::PieceType::BISHOP)];
        score += rookCount * PIECES_VALUE[static_cast<int>(chess::PieceType::ROOK)];
        score += queenCount * PIECES_VALUE[static_cast<int>(chess::PieceType::QUEEN)];

        score += bishopCount >= 2 ? BISHOP_PAIR_VALUE : 0;
        score += REDUNDANCY_VALUE * std::max(0, knightCount - 1);
        score += REDUNDANCY_VALUE * std::max(0, rookCount - 1);

        return score;
    }

}
