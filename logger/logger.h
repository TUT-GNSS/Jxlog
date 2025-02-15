#pragma once

#include <atomic>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "log_common.h"
#include "log_msg.h"

namespace logger {
namespace sink {
class Sink;  // 前置声明、pimpl模式
}
using SinkPtr = std::shared_ptr<sink::Sink>;
using SinkPtrInitList = std::initializer_list<SinkPtr>;

class Logger {
 public:
  explicit Logger(SinkPtr sink);
  explicit Logger(SinkPtrInitList sinks);
  explicit Logger(Logger&& other) noexcept;
  Logger& operator=(Logger other) noexcept;

  // 根据存储sink的容器迭代器构造
  template <typename It>
  Logger(It begin, It end) : Logger(SinkPtrInitList(begin, end)) {}

  // 析构
  virtual ~Logger() = default;

  // 禁用拷贝构造和赋值
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  void SetLevel(LogLevel level);

  LogLevel GetLevel() const;

  void Log(LogLevel level, SourceLocation loc, StringView message);

  void Flush();

 protected:
  bool ShouldLog_(LogLevel level) const noexcept { return level >= level_ && !sinks_.empty(); }

  virtual void Log_(const LogMsg& msg);

  void Flush_();

  bool ShouldFlush_() { return !sinks_.empty(); };

 private:
  std::string name_;
  std::atomic<LogLevel> level_;
  std::vector<SinkPtr> sinks_;
};

}  // namespace logger
