cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME "logger")
project(PROJECT_NAME)

set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Target architecture")



include_directories("/Users/jixi/Documents/code/JxLog/src") # 头文件目录

set(CMAKE_CXX_STANDARD 17)

# 设置vcpkg工具链文件
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

include(${CMAKE_TOOLCHAIN_FILE})

set(FORMATTER_SRCS formatter/default_formatter.cpp)
set(SINK_SRCS sinks/console_sink.cpp )

set(SRCS
    logger.cpp
    ${FORMATTER_SRCS}
    ${SINK_SRCS}
)


add_executable(logger test.cpp ${SRCS}) # 生成可执行文件，这里程序名即为功能名

# 从环境变量中读取 ENABLE_LOG 的值
set(ENABLE_LOG $ENV{ENABLE_LOG})
# 根据环境变量的值添加宏定义
if(ENABLE_LOG)
    target_compile_definitions(logger PRIVATE ENABLE_LOG)
endif()


find_package(GTest REQUIRED)
target_link_libraries(logger PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(logger PRIVATE fmt::fmt)

