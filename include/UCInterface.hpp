//
// Created by HITO on 29/08/2026.
//

#pragma once

#include <Interface.hpp>


namespace LiquorChess
{

    class UCInterface : public Interface
    {
    public:
        UCInterface(std::basic_istream<char>* input, std::basic_ostream<char>* output);

    protected:
        std::string SerializeEvent(Event* event) override;
        Event* DeserializeEvent(const std::string& event) override;

        Event* DeserializePositionEvent(const std::string& event);
    };

}
