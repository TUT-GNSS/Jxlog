cmake_minimum_required(VERSION 3.20)
project(decoder)

# # 设置vcpkg工具链文件
# set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
# include(${CMAKE_TOOLCHAIN_FILE})

add_executable(decoder decode.cpp decode_formatter.cpp)
# find_package(protobuf CONFIG REQUIRED)
# target_link_libraries(decoder logger protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
target_link_libraries(decoder logger)