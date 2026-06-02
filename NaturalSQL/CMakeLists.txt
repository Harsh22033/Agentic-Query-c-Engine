cmake_minimum_required(VERSION 3.14)

project(NaturalSQL VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -O3)
endif()

include_directories(src)

set(ENGINE_SOURCES
    src/parser/Lexer.cpp
    src/parser/Parser.cpp
    src/storage/CSVLoader.cpp
    src/engine/Executor.cpp
)

add_executable(
    NaturalSQL
    src/main.cpp
    ${ENGINE_SOURCES}
)