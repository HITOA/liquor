#include <sstream>
#include <UCInterface.hpp>

#include <StringHash.hpp>


LiquorChess::UCInterface::UCInterface(std::basic_istream<char>* input, std::basic_ostream<char>* output)
    : Interface{ input, output }
{

}

std::string LiquorChess::UCInterface::SerializeEvent(Event* event)
{
    if (const auto* e = event->Is<IdentifyNameEvent>())
        return std::string{ "id name " } + e->Name();
    if (const auto* e = event->Is<IdentifyAuthorEvent>())
        return std::string{ "id author " } + e->Author();
    if (event->Is<InterfaceInitializedEvent>())
        return std::string{ "uciok" };
    if (event->Is<EngineReadyEvent>())
        return std::string{ "readyok" };
    if (const auto* e = event->Is<BestMoveEvent>())
        return std::string{ "bestmove " } + e->Move();
    if (const auto* e = event->Is<InfoEvent>())
    {
        std::string serializeEvent{ "info" };
        serializeEvent += " depth " + std::to_string(e->Depth());
        serializeEvent += " seldepth " + std::to_string(e->SelDepth());
        serializeEvent += " time " + std::to_string(e->Time());
        serializeEvent += " nodes " + std::to_string(e->Nodes());
        serializeEvent += " score cp " + std::to_string(e->Score());
        return serializeEvent;
    }
    return std::string{};
}

LiquorChess::Event* LiquorChess::UCInterface::DeserializeEvent(const std::string& event)
{
    std::string head = event;
    if (const auto p = event.find(' '); p != std::string::npos)
        head = event.substr(0, event.find(' '));

    switch (fnv1a(head.c_str()))
    {
        case CTFNV1A("quit"):
            return AllocateEvent<QuitEvent>();
        case CTFNV1A("isready"):
            return AllocateEvent<IsReadyEvent>();
        case CTFNV1A("position"):
            return DeserializePositionEvent(event);
        case CTFNV1A("go"):
            return AllocateEvent<SearchEvent>();
        case CTFNV1A("stop"):
            return AllocateEvent<StopEvent>();
        default:
            return nullptr;
    }
}

LiquorChess::Event* LiquorChess::UCInterface::DeserializePositionEvent(const std::string& event)
{
    std::istringstream iss(event);
    std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss},
                                      std::istream_iterator<std::string>{} };

    std::string fen;
    size_t i = 0;

    if (tokens.empty())
        return nullptr;

    if (tokens[i++] != "position" || tokens.size() < 2)
        return nullptr;

    if (tokens[i] == "fen")
    {
        ++i;
        if (tokens.size() < i + 6)
            return nullptr;
        fen = tokens[i] + " " + tokens[i + 1] + " " + tokens[i + 2] + " "
            + tokens[i + 3] + " " + tokens[i + 4] + " " + tokens[i + 5];
        i += 6;
    }
    else
    {
        fen = chess::constants::STARTPOS;
        ++i;
    }

    FENPositionEvent* positionEvent = AllocateEvent<FENPositionEvent>(fen);

    if (i >= tokens.size() || tokens[i] != "moves")
        return positionEvent;

    std::vector<std::string> moves(tokens.begin() + i + 1, tokens.end());

    MovesEvent* movesEvent = AllocateEvent<MovesEvent>(moves);

    return AllocateCompoundEvent({ positionEvent, movesEvent });
}