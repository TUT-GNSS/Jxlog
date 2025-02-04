#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <mutex>

#include "compress/compress.h"
#include "context/context.h"
#include "crypt/crypt.h"
#include "mmap/mmapper.h"
#include "sinks/sink.h"
#include "space.h"

namespace logger {

namespace detail {
struct ChunkHeader {
  static constexpr uint64_t kMagic = 0xdeadbeefdada1100;
  uint64_t magic;
  uint64_t size;
  char pub_key[128];  // 公钥

  ChunkHeader() : magic(kMagic), size(0) {}
};

struct ItemHeader {
  static constexpr uint32_t kMagic = 0xbe5fba11;
  uint32_t magic;
  uint32_t size;

  ItemHeader() : magic(kMagic), size(0) {}
};

}  // namespace detail

namespace sink {
class EffectiveSink : public Sink {
 public:
  // 保存配置的结构体
  struct Conf {
    std::filesystem::path dir;         // 文件目录
    std::string prefix;                // 文件名前缀，文件名命名格式：{prefix}_{datetime}.log
    std::string pub_key;               // 公钥
    std::chrono::minutes interval{5};  // 淘汰间隔
    megabytes single_size{4};          // 单个文件大小
    megabytes total_size{100};         // 总共文件大小
  };
  EffectiveSink(Conf conf);

  ~EffectiveSink() override = default;

  void Log(const LogMsg& msg) override;

  void SetFormatter(std::unique_ptr<formatter::Formatter> formatter) override;

  void Flush() override;

 private:
  void SwapCache_();

  bool NeedCacheToFile_();

  void WriteToCache_(const void* data, uint32_t size);

  void PrepareToFile_();

  void CacheToFile_();

  std::filesystem::path GetFilePath_();

  void ElimateFiles_();

 private:
  Conf conf_;
  std::mutex mtx_;
  std::unique_ptr<formatter::Formatter> formatter_;
  context::TaskRunnerTag task_runner_;
  std::unique_ptr<crypt::Crypt> crypt_;
  std::unique_ptr<compress::Compression> compress_;
  std::unique_ptr<mmap::MMapper> master_cache_;
  std::unique_ptr<mmap::MMapper> slave_cache_;
  std::filesystem::path log_file_path_;
  std::string client_pub_key_;
  std::string compressed_buf_;
  std::string encryped_buf_;
  std::atomic<bool> is_slave_free_{true};
};

}  // namespace sink
}  // namespace logger