#include "formatter/formatter.h"

namespace logger {
namespace formatter {

// 根据level存储对应的levelmsg
const std::unordered_map<LogLevel, std::string>
    Formatter::kLogLevelMap{
        {LogLevel::kTrace, "Trace"}, {LogLevel::kDebug, "Debug"},
        {LogLevel::kInfo, "Info"},   {LogLevel::kWarn, "Warn"},
        {LogLevel::kError, "Error"}, {LogLevel::kFatal, "Fatal"},
        {LogLevel::kOff, "Off"}};

}  // namespace formatter
}  // namespace logger
