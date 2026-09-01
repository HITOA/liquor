#pragma once

#include <Chess.hpp>
#include <SearchEngine.hpp>

#include <thread>


namespace LiquorChess {
    class Interface;

    class Engine : public SearchObserver {
    private:
        constexpr static int32_t MAX_PLY = 32;
        constexpr static int32_t KILLER_COUNT = 3;

    public:
        explicit Engine(Interface* interface, std::unique_ptr<SearchEngine> searchEngine);
        ~Engine() override = default;

        void SetBoardInternal(const std::string& fen);
        void MakeMove(const std::string& move);

        void Run();
        void Stop();

        [[nodiscard]] inline bool IsRunning() const
        {
            return searching.load(std::memory_order_relaxed);
        }

    protected:
        void Search(std::stop_token stop);
        void OnSearchInfo(std::unique_ptr<SearchInfo> info) const override;

    private:
        Interface* interface;

        chess::Board board;

        std::jthread searchThread;
        std::atomic_bool searching;
        std::unique_ptr<SearchEngine> searchEngine;
    };

}