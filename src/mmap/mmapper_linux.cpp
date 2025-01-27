#include "mmap/mmapper.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "helpers/deffer.h"

namespace logger
{
namespace mmap
{

void MMapper::Sync_(){
  if(mmaped_address_){
    msync(mmaped_address_, capacity_, MS_SYNC);
  }
}

void MMapper::Unmap_(){
  if(mmaped_address_){
    munmap(mmaped_address_, capacity_);
  }
  mmaped_address_ = nullptr;
}

bool MMapper::TryMap_(size_t capacity){
  int fd = open(file_path_.string().data(), O_RDWR | O_CREAT, S_IRWXU);
  LOG_DEFER{
    if(fd != -1){
      close(fd); 
    }
  };

  if(fd == -1){
    return false;
  }else{
    ftruncate(fd, capacity);
  }

  mmaped_address_ = ::mmap(NULL, capacity, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  return mmaped_address_ != MAP_FAILED;
}

} // namespace mmap
  
} // namespace logger

