#include <PVTable.hpp>


void LiquorChess::PVTable::Enter(uint32_t ply)
{
    pv[ply][ply].setScore(static_cast<int16_t>(ply));
}

void LiquorChess::PVTable::Store(chess::Move move, uint32_t ply)
{
    move.setScore(static_cast<int16_t>(ply + 1));
    pv[ply][ply] = move;
}

void LiquorChess::PVTable::AppendChild(uint32_t ply)
{
    const int16_t childLen = pv[ply + 1][ply + 1].score();
    if (childLen <= static_cast<int16_t>(ply + 1))
        return;
    for (uint32_t i = ply + 1; i < static_cast<uint32_t>(childLen); ++i)
        pv[ply][i] = pv[ply + 1][i];
    pv[ply][ply].setScore(childLen);
}

void LiquorChess::PVTable::Clear()
{
    memset(pv, 0x0, sizeof(pv));
}

chess::Move LiquorChess::PVTable::Get(uint32_t ply, uint32_t idx) const
{
    return pv[ply][idx];
}

uint32_t LiquorChess::PVTable::Size(uint32_t ply) const
{
    return pv[ply][0].score();
}