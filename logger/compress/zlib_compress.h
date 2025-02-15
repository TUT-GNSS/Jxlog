#pragma once

#include "compress/compress.h"
#include "zlib.h"

namespace logger {
namespace compress {
// 用于处理压缩操作（deflate）的删除器
struct ZStreamDeflateDeleter {
  void operator()(z_stream* stream) {
    if (stream) {
      deflateEnd(stream);
      delete stream;
    }
  }
};
// 用于处理解压缩操作（inflate）的删除器
struct ZStreamInflateDeleter {
  void operator()(z_stream* stream) {
    if (stream) {
      inflateEnd(stream);
      delete stream;
    }
  }
};

class ZlibCompression final : public Compression {
 public:
  ~ZlibCompression() override = default;
  size_t Compress(const void* input, size_t input_size, void* output, size_t output_size) override;

  size_t CompressedBound(size_t input_size) override;

  std::string Uncompress(const void* data, size_t size) override;

  void ResetStream() override;

 private:
  void ResetUncompressStream_();

  std::unique_ptr<z_stream, ZStreamDeflateDeleter> compress_stream_;    // 压缩流
  std::unique_ptr<z_stream, ZStreamInflateDeleter> uncompress_stream_;  // 解压流
};
}  // namespace compress
}  // namespace logger
