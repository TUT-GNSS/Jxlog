#include "sinks/effective_sink.h"

#include <fmt/core.h>  // 引入fmt库的核心头文件
#include <fstream>

#include "compress/zstd_compress.h"
#include "crypt/aes_crypt.h"
#include "formatter/effective_formatter.h"
#include "utils/file_util.h"
#include "utils/sys_util.h"
#include "utils/timer_count.h"

namespace logger {
namespace sink {
EffectiveSink::EffectiveSink(Conf conf) : conf_(conf) {
  // 路径不存在则创建
  if (!std::filesystem::exists(conf_.dir)) {
    std::filesystem::create_directories(conf_.dir);
  }
  formatter_ = std::make_unique<formatter::EffectiveFormatter>();  // 初始化formatter_

  task_runner_ = NEW_TASK_RUNNER(20010305);  // tag为20010305
  // 初始化crypt_
  auto ecdh_key = crypt::GenECDHKey();
  auto client_pri = std::get<0>(ecdh_key);
  client_pub_key_ = std::get<1>(ecdh_key);
  LOG_INFO("EffectiveSink: client pub size {}", client_pub_key_.size());
  // std::string svr_pub_key_bin = crypt::HexKeyToBinary(conf_.pub_key);
  std::string svr_pub_key_bin = conf_.pub_key;
  std::string shared_secret = crypt::GenECDHSharedSecret(client_pri, svr_pub_key_bin);
  LOG_INFO("shared_secret没报错 size:{}",shared_secret.size());
  crypt_ = std::make_unique<crypt::AESCrypt>(shared_secret);
  // 初始化compress_
  compress_ = std::make_unique<compress::ZstdCompression>();
  // 初始化master_cache_和slave_cache_
  master_cache_ = std::make_unique<mmap::MMapper>(conf_.dir / "master_cache");
  slave_cache_ = std::make_unique<mmap::MMapper>(conf_.dir / "slave_cache");
  // 创建mmap失败
  if (!master_cache_ || !slave_cache_) {
    throw std::runtime_error("EffectiveSink::EffectiveSink: create mmap failed");
  }
  // 从缓冲区非空,则先将从缓冲区任务写入文件
  if (!slave_cache_->Empty()) {
    is_slave_free_.store(true);
    PrepareToFile_();
    WAIT_TASK_IDLE(task_runner_);
  }
  // 主缓冲区非空,从缓冲区为空则swap主从缓冲区,然后写入文件
  if (!master_cache_->Empty()) {
    if (is_slave_free_) {
      is_slave_free_.store(false);
      SwapCache_();
    }
    PrepareToFile_();
  }
  // 按时重复日志淘汰检查的任务
  POST_REPEATED_TASK(task_runner_, [this]() { ElimateFiles_(); }, conf_.interval, -1);
}

void EffectiveSink::Log(const LogMsg& msg) {
  static thread_local std::string buf;
  // 通过Formatter序列化msg到buf中
  formatter_->Format(msg, buf);

  // 如果主缓冲区空 重置压缩流
  if (master_cache_->Empty()) {
    compress_->ResetStream();
  }
  // 压缩 加密 写入master_cache_必须加锁
  {
    std::lock_guard<std::mutex> lock(mtx_);
    // 压缩
    //  重置压缩buf的容量为压缩后长度
    compressed_buf_.reserve(compress_->CompressedBound(buf.size()));
    // 压缩并得到压缩后大小
    size_t compressed_size =
        compress_->Compress(buf.data(), buf.size(), compressed_buf_.data(), compressed_buf_.capacity());
    if (compressed_size == 0) {
      LOG_ERROR("EffectiveSink::Log: compress failed");
      return;
    }
    // 加密
    encryped_buf_.clear();
    encryped_buf_.reserve(compressed_size + 16);  // 预留加密头部容量
    crypt_->Encrypt(compressed_buf_.data(), compressed_size, encryped_buf_);
    if (encryped_buf_.empty()) {
      LOG_ERROR("EffectiveSink::Log: encrypt failed");
      return;
    }
    // 写入主缓冲区
    WriteToCache_(encryped_buf_.data(), encryped_buf_.size());
  }
  // 判断缓冲区利用率是否超过80%,超过写入日志文件
  if (NeedCacheToFile_()) {
    // 判断从缓冲区是否空闲
    if (is_slave_free_.load()) {  // 原子变量，如果slave_cache_是空的，才能交换
      is_slave_free_.store(false);
      SwapCache_();
    }
    // 写入日志文件
    PrepareToFile_();
  }
}
void EffectiveSink::SetFormatter(std::unique_ptr<formatter::Formatter> formatter) {}

void EffectiveSink::Flush() {
  // 主从都刷新
  TIMER_COUNT("Flush");
  // 从缓冲区写入文件
  PrepareToFile_();
  WAIT_TASK_IDLE(task_runner_);

  // 交换主从缓冲区后，再写入文件
  if (is_slave_free_.load()) {
    is_slave_free_.store(false);
    SwapCache_();
  }
  PrepareToFile_();
  WAIT_TASK_IDLE(task_runner_);
}

void EffectiveSink::SwapCache_() {
  std::lock_guard<std::mutex> lock(mtx_);
  // 交换主从缓冲区指针
  std::swap(master_cache_, slave_cache_);
}

bool EffectiveSink::NeedCacheToFile_() {
  // 返回主缓冲区实际内容与mmap空间所占比率是否超过80%
  return master_cache_->GetRatio() > 0.8;
}

void EffectiveSink::WriteToCache_(const void* data, uint32_t size) {
  // 缓存头部,保存数据size
  detail::ItemHeader item_header;
  item_header.size = size;
  master_cache_->Push(&item_header, sizeof(item_header));
  master_cache_->Push(data, size);
}

void EffectiveSink::PrepareToFile_() {
  // 将任务发布出去
  POST_TASK(task_runner_, [this]() { CacheToFile_();});
}

void EffectiveSink::CacheToFile_() {
  TIMER_COUNT("CacheToFile_");
  // 从缓冲区空则返回
  if (is_slave_free_.load()) {
    return;
  }
  if (slave_cache_->Empty()) {
    is_slave_free_.store(true);
    return;
  }
  // 从cache内容转移到日志文件中
  {
    auto file_path = GetFilePath_();
    // 加入头部
    detail::ChunkHeader chunk_header;
    chunk_header.size = slave_cache_->Size();
    // copy公钥
    memcpy(chunk_header.pub_key, client_pub_key_.data(), client_pub_key_.size());
    // 写入头部和数据通过文件追加模式
    std::ofstream ofs(file_path, std::ios::binary | std::ios::app);
    ofs.write(reinterpret_cast<char*>(&chunk_header), sizeof(chunk_header));
    ofs.write(reinterpret_cast<char*>(slave_cache_->Data()), chunk_header.size);
  }
  // 清空从缓冲区,设置从缓冲区空闲
  slave_cache_->Clear();
  is_slave_free_.store(true);
}

std::filesystem::path EffectiveSink::GetFilePath_() {
  // 文件名格式：{prefix}_{datetime}.log 或 {prefix}_{datetime}_{index}.log
  auto GetDateTimePath = [this]() -> std::filesystem::path {
    std::time_t now = std::time(nullptr);
    std::tm tm;
    utils::LocalTime(&tm, &now);
    char time_buf[32] = {0};
    std::strftime(time_buf, sizeof(time_buf), "%Y%m%d%H%M%S", &tm);
    return (conf_.dir / (conf_.prefix + "_" + time_buf));
  };

  if (log_file_path_.empty()) {
    // 文件路径为空 直接赋值
    log_file_path_ = GetDateTimePath().string() + ".log";
  } else {
    // 获取文件大小
    auto file_size = filesystem::GetFileSize(log_file_path_);
    // 文件大小超过单个文件最大值 创建新文件 否则继续用之前log_file_path_
    if (file_size > conf_.single_size.count()) {
      std::string date_time_path = GetDateTimePath().string();
      std::string file_path = date_time_path + ".log";
      // 同名文件 加索引号区分
      if (std::filesystem::exists(file_path)) {
        int idx = 0;
        // 寻找同名文件个数
        for (auto& path : std::filesystem::directory_iterator(conf_.dir)) {
          if (path.path().filename().string().find(date_time_path) != std::string::npos) {
            ++idx;
          }
        }
        log_file_path_ = date_time_path + "_" + std::to_string(idx) + ".log";
      } else {
        log_file_path_ = std::move(file_path);
      }
    }
  }
  LOG_INFO("EffectiveSink::GetFilePath_: log_file_path={}", log_file_path_.string());
  return log_file_path_;
}

void EffectiveSink::ElimateFiles_() {
  LOG_INFO("EffectiveSink::ElimateFiles_: start");
  // 将目录下所有日志文件存入files数组
  std::vector<std::filesystem::path> files;
  for (auto& path : std::filesystem::directory_iterator(conf_.dir)) {
    if (path.path().extension() == ".log") {
      files.push_back(path.path());
    }
  }
  // 根据文件最后写入时间倒序排序
  std::sort(files.begin(), files.end(), [](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
    return std::filesystem::last_write_time(lhs) > std::filesystem::last_write_time(rhs);
  });

  // 超过总文件最大size 淘汰日期早的日志文件
  size_t total_bytes = space_cast<bytes>(conf_.total_size).count();
  size_t used_bytes = 0;
  for (auto& file : files) {
    used_bytes += filesystem::GetFileSize(file);
    if (used_bytes > total_bytes) {
      LOG_INFO("EffectiveSink::ElimateFiles_: remove file={}", file.string());
      std::filesystem::remove(file);
    }
  }
}

}  // namespace sink
}  // namespace logger
