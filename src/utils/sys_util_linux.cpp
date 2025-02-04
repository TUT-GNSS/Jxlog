

#include <sys/syscall.h>
#include <unistd.h>

#include "utils/sys_util.h"

namespace logger {
namespace utils {

size_t GetPageSize() { return getpagesize(); }

size_t GetProcessID() { return static_cast<size_t>(::getpid()); }

size_t GetThreadID() {
  pthread_t thread = pthread_self();
  uint64_t thread_id;
  pthread_threadid_np(thread, &thread_id);
  return static_cast<size_t>(thread_id);
}

void LocalTime(std::tm* tm, std::time_t* now) { localtime_r(now, tm); }

}  // namespace utils
}  // namespace logger
