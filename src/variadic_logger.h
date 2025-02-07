#pragma once

#include "fmt/chrono.h"
#include "fmt/core.h"
#include "fmt/base.h"

#include "logger.h"

namespace logger
{
class VariadicLogger : public Logger {
 public:
  using Logger::Logger;

  template <typename... Args>
  void Log(SourceLocation loc, LogLevel lvl, fmt::format_string<Args...> fmt, Args&&... args) {
    Log_(loc, lvl, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Log(LogLevel lvl, fmt::format_string<Args...> fmt, Args&&... args) {
    Log(SourceLocation{}, lvl, fmt, std::forward<Args>(args)...);
  }

 private:
  template <typename... Args>
  void Log_(SourceLocation loc, LogLevel lvl, fmt::basic_string_view<char> fmt, Args&&... args) {
    if (!ShouldLog_(lvl)) {
      return;
    }
    fmt::basic_memory_buffer<char, 256> buf;
    fmt::detail::vformat_to(buf, fmt, fmt::make_format_args(std::forward<Args>(args)...));
    LogMsg msg(loc, lvl, StringView(buf.data(), buf.size()));
    Logger::Log_(msg);
  }
};
} // namespace logger
