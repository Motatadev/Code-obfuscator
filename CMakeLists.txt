cmake_minimum_required(VERSION 3.15)
project(Obfuscator LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(obfuscator
    src/main.cpp
    src/obfuscator.cpp
    src/language.cpp
    src/lexer.cpp
    src/encodings.cpp
)

if(MSVC)
    target_compile_options(obfuscator PRIVATE /W4 /O2)
else()
    target_compile_options(obfuscator PRIVATE -Wall -Wextra -O2)
endif()
