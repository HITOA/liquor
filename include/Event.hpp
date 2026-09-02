//
// Created by HITO on 29/08/2026.
//

#pragma once

#include <string>
#include <typeinfo>

#include <Chess.hpp>

#include "Types.hpp"


namespace LiquorChess
{

    class Event
    {
    public:
        Event() = default;
        virtual ~Event() = default;

        [[nodiscard]] size_t Type() const { return type; }

        template<typename T>
        [[nodiscard]] T* Is()
        {
            if (Type() == Of<T>())
                return static_cast<T*>(this);
            return nullptr;
        }

        template<typename T>
        [[nodiscard]] const T* Is() const
        {
            if (Type() == Of<T>())
                return static_cast<T*>(this);
            return nullptr;
        }

        template<typename T>
        [[nodiscard]] static size_t Of()
        {
            return typeid(T).hash_code();
        }

    private:
        size_t type = 0;

        friend class Interface;
    };

    class CompoundEvent : public Event
    {
    public:
        CompoundEvent(std::initializer_list<Event*> events) : events{ new Event*[events.size()] }, count{ events.size() }
        {
            std::copy(events.begin(), events.end(), this->events);
        }
        ~CompoundEvent() override
        {
            delete[] this->events;
        }

        [[nodiscard]] Event** begin() const { return events; }
        [[nodiscard]] Event** end() const { return &events[count]; }

    private:
        Event** events;
        size_t count;
    };

    // GUI -> Engine Events

    /**
     * Signal the engine should quit
     */
    class QuitEvent : public Event
    {
    public:
        QuitEvent() = default;
        ~QuitEvent() override = default;
    };

    /**
     * Ask if the engine is ready
     */
    class IsReadyEvent : public Event
    {
    public:
        IsReadyEvent() = default;
        ~IsReadyEvent() override = default;
    };

    /**
     * Set the position of the internal board
     */
    class FENPositionEvent : public Event
    {
    public:
        FENPositionEvent(std::string fen) : fen{ std::move(fen) } {}
        ~FENPositionEvent() override = default;

        [[nodiscard]] const std::string& FEN() const { return fen; }

    private:
        std::string fen;
    };

    /**
     * Sequence of moves
     */
    class MovesEvent : public Event
    {
    public:
        MovesEvent(std::vector<std::string> moves) : moves{ std::move(moves) } {}
        ~MovesEvent() override = default;

        [[nodiscard]] const std::vector<std::string>& Moves() const { return moves; }

    private:
        std::vector<std::string> moves;
    };

    /**
     * Signal the engine to start searching for a best move
     */
    class SearchEvent : public Event
    {
    public:
        SearchEvent(uint32_t wtime, uint32_t btime, uint32_t winc, uint32_t binc, uint32_t depth, bool infinite)
            : wtime{ wtime }, btime{ btime }, winc{ winc }, binc{ binc }, depth{ depth }, infinite{ infinite } {}
        ~SearchEvent() override = default;

        [[nodiscard]] uint32_t WhiteTime() const { return wtime; }
        [[nodiscard]] uint32_t BlackTime() const { return btime; }
        [[nodiscard]] uint32_t WhiteIncrement() const { return winc; }
        [[nodiscard]] uint32_t BlackIncrement() const { return binc; }
        [[nodiscard]] uint32_t Depth() const { return depth; }
        [[nodiscard]] bool Infinite() const { return infinite; }

    private:
        uint32_t wtime = 0;
        uint32_t btime = 0;
        uint32_t winc = 0;
        uint32_t binc = 0;
        uint32_t depth = 0;
        bool infinite = false;
    };

    /**
     * Signal the engine to stop searching and return its best move
     */
    class StopEvent : public Event
    {
    public:
        StopEvent() = default;
        ~StopEvent() override = default;
    };

    /**
     * Signal the engine a new game is starting and should update its state
     */
    class NewGameEvent : public Event
    {
    public:
        NewGameEvent() = default;
        ~NewGameEvent() override = default;
    };

    // Engine -> GUI Events

    /**
     * Identify the engine's name to the GUI
     */
    class IdentifyNameEvent : public Event
    {
    public:
        IdentifyNameEvent(std::string name) : name{ std::move(name) } {}
        ~IdentifyNameEvent() override = default;

        [[nodiscard]] const std::string& Name() const { return name; }

    private:
        std::string name;
    };

    /**
     * Identify the engine's author to the GUI
     */
    class IdentifyAuthorEvent : public Event
    {
    public:
        IdentifyAuthorEvent(std::string author) : author{ std::move(author) } {}
        ~IdentifyAuthorEvent() override = default;

        [[nodiscard]] const std::string& Author() const { return author; }

    private:
        std::string author;
    };


    /**
     * Signal the interface is initialized and ready to continue
     */
    class InterfaceInitializedEvent : public Event
    {
    public:
        InterfaceInitializedEvent() = default;
        ~InterfaceInitializedEvent() override = default;
    };

    /**
     * Signal the engine is ready to continue
     */
    class EngineReadyEvent : public Event
    {
    public:
        EngineReadyEvent() = default;
        ~EngineReadyEvent() override = default;
    };

    /**
     * Return the best move found by the engine
     */
    class BestMoveEvent : public Event
    {
    public:
        BestMoveEvent(std::string move) : move{ std::move(move) } {}
        ~BestMoveEvent() override = default;

        [[nodiscard]] const std::string& Move() const { return move; }

    private:
        std::string move;
    };

    /**
     * Info sent from the engine to the gui
     */
    class InfoEvent : public Event
    {
    public:
        InfoEvent(uint32_t depth, uint32_t seldepth, uint32_t time, uint32_t nodes, Centipawn score) :
            depth{ depth }, seldepth{ seldepth }, time{ time },  nodes{ nodes }, score{ score }, pv{} {}
        InfoEvent(uint32_t depth, uint32_t seldepth, uint32_t time, uint32_t nodes, Centipawn score, std::vector<chess::Move> pv) :
            depth{ depth }, seldepth{ seldepth }, time{ time },  nodes{ nodes }, score{ score }, pv{ std::move(pv) } {}
        ~InfoEvent() override = default;

        [[nodiscard]] uint32_t Depth() const { return depth; }
        [[nodiscard]] uint32_t SelDepth() const { return seldepth; }
        [[nodiscard]] uint32_t Time() const { return time; }
        [[nodiscard]] uint32_t Nodes() const { return nodes; }
        [[nodiscard]] Centipawn Score() const { return score; }
        [[nodiscard]] const std::vector<chess::Move>& Pv() const { return pv; }

    private:
        uint32_t depth;
        uint32_t seldepth;
        uint32_t time;
        uint32_t nodes;
        Centipawn score;
        std::vector<chess::Move> pv;
    };

}
