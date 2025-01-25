#include "formatter/default_formatter.h"

namespace logger
{
// 根据level存储对应的levelmsg
const std::unordered_map<LogLevel, std::string> logger::Formatter::kLogLevelMap {
    {LogLevel::kTrace, "Trace"},
    {LogLevel::kDebug, "Debug"},
    {LogLevel::kInfo, "Info"},
    {LogLevel::kWarn, "Warn"},
    {LogLevel::kError, "Error"},
    {LogLevel::kFatal, "Fatal"},
    {LogLevel::kOff, "Off"}
};

void DefaultFormatter::Format(const LogMsg& msg, std::string& dest){
  dest = fmt::format("[{0:%Y-%m-%d %H:%M:%S}] [{1}] [{2}:{3}] [{4}:{5}] {6}",std::chrono::system_clock::now(),
                                                                            kLogLevelMap.at(msg.level),
                                                                            msg.location.file_name.data(),
                                                                            msg.location.line,
                                                                            34,
                                                                            12,
                                                                            msg.message.data()); 
}

} // namespace logger


