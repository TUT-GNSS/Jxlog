#include <iostream>
#include <memory>

#include "compress/zlib_compress.h"
#include "compress/zstd_compress.h"
#include "context/context.h"
#include "crypt/crypt.h"
#include "formatter/effective_formatter.h"
#include "log_common.h"
#include "logger.h"
#include "proto/effective_msg.pb.h"
#include "sinks/console_sink.h"
#include "sinks/sink.h"
#include "space.h"
#include "utils/sys_util.h"

int main() {
  // logger::SourceLocation sl("test.cpp",0,"func_name_in");
  // std::string msgstr= "hello logger";
  // logger::LogMsg log_msg(logger::LogLevel::kInfo,msgstr);
  // logger::formatter::EffectiveFormatter eff;
  // std::string eff_str;
  // eff.Format(log_msg,eff_str);

  // EffectiveMsg new_eff;
  // if(!new_eff.ParseFromArray(eff_str.data(),eff_str.size())){
  //     std::cerr<<" parse error"<<std::endl;
  //     return -1;
  // }
  // std::cout<<"eff: level: "<< new_eff.level()<<"\n";

  logger::megabytes mbytes{4};
  std::cout << mbytes.count() << "\n";
  logger::kilobytes kbytes = logger::space_cast<logger::kilobytes>(mbytes);
  std::cout << kbytes.count() << "\n";
  return 0;
}

// int main() {
//     // 输入数据
//     // 生成较大的输入数据
//     std::string input = "This is a test string for compression and
//     decompression."; std::string large_input; for (int i = 0; i < 1000; ++i)
//     {
//         large_input += input;
//     }

//     // 测试 ZstdCompression
//     {
//         logger::compress::ZstdCompression zstd_compressor;

//         // 压缩数据
//         auto start = std::chrono::high_resolution_clock::now();
//         size_t input_size = input.size();
//         size_t output_size = zstd_compressor.CompressedBound(input_size);
//         std::vector<uint8_t> compressed_data(output_size);
//         size_t compressed_size = zstd_compressor.Compress(input.data(),
//         input_size, compressed_data.data(), output_size); auto end =
//         std::chrono::high_resolution_clock::now();
//         std::chrono::duration<double, std::milli> compress_duration = end -
//         start;

//         if (compressed_size == 0) {
//             std::cerr << "Zstd Compression failed" << std::endl;
//             return 1;
//         }
//         compressed_data.resize(compressed_size);

//         // 解压数据
//         start = std::chrono::high_resolution_clock::now();
//         std::string decompressed_data =
//         zstd_compressor.Uncompress(compressed_data.data(), compressed_size);
//         end = std::chrono::high_resolution_clock::now();
//         std::chrono::duration<double, std::milli> decompress_duration = end -
//         start;

//         if (decompressed_data.empty()) {
//             std::cerr << "Zstd Decompression failed" << std::endl;
//             return 1;
//         }

//         std::cout << "Zstd Compression time: " << compress_duration.count()
//         << " ms" << std::endl; std::cout << "Zstd Decompression time: " <<
//         decompress_duration.count() << " ms" << std::endl;
//     }

//     // 测试 ZlibCompression
//     {
//         logger::compress::ZlibCompression zlib_compressor;
//         // 压缩数据
//         auto start = std::chrono::high_resolution_clock::now();
//         zlib_compressor.ResetStream();
//         size_t input_size = input.size();
//         size_t output_size = zlib_compressor.CompressedBound(input_size);
//         std::vector<uint8_t> compressed_data(output_size);
//         size_t compressed_size = zlib_compressor.Compress(input.data(),
//         input_size, compressed_data.data(), output_size); auto end =
//         std::chrono::high_resolution_clock::now();
//         std::chrono::duration<double, std::milli> compress_duration = end -
//         start;

//         if (compressed_size == 0) {
//             std::cerr << "Zlib Compression failed" << std::endl;
//             return 1;
//         }
//         compressed_data.resize(compressed_size);

//         // 解压数据
//         start = std::chrono::high_resolution_clock::now();
//         std::string decompressed_data =
//         zlib_compressor.Uncompress(compressed_data.data(), compressed_size);
//         end = std::chrono::high_resolution_clock::now();
//         std::chrono::duration<double, std::milli> decompress_duration = end -
//         start;

//         if (decompressed_data.empty()) {
//             std::cerr << "Zlib Decompression failed" << std::endl;
//             return 1;
//         }

//         std::cout << "Zlib Compression time: " << compress_duration.count()
//         << " ms" << std::endl; std::cout << "Zlib Decompression time: " <<
//         decompress_duration.count() << " ms" << std::endl;
//     }

//     return 0;
// }
