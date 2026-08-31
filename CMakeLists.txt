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
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=x86-64-v3 -flto -DNDEBUG")

find_package(Git)
execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
        OUTPUT_VARIABLE LIQUOR_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

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
        include/Version.hpp
        include/SearchEngine.hpp
        include/Evaluator.hpp
        include/AlphaBetaSearch.hpp
        include/Types.hpp
        include/TranspositionTable.hpp
        src/TranspositionTable.cpp
)

target_include_directories(LiquorChess PRIVATE "include")

target_compile_definitions(LiquorChess PUBLIC LIQUOR_VERSION="${LIQUOR_VERSION}")
set_target_properties(LiquorChess PROPERTIES OUTPUT_NAME "Liquor-${LIQUOR_VERSION}")

target_compile_options(LiquorChess PRIVATE $<$<CONFIG:Release>:-O3 -march=x86-64-v3 -flto -DNDEBUG>)
target_link_options(LiquorChess PRIVATE $<$<CONFIG:Release>:-flto>)