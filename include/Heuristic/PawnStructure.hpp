//
// Created by HITO on 31/08/2026.
//

#pragma once
#include "Chess.hpp"
#include "Types.hpp"


namespace LiquorChess::Heuristic::PawnStructure
{

    constexpr Centipawn CONNECTED_PAWN_VALUE = 7;
    constexpr Centipawn ISOLATED_PAWN_VALUE = -10;

    inline Centipawn Score(const chess::Board& board, chess::Color color)
    {
        Centipawn score = 0;

        chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN, color);

        chess::Bitboard defendedFromWest = 0;
        chess::Bitboard defendedFromEast = 0;
        chess::Bitboard defender = 0;

        if (color == chess::Color::WHITE)
        {
            defendedFromWest = pawns & chess::attacks::pawnRightAttacks<chess::Color::WHITE>(pawns);
            defendedFromEast = pawns & chess::attacks::pawnLeftAttacks<chess::Color::WHITE>(pawns);
            defender = chess::attacks::shift<chess::Direction::SOUTH_WEST>(defendedFromWest) |
                        chess::attacks::shift<chess::Direction::SOUTH_EAST>(defendedFromEast);
        } else
        {
            defendedFromWest = pawns & chess::attacks::pawnRightAttacks<chess::Color::BLACK>(pawns);
            defendedFromEast = pawns & chess::attacks::pawnLeftAttacks<chess::Color::BLACK>(pawns);
            defender = chess::attacks::shift<chess::Direction::NORTH_WEST>(defendedFromWest) |
                        chess::attacks::shift<chess::Direction::NORTH_EAST>(defendedFromEast);
        }

        score += (defendedFromWest | defendedFromEast | defender).count() * CONNECTED_PAWN_VALUE;

        return score;
    }

}
