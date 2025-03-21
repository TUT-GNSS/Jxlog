#pragma once

#include "fmt/chrono.h"
#include "fmt/core.h"
#include "log_common.h"
#include "log_msg.h"
#include "utils/sys_util.h"

namespace logger {
namespace formatter {

class Formatter {
 public:
  virtual ~Formatter() = default;
  virtual void Format(const LogMsg& msg, std::string& dest) = 0;

 protected:
  static const std::unordered_map<LogLevel, std::string> kLogLevelMap;
};

}  // namespace formatter
}  // namespace logger