#pragma once


#include "logger.h"

namespace logger
{
enum class async_overflow_policy
{
    kBlock,         // 阻塞 
    kOverrun_Oldest // 以旧换新
};

class thread_pool;

class AsyncLogger final : public std::enable_shared_from_this<AsyncLogger> , public Logger{
  friend class thread_pool;
public:
  void Log_ override(const LogMsg& msg); 

private:
  std::weak_ptr<thread_pool> thread_pool_;
  async_overflow_policy overflow_policy_;   
};
} // namespace logger
