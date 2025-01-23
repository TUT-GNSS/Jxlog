#pragma once

#include <atomic>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <iostream>

#include <vector>

#include "log_common.h"
#include "log_msg.h"
#include "sinks/sink.h"
namespace logger
{
class LogSink;//前置声明、pimpl模式
using LogSinkPtr = std::shared_ptr<LogSink>;
using LogSinkPtrInitList = std::initializer_list<LogSink>;

//抽象Logger提供统一接口
class Logger
{
public:
  explicit Logger(std::string name) : name_{std::move(name)}, level_{LogLevel::kInfo} {}
  explicit Logger(std::string name, LogSinkPtr sink) : Logger(name) {sinks_.emplace_back(sink);}
  explicit Logger(std::string name, LogSinkPtrInitList sinks);
  explicit Logger(Logger &&other) noexcept;
  Logger& operator = (Logger other) noexcept;

  //根据存储sink的容器迭代器构造
  template <typename It>
  Logger(std::string name, It begin, It end) : Logger(name, LogSinkPtrInitList(begin, end)) {}

  //虚析构
  ~Logger() = default;

  //禁用拷贝构造和赋值
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  void SetLevel(LogLevel level);

  LogLevel GetLevel() const;

  void Log(LogLevel level, SourceLocation loc, StringView message);

protected:
  bool ShouldLog_(LogLevel level) const noexcept {
      return level >= level_ && !sinks_.empty(); 
  }

  virtual void Log_(const LogMsg& msg);

  // void sink_it_(const LogMsg &msg);
  //virtual void flush_();
  //bool should_flush_(const LogMsg &msg);

 private:
  std::string name_;
  std::atomic<LogLevel> level_;
  std::vector<LogSinkPtr> sinks_;
};

} // namespace logger
