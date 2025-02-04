cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME "logger")
project(PROJECT_NAME)

set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Target architecture")



include_directories("/Users/jixi/Documents/code/JxLog/src") # 头文件目录

set(CMAKE_CXX_STANDARD 17)

# 设置vcpkg工具链文件
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

include(${CMAKE_TOOLCHAIN_FILE})

# 从环境变量中读取 ENABLE_LOG 的值
set(ENABLE_LOG $ENV{ENABLE_LOG})
# 根据环境变量的值添加宏定义
if(ENABLE_LOG)
    target_compile_definitions(logger PRIVATE ENABLE_LOG)
endif()

set(FORMATTER_SRCS formatter/formatter.cpp formatter/effective_formatter.cpp formatter/default_formatter.cpp)
set(SINK_SRCS sinks/console_sink.cpp sinks/effective_sink.cpp )
set(CONTEXT_SRCS context/context.cpp context/executor.cpp context/thread_pool.cpp)
set(COMPRESS_SRCS compress/zlib_compress.cpp compress/zstd_compress.cpp)
set(CRYPT_SRCS crypt/aes_crypt.cpp crypt/crypt.cpp)
set(PROTO_SRCS proto/effective_msg.pb.cc)
# 条件编译 根据系统编译不同文件
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(UTILS_SRCS utils/sys_util_linux.cpp utils/file_util.cpp)
    set(MMAP_SRCS mmap/mmapper.cpp mmap/mmapper_linux.cpp)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(UTILS_SRCS utils/sys_util_linux.cpp utils/file_util.cpp)
    set(MMAP_SRCS mmap/mmapper.cpp mmap/mmapper_linux.cpp)
else()
    message(FATAL_ERROR "system unsupported.")
endif()

set(SRCS
    logger.cpp
    ${FORMATTER_SRCS}
    ${SINK_SRCS}
    ${MMAP_SRCS}
    ${UTILS_SRCS}
    ${CONTEXT_SRCS}
    ${COMPRESS_SRCS}
    ${CRYPT_SRCS}
    ${PROTO_SRCS}
)


add_executable(logger test.cpp ${SRCS}) # 生成可执行文件，这里程序名即为功能名


find_package(GTest REQUIRED)
target_link_libraries(logger PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(logger PRIVATE fmt::fmt)

find_package(ZLIB REQUIRED)
target_link_libraries(logger PRIVATE ZLIB::ZLIB)

find_package(zstd CONFIG REQUIRED)
target_link_libraries(logger PRIVATE zstd::libzstd)

find_package(OpenSSL REQUIRED)
target_link_libraries(logger PRIVATE OpenSSL::SSL)
target_link_libraries(logger PRIVATE OpenSSL::Crypto)

find_package(cryptopp CONFIG REQUIRED)
target_link_libraries(logger PRIVATE cryptopp::cryptopp)

find_package(protobuf CONFIG REQUIRED)
target_link_libraries(logger PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)