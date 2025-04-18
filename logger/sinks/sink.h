#pragma once

#include "formatter/formatter.h"
#include "log_common.h"
#include "log_msg.h"

namespace logger {
namespace sink {

class Sink {
 public:
  virtual ~Sink() = default;

  virtual void Log(const LogMsg& msg) = 0;

  virtual void SetFormatter(std::unique_ptr<formatter::Formatter> formatter) = 0;

  virtual void Flush() {}
};

}  // namespace sink
}  // namespace logger