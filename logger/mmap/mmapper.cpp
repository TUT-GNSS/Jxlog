#include <string.h>

#include "mmap/mmapper.h"
#include "utils/file_util.h"
#include "utils/sys_util.h"

namespace logger {
namespace mmap {

static constexpr size_t kDefaultCapacity = 512 * 1024;  // 512KB

MMapper::MMapper(FilePath file_path) : file_path_(std::move(file_path)), mmaped_address_(nullptr), capacity_(0) {
  size_t file_size = logger::filesystem::GetFileSize(file_path);
  Reserve_(std::max(file_size, kDefaultCapacity));
  Init_();
}

void MMapper::Resize(size_t new_size) {
  if (!IsValid_()) {
    return;
  }
  EnsureCapacity_(new_size);
  GetHeader_()->size = new_size;
}

void MMapper::Reserve_(size_t new_capcity) {
  if (new_capcity <= capacity_) {
    return;
  }
  // 获取页表大小
  size_t page_size = utils::GetPageSize();
  // 扩容new_capcity + 一页页表的空间, 随后调整为page_size的整数倍
  new_capcity = (((new_capcity + page_size - 1) / page_size) * page_size);
  if (new_capcity == capacity_) {
    return;
  }
  // 解除原有映射
  Unmap_();
  // 映射分配新内存
  TryMap_(new_capcity);
  capacity_ = new_capcity;
}

void MMapper::Init_() {

  MmapHeader* mmap_header_ptr = GetHeader_();

  if (!mmap_header_ptr) {
    return;
  }
  // 初始化MmapHeader
  if (mmap_header_ptr->magic != MmapHeader::kMagic) {
    mmap_header_ptr->magic = MmapHeader::kMagic;
    mmap_header_ptr->size = 0;
  }
}

MMapper::MmapHeader* MMapper::GetHeader_() const {
  if (!mmaped_address_) {
    return nullptr;
  }
  if (capacity_ < sizeof(MmapHeader)) {
    return nullptr;
  }
  // 类型转换为MmapHeader* 相当于 (MmapHeader*)mmaped_address_
  return static_cast<MmapHeader*>(mmaped_address_);
}

bool MMapper::IsValid_() const {
  MmapHeader* mmap_header_ptr = GetHeader_();
  if (!mmap_header_ptr) {
    return false;
  }
  // 检查magic是否与kMagic相同
  return mmap_header_ptr->magic == MmapHeader::kMagic;
}

uint8_t* MMapper::Data() const {
  if (!IsValid_()) {
    return nullptr;
  }
  // 偏移MmapHeader大小头部
  return static_cast<uint8_t*>(mmaped_address_) + sizeof(MmapHeader);
}

size_t MMapper::Size() const {
  if (!IsValid_()) {
    return 0;
  }
  return GetHeader_()->size;
}

void MMapper::Clear() {
  if (!IsValid_()) {
    return;
  }
  GetHeader_()->size = 0;
}

void MMapper::Push(const void* data, size_t size) {
  if (!IsValid_()) {
    return;
  }
  size_t new_size = Size() + size;
  EnsureCapacity_(new_size);
  memcpy(Data() + Size(), data, size);
  GetHeader_()->size = new_size;
}

void MMapper::EnsureCapacity_(size_t new_size) {
  size_t real_size = new_size + sizeof(MmapHeader);
  if (real_size <= capacity_) {
    return;
  }
  size_t new_capacity = capacity_;
  // 以page_size整数倍扩容capacity
  while(new_capacity < real_size){
    new_capacity+=logger::utils::GetPageSize();
  }

  Reserve_(new_capacity);
}

double MMapper::GetRatio() const {
  if (!IsValid_()) {
    return 0.0;
  }
  return static_cast<double>(Size()) / (Capacity_() - sizeof(MmapHeader));
}

}  // namespace mmap
}  // namespace logger
