#include "formatter/default_formatter.h"

namespace logger {
namespace formatter {

void DefaultFormatter::Format(const LogMsg& msg, std::string& dest) {
  dest = fmt::format("[{0:%Y-%m-%d %H:%M:%S}] [{1}] [{2}:{3}] [PID:{4} TID:{5}] {6}", std::chrono::system_clock::now(),
                     kLogLevelMap.at(msg.level), msg.location.file_name.data(), msg.location.line,
                     logger::utils::GetProcessID(), logger::utils::GetThreadID(), msg.message.data());
}

}  // namespace formatter
}  // namespace logger
