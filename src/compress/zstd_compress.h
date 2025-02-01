#include "compress/compress.h"
#include "zstd.h"

namespace logger
{
namespace compress
{

class ZstdCompression final : public Compression{
public:
  ZstdCompression();
  ~ZstdCompression() override;

  size_t Compress(const void* input, size_t input_size, void* output, size_t output_size) override;

  std::string Uncompress(const void* data, size_t size) override;

  void ResetStream() override;

  size_t CompressedBound(size_t input_size) override;

 private:
  void ResetUncompressStream_();

 private:
  ZSTD_CCtx* cctx_;
  ZSTD_DCtx* dctx_;
};    
} // namespace compress
} // namespace logger
