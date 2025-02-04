#pragma once

#include <memory>
#include <string>

namespace logger {
namespace compress {
// 压缩类的父类,提供统一的接口
class Compression {
 public:
  Compression() = default;
  virtual ~Compression() = default;

  virtual size_t Compress(const void* input, size_t input_size, void* output,
                          size_t output_size) = 0;

  virtual size_t CompressedBound(size_t input_size) = 0;

  virtual std::string Uncompress(const void* data, size_t size) = 0;

  virtual void ResetStream() = 0;
};
}  // namespace compress
}  // namespace logger
