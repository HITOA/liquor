#include <Interface.hpp>

#include <Engine.hpp>

#include <thread>

#include "AlphaBetaSearch.hpp"
#include "Version.hpp"


LiquorChess::Interface::Interface(std::basic_istream<char>* input, std::basic_ostream<char>* output)
    : input{ input }, output{ output }, inputEvents{}, outputEvents{}, engine{ nullptr }
{

}

void LiquorChess::Interface::Init()
{
    outputEvents.Push(AllocateEvent<IdentifyNameEvent>(LIQUOR_FULL_NAME));
    outputEvents.Push(AllocateEvent<IdentifyAuthorEvent>("HITO"));
    outputEvents.Push(AllocateEvent<InterfaceInitializedEvent>());
}

void LiquorChess::Interface::Run()
{
    running.store(true);
    std::jthread listeningThread{ [this] { ListenInputLoop(); } };

    while (running.load(std::memory_order_relaxed))
    {
        while (!inputEvents.Empty())
        {
            Event* event = inputEvents.Pop();
            HandleEvent(event);
            DeallocateEvent(event);
        }

        while (!outputEvents.Empty())
        {
            Event* event = outputEvents.Pop();
            std::string serializeEvent = SerializeEvent(event);
            DeallocateEvent(event);
            *output << serializeEvent << std::endl;
        }
    }

    listeningThread.join();
}

void LiquorChess::Interface::ListenInputLoop()
{
    bool shouldQuit = false;
    while (running.load(std::memory_order_relaxed) && !shouldQuit)
    {
        std::string event;
        std::getline(*input, event);
        if (Event* deserializeEvent = DeserializeEvent(event))
        {
            if (const auto* c = deserializeEvent->Is<CompoundEvent>())
            {
                for (Event* e : *c)
                {
                    inputEvents.Push(e);
                    if (e->Is<QuitEvent>())
                        shouldQuit = true;
                }
                DeallocateEvent(deserializeEvent);
            } else
            {
                inputEvents.Push(deserializeEvent);
                if (deserializeEvent->Is<QuitEvent>())
                    shouldQuit = true;
            }
        }
    }
}

void LiquorChess::Interface::HandleEvent(Event* event)
{
    assert(event != nullptr);

    if (event->Is<QuitEvent>())
    {
        running.store(false);
        return;
    }
    if (event->Is<IsReadyEvent>())
    {
        ReadyEngine();
        outputEvents.Push(AllocateEvent<EngineReadyEvent>());
        return;
    }
    if (const auto* e = event->Is<FENPositionEvent>())
    {
        assert(engine != nullptr);
        engine->SetBoardInternal(e->FEN());
        return;
    }
    if (const auto* e = event->Is<MovesEvent>())
    {
        assert(engine != nullptr);
        for (const std::string& move : e->Moves())
        {
            engine->MakeMove(move);
        }
        return;
    }
    if (event->Is<SearchEvent>())
    {
        assert(engine != nullptr);
        engine->Run();
        return;
    }
    if (event->Is<StopEvent>())
    {
        assert(engine != nullptr);
        engine->Stop();
        return;
    }
}

void LiquorChess::Interface::ReadyEngine()
{
    if (engine == nullptr)
    {
        engine = std::make_unique<Engine>(this, std::make_unique<AlphaBetaSearch<MinimalistHeuristic>>());
    }
}
