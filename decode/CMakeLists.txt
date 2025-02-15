cmake_minimum_required(VERSION 3.20)
project(decoder)

add_executable(decoder decode.cpp decode_formatter.cpp)
target_link_libraries(decoder logger)