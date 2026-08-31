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
    };

    class SearchObserver
    {
    public:
        SearchObserver() = default;
        virtual ~SearchObserver() = default;

    protected:
        virtual void OnSearchInfo(std::unique_ptr<SearchInfo> info) const = 0;

    private:
        friend class SearchEngine;
    };

    struct SearchParameters
    {
        const chess::Board& board;
        std::chrono::milliseconds limit;
        const SearchObserver* observer;

        SearchParameters(const chess::Board& board, std::chrono::milliseconds limit, const SearchObserver* observer) :
            board{ board }, limit{ limit }, observer{ observer } {}
    };

    class SearchEngine
    {
    public:
        SearchEngine() = default;
        virtual ~SearchEngine() = default;

        virtual chess::Move Search(std::stop_token stop, const SearchParameters& parameters) = 0;

    protected:
        void PushSearchInfo(const SearchObserver* observer, std::unique_ptr<SearchInfo> info)
        {
            observer->OnSearchInfo(std::move(info));
        }
    };
}
