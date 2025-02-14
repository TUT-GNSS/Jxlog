#include "logger.h"
#include "sinks/sink.h"

namespace logger {
Logger::Logger(SinkPtr sink) : level_{LogLevel::kInfo}, sinks_{std::move(sink)} {}

Logger::Logger(SinkPtrInitList sinks) : level_{LogLevel::kInfo} {
  for (auto& sink : sinks) {
    sinks_.emplace_back(std::move(sink));
  }
}

Logger::Logger(Logger&& other) noexcept
    : sinks_(std::move(other.sinks_)), level_(other.level_.load(std::memory_order_relaxed)) {}

Logger& Logger::operator=(Logger other) noexcept {
  // copy and swap
  sinks_.swap(other.sinks_);
  // swap level_
  auto other_level = other.level_.load();
  auto my_level = level_.exchange(other_level);
  other.level_.store(my_level);
  return *this;
}

void Logger::SetLevel(LogLevel level) {
  level_ = level;
}

LogLevel Logger::GetLevel() const {
  return level_;
}

void Logger::Log(LogLevel level, SourceLocation loc, StringView message) {
  if (!ShouldLog_(level)) {
    return;
  }

  LogMsg msg(loc, level, message);

  Log_(msg);
}

void Logger::Log_(const LogMsg& msg) {
  for (auto& sink : sinks_) {
    sink->Log(msg);
  }
}

void Logger::Flush() {
  if (!ShouldFlush_()) {
    return;
  }
  Flush_();
}

void Logger::Flush_() {
  for (auto& sink : sinks_) {
    sink->Flush();
  }
}

}  // namespace logger
