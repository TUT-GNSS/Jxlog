#pragma once

#include <memory>

namespace logger
{
class VariadicLogger; // pimpl
class LogFactory{
 public:
  LogFactory(const LogFactory&) = delete;
  LogFactory& operator=(const LogFactory&) = delete;
  
  static LogFactory& Instance();

  VariadicLogger* GetLogger();

  void SetLogger(std::shared_ptr<VariadicLogger> logger);

 private:
  LogFactory();

  std::shared_ptr<VariadicLogger> logger_;
};
} // namespace logger