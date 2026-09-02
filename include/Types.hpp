//
// Created by HITO on 30/08/2026.
//

#pragma once

#include <cstdint>


namespace LiquorChess
{

    constexpr int32_t MAX_MOVES = 256;
    constexpr int32_t MAX_PLY = 246;

    using Centipawn = int32_t;

    constexpr Centipawn CENTIPAWN_ZERO = 0;
    constexpr Centipawn CENTIPAWN_DRAW = 0;
    constexpr Centipawn CENTIPAWN_NONE = 32002;
    constexpr Centipawn CENTIPAWN_INFINITE = 32001;

    constexpr Centipawn CENTIPAWN_MATE = 32000;
    constexpr Centipawn CENTIPAWN_MATE_IN_MAX_PLY = CENTIPAWN_MATE - MAX_PLY;
    constexpr Centipawn CENTIPAWN_MATED_IN_MAX_PLY = -CENTIPAWN_MATE_IN_MAX_PLY;

    constexpr bool IsValid(const Centipawn v) { return v != CENTIPAWN_NONE; }

    constexpr Centipawn IDEAL_PAWN      = 100;
    constexpr Centipawn IDEAL_KNIGHT    = 320;
    constexpr Centipawn IDEAL_BISHOP    = 330;
    constexpr Centipawn IDEAL_ROOK      = 500;
    constexpr Centipawn IDEAL_QUEEN     = 900;
    constexpr Centipawn IDEAL_KING      = 5000;

    constexpr Centipawn MAX_HISTORY = 97;
    constexpr Centipawn MIN_HISTORY = -MAX_HISTORY;

    constexpr size_t MAX_PV_LENGTH = 64;
    constexpr size_t MAX_EXTENSION = 16;
    constexpr size_t MAX_DEPTH     = 128;

}
