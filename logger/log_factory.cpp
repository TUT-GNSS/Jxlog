#include "log_factory.h"
#include "variadic_logger.h"

namespace logger
{
LogFactory::LogFactory(){

}

LogFactory& LogFactory::Instance(){
  static LogFactory instance;
  return instance;
}

VariadicLogger* LogFactory::GetLogger(){
  return logger_.get();
}

void LogFactory::SetLogger(std::shared_ptr<VariadicLogger> logger){
  logger_ = logger;
}

} // namespace logger
