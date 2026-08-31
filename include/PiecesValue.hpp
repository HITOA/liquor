//
// Created by HITO on 30/08/2026.
//

#pragma once

#include "Types.hpp"


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


    inline Centipawn Score(const chess::Board& board, chess::Color color)
    {
        Centipawn score = 0;
        score += board.pieces(chess::PieceType::PAWN, color).count() * PIECES_VALUE[static_cast<int>(chess::PieceType::PAWN)];
        score += board.pieces(chess::PieceType::KNIGHT, color).count() * PIECES_VALUE[static_cast<int>(chess::PieceType::KNIGHT)];
        score += board.pieces(chess::PieceType::BISHOP, color).count() * PIECES_VALUE[static_cast<int>(chess::PieceType::BISHOP)];
        score += board.pieces(chess::PieceType::ROOK, color).count() * PIECES_VALUE[static_cast<int>(chess::PieceType::ROOK)];
        score += board.pieces(chess::PieceType::QUEEN, color).count() * PIECES_VALUE[static_cast<int>(chess::PieceType::QUEEN)];
        return score;
    }

}
