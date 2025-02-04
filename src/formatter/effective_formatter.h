#pragma once

#include "formatter/formatter.h"

namespace logger {
namespace formatter {
class EffectiveFormatter : public Formatter {
 public:
  void Format(const LogMsg& msg, std::string& dest) override;
};

}  // namespace formatter
}  // namespace logger