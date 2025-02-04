#include <sysinfoapi.h>
#include <windows.h>

#include "utils/sys_util.h"

namespace logger {
namespace utils {

size_t GetPageSize() {
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return info.dwPageSize;
}

size_t GetProcessID() {
  return static_cast<size_t>(::GetCurrentProcessId());
}

size_t GetThreadID() {
  return static_cast<size_t>(::GetCurrentThreadId());
}

void LocalTime(std::tm* tm, std::time_t* now) {
  localtime_s(tm, now);
}

}  // namespace utils
}  // namespace logger