#pragma once

#include "log_common.h"
#include "log_msg.h"
#include "fmt/core.h"
#include "fmt/chrono.h"

namespace logger{

class Formatter{
public:
  virtual ~Formatter() = default;
  virtual void Format (const LogMsg& msg, std::string& dest) = 0;
protected:
  static const std::unordered_map<LogLevel, std::string> kLogLevelMap;
};

}// namespace logger