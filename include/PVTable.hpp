//
// Created by HITO on 02/09/2026.
//

#pragma once
#include "Chess.hpp"
#include "Types.hpp"


namespace LiquorChess
{

    class PVTable
    {
    public:
        PVTable() = default;
        ~PVTable() = default;

        void Enter(uint32_t ply);
        void Store(chess::Move move, uint32_t ply);
        void AppendChild(uint32_t ply);
        void Clear();

        [[nodiscard]] chess::Move Get(uint32_t ply, uint32_t idx) const;
        [[nodiscard]] uint32_t Size(uint32_t ply) const;

    private:
        // Store the pv length in the first move's score
        chess::Move pv[MAX_PLY][MAX_PLY]{};
    };

}
