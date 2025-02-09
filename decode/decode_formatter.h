#pragma once

#include <string>

#include "effective_msg.pb.h"

class DecodeFormatter {
 public:
  void SetPattern(const std::string& pattern);

  // 根据设置的Pattern处理msg输出到dest中，没设置Pattern默认处理msg
  void Format(const EffectiveMsg& msg, std::string& dest);
};
