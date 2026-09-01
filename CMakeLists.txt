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

set(CPM_DOWNLOAD_VERSION 0.38.0)

if(CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
            https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
            ${CPM_DOWNLOAD_LOCATION}
    )
endif()

include(${CPM_DOWNLOAD_LOCATION})

CPMAddPackage("gh:TheBlackPlague/MantaRay#master")

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
        include/Heuristic/PST.hpp
        include/Heuristic/Material.hpp
        include/Version.hpp
        include/SearchEngine.hpp
        include/Evaluator.hpp
        include/AlphaBetaSearch.hpp
        include/Types.hpp
        include/TranspositionTable.hpp
        src/TranspositionTable.cpp
        include/Heuristic/PawnStructure.hpp
        include/Heuristic/Mobility.hpp
        src/NNUEEvaluator.cpp
)

target_include_directories(LiquorChess PRIVATE "include")

target_compile_definitions(LiquorChess PUBLIC LIQUOR_VERSION="${LIQUOR_VERSION}")
set_target_properties(LiquorChess PROPERTIES OUTPUT_NAME "Liquor-${LIQUOR_VERSION}")

target_compile_options(LiquorChess PRIVATE $<$<CONFIG:Release>:-O3 -march=x86-64-v3 -flto -DNDEBUG>)
target_link_options(LiquorChess PRIVATE $<$<CONFIG:Release>:-flto>)

target_link_libraries(LiquorChess MantaRay)