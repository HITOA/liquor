#include <Engine.hpp>

#include <Interface.hpp>

#include <chrono>

#include "AlphaBetaSearch.hpp"


LiquorChess::Engine::Engine(Interface* interface, std::unique_ptr<SearchEngine> searchEngine)
    : interface{ interface }, searchThread{}, searching{ false }, searchEngine{ std::move(searchEngine) }
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

void LiquorChess::Engine::SetTimeLimit(uint32_t wtime, uint32_t btime, uint32_t winc, uint32_t binc)
{
    if (board.sideToMove() == chess::Color::WHITE)
    {
        timeLimit = wtime / 20 + winc / 2;
    } else
    {
        timeLimit = btime / 20 + binc / 2;
    }
    depthLimit = 0;
    searchMode = SearchMode::TIME;
}

void LiquorChess::Engine::SetDepthLimit(uint32_t depth)
{
    depthLimit = depth;
    timeLimit = 0;
    searchMode = SearchMode::DEPTH;
}

void LiquorChess::Engine::SetMoveTime(uint32_t movetime)
{
    depthLimit = 0;
    timeLimit = movetime;
    searchMode = SearchMode::TIME;
}

void LiquorChess::Engine::SetNoLimit()
{
    depthLimit = 0;
    timeLimit = 0;
    searchMode = SearchMode::INFINITE;
}

void LiquorChess::Engine::Update()
{
    if (!IsRunning())
        return;

    switch (searchMode)
    {
    case SearchMode::INFINITE:
        break;
    case SearchMode::DEPTH:
        if (lastReachedDepth >= depthLimit)
                searchThread.request_stop();
        break;
    case SearchMode::TIME:
        {
            std::chrono::system_clock::time_point current = std::chrono::system_clock::now();
            std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - searchBeganAt);
            if (duration.count() >= timeLimit)
                searchThread.request_stop();
            break;
        }
    }
}

void LiquorChess::Engine::Run()
{
    assert(IsRunning() == false);

    searchBeganAt = std::chrono::system_clock::now();

    searching.store(true);
    searchThread = std::jthread{ [this](std::stop_token stop){ Search(stop); } };
}

void LiquorChess::Engine::Stop()
{
    if (!searchThread.joinable()) return;
    searchThread.request_stop();
    searchThread.join();
}

void LiquorChess::Engine::Clear()
{
    assert(IsRunning() == false);

    searchEngine->Clear();
}

void LiquorChess::Engine::Search(const std::stop_token& stop)
{
    const SearchParameters parameters{
        board,
        searchMode == SearchMode::NONE ? 5000 : timeLimit,
        depthLimit,
        this
    };
    const chess::Move bestMove = searchEngine->Search(stop, parameters);

    std::string move = chess::uci::moveToUci(bestMove);
    interface->PushToGUI<BestMoveEvent>(move);
    searching.store(false);
}

void LiquorChess::Engine::OnSearchInfo(std::unique_ptr<SearchInfo> info)
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
                    si->score,
                    si->pv);
                break;
            }
        default:
            interface->PushToGUI<InfoEvent>(info->depth, 0, info->elapsed.count(), 0, info->score);
            break;
    }

    lastReachedDepth = info->depth;
}