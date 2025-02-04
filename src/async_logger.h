#pragma once

#include "logger.h"

namespace logger {
enum class async_overflow_policy {
  kBlock,          // 阻塞
  kOverrun_Oldest  // 以旧换新
};

class ThreadPool {
 public:
};

class AsyncLogger final : public std::enable_shared_from_this<AsyncLogger>,
                          public Logger {
  friend class ThreadPool;

 public:
  template <typename It>
  AsyncLogger(
      std::string name, It begin, It end, std::weak_ptr<ThreadPool> tp,
      async_overflow_policy overflow_policy = async_overflow_policy::kBlock)
      : Logger(std::move(name), begin, end),
        thread_pool_(std::move(tp)),
        overflow_policy_(overflow_policy) {}
  AsyncLogger(
      std::string name, SinkPtr sink, std::weak_ptr<ThreadPool> tp,
      async_overflow_policy overflow_policy = async_overflow_policy::kBlock);

  AsyncLogger(
      std::string name, SinkPtrInitList sinks, std::weak_ptr<ThreadPool> tp,
      async_overflow_policy overflow_policy = async_overflow_policy::kBlock);

  ~AsyncLogger();

 protected:
  void Log_(const LogMsg& msg) override;

 private:
  std::weak_ptr<ThreadPool> thread_pool_;
  async_overflow_policy overflow_policy_;
};
}  // namespace logger
