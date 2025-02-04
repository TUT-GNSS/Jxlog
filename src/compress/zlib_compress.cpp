#include "compress/zlib_compress.h"

namespace logger {
namespace compress {
size_t ZlibCompression::Compress(const void* input, size_t input_size,
                                 void* output, size_t output_size) {
  if (!input_size || !output_size) {
    return 0;
  }
  // 检查压缩流是否有效
  if (!compress_stream_) {
    return 0;
  }
  // 设置输入输出数据的指针和大小
  compress_stream_->next_in = (Bytef*)input;
  compress_stream_->avail_in = static_cast<uInt>(input_size);

  compress_stream_->next_out = (Bytef*)output;
  compress_stream_->avail_out = static_cast<uInt>(output_size);

  int ret = Z_OK;
  do {
    // 调用 deflate 函数进行压缩操作 Z_SYNC_FLUSH 表示在每次调用后刷新输出缓冲区
    ret = deflate(compress_stream_.get(), Z_SYNC_FLUSH);
    if (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_STREAM_END) {
      return 0;
    }
  } while (ret == Z_BUF_ERROR);
  // 计算压缩后的数据大小
  size_t out_len = output_size - compress_stream_->avail_out;
  return out_len;
}

static bool IsCompress(const void* data, size_t size) {
  if (!data) {
    return false;
  }

  if (size < 2) {
    return false;
  }

  uint16_t magic = *(uint16_t*)data;
  if (magic == 0x9c78 || magic == 0xda78 || magic == 0x5e78 ||
      magic == 0x0178) {
    return true;
  }

  return false;
}

std::string ZlibCompression::Uncompress(const void* data, size_t size) {
  if (!data) {
    return "";
  }
  if (IsCompress(data, size)) {
    // 重置解压缩流
    ResetUncompressStream_();
  }
  // 检查解压缩流是否有效
  if (!uncompress_stream_) {
    return "";
  }
  // 设置解压缩流的输入
  uncompress_stream_->next_in = (Bytef*)data;
  uncompress_stream_->avail_in = static_cast<uInt>(size);

  std::string output;
  output.reserve(size);
  // 循环解压缩数据
  while (uncompress_stream_->avail_in > 0) {
    // 定义一个缓冲区，用于存储解压缩后的数据
    char buffer[4096] = {0};
    uncompress_stream_->next_out = (Bytef*)buffer;
    uncompress_stream_->avail_out = sizeof(buffer);
    // 调用zlib 的 inflate 函数进行解压缩
    int ret = inflate(uncompress_stream_.get(), Z_SYNC_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END) {
      return "";
    }
    // 将解压缩后的数据追加到输出字符串中
    output.append(buffer, sizeof(buffer) - uncompress_stream_->avail_out);
  }
  return output;
}

void ZlibCompression::ResetStream() {
  // 创建并初始化压缩流对象
  compress_stream_ =
      std::unique_ptr<z_stream, ZStreamDeflateDeleter>(new z_stream());
  compress_stream_->zalloc = Z_NULL;
  compress_stream_->zfree = Z_NULL;
  compress_stream_->opaque = Z_NULL;
  // 调用 deflateInit2 初始化压缩流
  int32_t ret =
      deflateInit2(compress_stream_.get(), Z_BEST_COMPRESSION, Z_DEFLATED,
                   MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
  if (ret != Z_OK) {
    compress_stream_.reset();  // 初始化失败，释放压缩流
  }
}

void ZlibCompression::ResetUncompressStream_() {
  // 创建并初始化解压缩流
  uncompress_stream_ =
      std::unique_ptr<z_stream, ZStreamInflateDeleter>(new z_stream());
  uncompress_stream_->zalloc = Z_NULL;
  uncompress_stream_->zfree = Z_NULL;
  uncompress_stream_->opaque = Z_NULL;
  uncompress_stream_->avail_in = 0;
  uncompress_stream_->next_in = Z_NULL;
  // 调用inflateInit2初始化解压缩流
  int32_t ret = inflateInit2(uncompress_stream_.get(), MAX_WBITS);
  if (ret != Z_OK) {
    uncompress_stream_.reset();
  }
}

size_t ZlibCompression::CompressedBound(size_t input_size) {
  // 使用 zlib 的 deflateBound 函数计算压缩数据的最大可能大小
  //   printf("%d\n",input_size + 10);
  //   return deflateBound(compress_stream_.get(), input_size);
  return input_size + 10;
}

}  // namespace compress
}  // namespace logger
