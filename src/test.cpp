#pragma once
#include <iostream>
#include <memory>

#include "logger.h"
#include "sinks/sink.h"
#include "sinks/console_sink.h"
#include "log_common.h"
#include "utils/sys_util.h"
#include "context/context.h"
#include "compress/zlib_compress.h"
#include "compress/zstd_compress.h"

// int main(){
//     // auto sinkPtr=std::make_shared<logger::sink::ConsoleSink>();
//     // logger::Logger log("test1",sinkPtr);
//     // log.Log(logger::LogLevel::kInfo,logger::SourceLocation("test",1,"test.111"),std::to_string(logger::utils::GetThreadID()));

//     // logger::context::Context* context = logger::context::Context::GetInstance();
//     // context->GetExecutor()->AddTaskRunner(1);
//     // for(int i=0;i<5;++i){
//     //     context->GetExecutor()->PostTask(1,[i](){std::cout<<"1111  :"<<i<<"\n";});
//     // }
//     // while(1){}
//     // 创建任务运行器

//     logger::context::TaskRunnerTag runner_tag = NEW_TASK_RUNNER(1);

//     // 提交一个普通任务
//     POST_TASK(runner_tag, [] {
//         std::cout << "Immediate task executed" << std::endl;
//     });

//     // 提交一个延迟任务
//     POST_DELAY_TASK(runner_tag, [] {
//         std::cout << "Delayed task executed" << std::endl;
//     }, std::chrono::milliseconds(200));

//     // 提交一个重复任务
//     POST_REPEATED_TASK(runner_tag, [] {
//         std::cout << "Repeated task executed" << std::endl;
//     }, std::chrono::milliseconds(100), 3);

//     // 确保所有任务都已完成
//     std::this_thread::sleep_for(std::chrono::seconds(2));

//     return 0;
// }


#include <iostream>
#include <vector>

int main() {
    // 输入数据
    // 生成较大的输入数据
    std::string input = "This is a test string for compression and decompression.";
    std::string large_input;
    for (int i = 0; i < 1000; ++i) {
        large_input += input;
    }

    // 测试 ZstdCompression
    {
        logger::compress::ZstdCompression zstd_compressor;

        // 压缩数据
        auto start = std::chrono::high_resolution_clock::now();
        size_t input_size = input.size();
        size_t output_size = zstd_compressor.CompressedBound(input_size);
        std::vector<uint8_t> compressed_data(output_size);
        size_t compressed_size = zstd_compressor.Compress(input.data(), input_size, compressed_data.data(), output_size);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> compress_duration = end - start;

        if (compressed_size == 0) {
            std::cerr << "Zstd Compression failed" << std::endl;
            return 1;
        }
        compressed_data.resize(compressed_size);

        // 解压数据
        start = std::chrono::high_resolution_clock::now();
        std::string decompressed_data = zstd_compressor.Uncompress(compressed_data.data(), compressed_size);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> decompress_duration = end - start;

        if (decompressed_data.empty()) {
            std::cerr << "Zstd Decompression failed" << std::endl;
            return 1;
        }

        std::cout << "Zstd Compression time: " << compress_duration.count() << " ms" << std::endl;
        std::cout << "Zstd Decompression time: " << decompress_duration.count() << " ms" << std::endl;
    }

    // 测试 ZlibCompression
    {
        logger::compress::ZlibCompression zlib_compressor;
        // 压缩数据
        auto start = std::chrono::high_resolution_clock::now();
        zlib_compressor.ResetStream();
        size_t input_size = input.size();
        size_t output_size = zlib_compressor.CompressedBound(input_size);
        std::vector<uint8_t> compressed_data(output_size);
        size_t compressed_size = zlib_compressor.Compress(input.data(), input_size, compressed_data.data(), output_size);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> compress_duration = end - start;
        
        if (compressed_size == 0) {
            std::cerr << "Zlib Compression failed" << std::endl;
            return 1;
        }
        compressed_data.resize(compressed_size);

        // 解压数据
        start = std::chrono::high_resolution_clock::now();
        std::string decompressed_data = zlib_compressor.Uncompress(compressed_data.data(), compressed_size);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> decompress_duration = end - start;

        if (decompressed_data.empty()) {
            std::cerr << "Zlib Decompression failed" << std::endl;
            return 1;
        }

        std::cout << "Zlib Compression time: " << compress_duration.count() << " ms" << std::endl;
        std::cout << "Zlib Decompression time: " << decompress_duration.count() << " ms" << std::endl;
    }

    return 0;
}