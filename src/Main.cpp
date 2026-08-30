#include <Interface.hpp>
#include <UCInterface.hpp>
#include <StringHash.hpp>

#include <iostream>

int main(int arc, char* argv[]) {
    LiquorChess::Interface* interface = nullptr;

    std::string initLine;
    std::getline(std::cin, initLine);

    switch (LiquorChess::fnv1a(initLine.c_str()))
    {
        case CTFNV1A("uci"):
            interface = new LiquorChess::UCInterface{ &std::cin, &std::cout };
            break;
        default:
            std::cout << "Unknown interface." << std::endl;
            return -1;
    }

    interface->Init();
    interface->Run();

    return 0;
}