#pragma once

#include <Chess.hpp>
#include <SearchEngine.hpp>

#include <thread>


namespace LiquorChess {
    class Interface;

    class Engine : public SearchObserver {
    public:
        enum class SearchMode
        {
            INFINITE,
            DEPTH,
            TIME
        };

    public:
        explicit Engine(Interface* interface, std::unique_ptr<SearchEngine> searchEngine);
        ~Engine() override = default;

        void SetBoardInternal(const std::string& fen);
        void MakeMove(const std::string& move);

        void SetTimeLimit(uint32_t wtime, uint32_t btime, uint32_t winc, uint32_t binc);
        void SetDepthLimit(uint32_t depth);
        void SetNoLimit();

        void Update();
        void Run();
        void Stop();
        void Clear();

        [[nodiscard]] inline bool IsRunning() const
        {
            return searching.load(std::memory_order_relaxed);
        }

    protected:
        void Search(const std::stop_token& stop);
        void OnSearchInfo(std::unique_ptr<SearchInfo> info) override;

    private:
        Interface* interface = nullptr;

        chess::Board board{};

        SearchMode searchMode = SearchMode::INFINITE;
        uint32_t timeLimit = 0;
        uint32_t depthLimit = 0;

        uint32_t lastReachedDepth{};
        std::chrono::system_clock::time_point searchBeganAt{};

        std::jthread searchThread{};
        std::atomic_bool searching{ false };
        std::unique_ptr<SearchEngine> searchEngine = nullptr;
    };

}