#include <cstring>

#include "compress/zstd_compress.h"

namespace logger {
namespace compress {

ZstdCompression::ZstdCompression() {
  // 创建压缩上下文
  cctx_ = ZSTD_createCCtx();
  // 设置压缩级别为 5
  ZSTD_CCtx_setParameter(cctx_, ZSTD_c_compressionLevel, 5);

  // 创建解压缩上下文
  dctx_ = ZSTD_createDCtx();
}

ZstdCompression::~ZstdCompression() {
  // 释放压缩上下文
  if (cctx_) {
    ZSTD_freeCCtx(cctx_);
  }

  // 释放解压缩上下文
  if (dctx_) {
    ZSTD_freeDCtx(dctx_);
  }
}

size_t ZstdCompression::Compress(const void* input, size_t input_size, void* output, size_t output_size) {
  if (!input || input_size == 0) {
    return 0;
  }

  // 初始化输入和输出缓冲区
  ZSTD_inBuffer input_buffer = {input, input_size, 0};
  ZSTD_outBuffer output_buffer = {const_cast<void*>(reinterpret_cast<const void*>(output)), output_size, 0};

  // 调用 ZSTD_compressStream2 进行压缩
  size_t ret = ZSTD_compressStream2(cctx_, &output_buffer, &input_buffer, ZSTD_e_flush);

  // 检查是否发生错误
  if (ZSTD_isError(ret) != 0) {
    return 0;
  }

  // 返回压缩后的数据大小
  return output_buffer.pos;
}

// 检查数据是否为 ZSTD 压缩格式
static bool IsZSTDCompressed(const void* input, size_t input_size) {
  // 检查输入数据是否为空
  if (!input) {
    return false;
  }

  // 检查输入数据大小是否小于 ZSTD 的最小头部大小
  if (input_size < 4) {
    return false;
  }

  // 定义 ZSTD 的魔数（大端和小端）
  const uint8_t kMagicNumberBigEndian[] = {0x28, 0xb5, 0x2f, 0xfd};
  const uint8_t kMagicNumberLittleEndian[] = {0xfd, 0x2f, 0xb5, 0x28};

  // 检查输入数据的头部是否匹配 ZSTD 的魔数
  // memcp 如果两个内存区域的内容相同，返回 0；否则返回非零值
  if (memcmp(input, kMagicNumberBigEndian, sizeof(kMagicNumberBigEndian)) == 0) {
    return true;
  }

  if (memcmp(input, kMagicNumberLittleEndian, sizeof(kMagicNumberLittleEndian)) == 0) {
    return true;
  }

  return false;
}

// 解压缩数据
std::string ZstdCompression::Uncompress(const void* data, size_t size) {
  if (!data || size == 0) {
    return "";
  }

  // 检查数据是否为 ZSTD 压缩格式
  if (IsZSTDCompressed(data, size)) {
    // 重置解压缩流
    ResetStream();
  }

  // 初始化输出字符串
  std::string output;
  output.reserve(10 * 1024);  // 预分配 10KB 的空间

  // 初始化输入和输出缓冲区
  ZSTD_inBuffer input = {data, size, 0};
  ZSTD_outBuffer output_buffer = {const_cast<void*>(reinterpret_cast<const void*>(output.data())), output.capacity(),
                                  0};

  // 调用 ZSTD_decompressStream 进行解压缩
  size_t ret = ZSTD_decompressStream(dctx_, &output_buffer, &input);

  // 检查是否发生错误
  if (ZSTD_isError(ret) != 0) {
    return "";
  }

  // 返回解压缩后的数据
  output = std::string(reinterpret_cast<char*>(output_buffer.dst), output_buffer.pos);
  return output;
}

// 重置压缩流
void ZstdCompression::ResetStream() {
  // 重置压缩上下文
  if (cctx_) {
    ZSTD_CCtx_reset(cctx_, ZSTD_reset_session_only);
  }
}

// 计算压缩数据的最大可能大小
size_t ZstdCompression::CompressedBound(size_t input_size) {
  // 调用 ZSTD_compressBound 计算压缩数据的最大可能大小
  return ZSTD_compressBound(input_size);
}

// 重置解压缩流
void ZstdCompression::ResetUncompressStream_() {
  // 重置解压缩上下文
  if (dctx_) {
    ZSTD_DCtx_reset(dctx_, ZSTD_reset_session_only);
  }
}

}  // namespace compress
}  // namespace logger