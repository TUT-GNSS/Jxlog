#include "async_logger.h"

namespace logger {

AsyncLogger::AsyncLogger(std::string name, SinkPtr sink,
                         std::weak_ptr<ThreadPool> tp,
                         async_overflow_policy overflow_policy)
    : Logger(name, sink), thread_pool_(tp), overflow_policy_(overflow_policy) {}

AsyncLogger::AsyncLogger(std::string name, SinkPtrInitList sinks,
                         std::weak_ptr<ThreadPool> tp,
                         async_overflow_policy overflow_policy)
    : Logger(name, sinks),
      thread_pool_(tp),
      overflow_policy_(overflow_policy) {}

AsyncLogger::~AsyncLogger() { std::cout << "AsyncLogger析构" << "\n"; }

void AsyncLogger::Log_(const LogMsg& msg) {
  std::cout << "AsyncLogger Log_" << "\n";
}

}  // namespace logger