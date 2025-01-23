#include "logger.h"

namespace logger
{

Logger::Logger(std::string name, LogSinkPtrInitList sinks):name_{std::move(name)},level_{LogLevel::kInfo}{
  for(auto& sink : sinks){
    sinks.emplace_back(std::move(sink));
  }
}

Logger::Logger(Logger &&other) noexcept :name_(std::move(other.name)),
                                                sinks_(std::move(other.sinks_)),
                                                level_(other.level_){}

Logger::Logger& operator = (Logger other) noexcept {
  //copy and swap
  name_.swap(other.name_);
  sinks_.swap(other.sinks_);
  // swap level_
  auto other_level = other.level_.load();
  auto my_level = level_.exchange(other_level);
  other.level_.store(my_level);
}  

void Logger::set_level(LogLevel level){
  level_ = level;
}

LogLevel Logger::GetLevel() const{
  return level_;
}

void Logger::Log(LogLevel level, SourceLocation loc, StringView message){
  if(!ShouldLog_(level)){
    return;
  }

  LogMsg msg(loc,level,message);

  Log_(msg);
}

void Logger::Log_(const LogMsg& msg){
//   sink_it_(msg);
  for(auto& sink:sinks){
    sink->Log(msg);
  }
}

// void sink_it_(const LogMsg &msg){
//   for(auto& sink:sinks){
//     sink->Log(msg);
//   }
// }

} // namespace logger
