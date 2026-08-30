cmake_minimum_required(VERSION 4.1)
cmake_policy(SET CMP0091 NEW)
project(LiquorChess VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(LiquorChess
        src/Main.cpp
        include/CommandBuffer.hpp
        include/Interface.hpp
        include/Engine.hpp
        include/Event.hpp
        src/Interface.cpp
        include/UCInterface.hpp
        src/UCInterface.cpp
        include/StringHash.hpp
        src/Engine.cpp
        include/PST.hpp
        include/PiecesValue.hpp
)

target_include_directories(LiquorChess PRIVATE "include")