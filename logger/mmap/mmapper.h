#pragma once

#include <filesystem>
#include <memory>

namespace logger {
namespace mmap {

class MMapper {
 public:
  using FilePath = std::filesystem::path;

  explicit MMapper(FilePath file_path);

  ~MMapper() = default;
  MMapper(const MMapper& other) = delete;
  MMapper& operator=(MMapper other) = delete;

  void Resize(size_t new_size);

  uint8_t* Data() const;

  size_t Size() const;

  void Clear();

  void Push(const void* data, size_t size);

  // mmap实际内容与mmap所占空间比率
  double GetRatio() const;

  bool Empty() const { return Size() == 0; }

 private:
  // 内存映射的头部
  struct MmapHeader {
    static constexpr uint32_t kMagic = 0xdeadbeef;
    uint32_t magic = kMagic;
    uint32_t size;
  };

  void Reserve_(size_t new_capacity);

  void EnsureCapacity_(size_t new_size);

  size_t Capacity_() const noexcept { return capacity_; }

  void Sync_();

  bool IsValid_() const;

  MmapHeader* GetHeader_() const;

  void Init_();

  // 根据系统不同有不同实现
  bool TryMap_(size_t capacity);  // 分配新内存
  // 根据系统不同有不同实现
  void Unmap_();  // 解除原有映射

  FilePath file_path_;
  void* mmaped_address_;  // mmap映射内存的首地址
  size_t capacity_;
};

}  // namespace mmap
}  // namespace logger
