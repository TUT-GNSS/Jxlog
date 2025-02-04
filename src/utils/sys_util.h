
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

namespace logger {
namespace utils {

size_t GetProcessID();
size_t GetPageSize();
size_t GetThreadID();
void LocalTime(std::tm* tm, std::time_t* now);

}  // namespace utils
}  // namespace logger
