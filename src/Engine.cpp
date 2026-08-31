#include <Engine.hpp>

#include <Interface.hpp>
#include <PST.hpp>
#include <PiecesValue.hpp>

#include <chrono>

#include "AlphaBetaSearch.hpp"


LiquorChess::Engine::Engine(Interface* interface, std::unique_ptr<SearchEngine> searchEngine)
    : interface{ interface }, searchThread{}, searchEngine{ std::move(searchEngine) }
{
}

void LiquorChess::Engine::SetBoardInternal(const std::string& fen)
{
    assert(IsRunning() == false);

    board.setFen(fen);
}

void LiquorChess::Engine::MakeMove(const std::string& move)
{
    assert(IsRunning() == false);

    board.makeMove(chess::uci::uciToMove(board, move));
}

void LiquorChess::Engine::Run()
{
    assert(IsRunning() == false);

    searchThread = std::jthread{ [this](std::stop_token stop){ Search(stop); } };
}

void LiquorChess::Engine::Stop()
{
    assert(IsRunning() == true);

    searchThread.request_stop();
    searchThread.join();
}

void LiquorChess::Engine::Search(std::stop_token stop) const
{
    SearchParameters parameters{
        board,
        std::chrono::milliseconds{ 5000 },
        this
    };
    chess::Move bestMove = searchEngine->Search(stop, parameters);

    std::string move = chess::uci::moveToUci(bestMove);
    interface->PushToGUI<BestMoveEvent>(move);
}

void LiquorChess::Engine::OnSearchInfo(std::unique_ptr<SearchInfo> info) const
{
    switch (info->hash)
    {
        case CTFNV1A("AlphaBetaSearchInfo"):
            {
                AlphaBetaSearchInfo* si = static_cast<AlphaBetaSearchInfo*>(info.get());
                interface->PushToGUI<InfoEvent>(
                    si->depth,
                    si->seldepth,
                    si->elapsed.count(),
                    si->nodes,
                    si->score);
                break;
            }
        default:
            interface->PushToGUI<InfoEvent>(0, 0, info->elapsed.count(), 0, info->score);
            break;
    }
}