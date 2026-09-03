//
// Created by HITO on 30/08/2026.
//

#pragma once
#include <stop_token>

#include <Chess.hpp>
#include <Evaluator.hpp>


namespace LiquorChess
{
    struct SearchInfo
    {
        uint32_t hash;
        std::chrono::milliseconds elapsed;
        Centipawn score;
        uint32_t depth;
    };

    class SearchObserver
    {
    public:
        SearchObserver() = default;
        virtual ~SearchObserver() = default;

    protected:
        virtual void OnSearchInfo(std::unique_ptr<SearchInfo> info) = 0;

    private:
        friend class SearchEngine;
    };

    struct SearchParameters
    {
        chess::Board board;
        uint32_t timeLimit;
        uint32_t depthLimit;
        SearchObserver* observer;

        SearchParameters(chess::Board board, uint32_t timeLimit, uint32_t depthLimit, SearchObserver* observer) :
            board{ board }, timeLimit{ timeLimit }, depthLimit{ depthLimit }, observer{ observer } {}
    };

    class SearchEngine
    {
    public:
        SearchEngine() = default;
        virtual ~SearchEngine() = default;

        virtual chess::Move Search(const std::stop_token& stop, const SearchParameters& parameters) = 0;
        virtual void Clear() = 0;

    protected:
        static void PushSearchInfo(SearchObserver* observer, std::unique_ptr<SearchInfo> info)
        {
            observer->OnSearchInfo(std::move(info));
        }
    };
}
