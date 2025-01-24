#pragma once

#include <string.h>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <iostream>

#define LOGGER_LEVEL_TRACE 0
#define LOGGER_LEVEL_DEBUG 1
#define LOGGER_LEVEL_INFO 2
#define LOGGER_LEVEL_WARN 3
#define LOGGER_LEVEL_ERROR 4
#define LOGGER_LEVEL_CRITICAL 5
#define LOGGER_LEVEL_OFF 6


namespace logger
{
using StringView = std::string_view;

enum class LogLevel{
  kTrace = LOGGER_LEVEL_TRACE,
  kDebug = LOGGER_LEVEL_DEBUG,
  kInfo = LOGGER_LEVEL_INFO,
  kWarn = LOGGER_LEVEL_WARN,
  kError = LOGGER_LEVEL_ERROR,
  kFatal = LOGGER_LEVEL_CRITICAL,
  kOff = LOGGER_LEVEL_OFF
};  


struct SourceLocation {
  constexpr SourceLocation() = default;

  SourceLocation(StringView file_name_in, int32_t line_in, StringView func_name_in)
      : file_name{file_name_in}, line{line_in}, func_name{func_name_in} {
    
    // 获取文件名
    if (!file_name.empty()) {
      size_t pos = file_name.rfind('/'); //Unix文件路径分隔
      if (pos != StringView::npos) {
        //截取'/'之后
        file_name = file_name.substr(pos + 1);
      } else {
        pos = file_name.rfind('\\');//windows文件路径分割
        if (pos != StringView::npos) {
          file_name = file_name.substr(pos + 1);
        }
      }
    }
  }

  StringView file_name;
  int32_t line{0};
  StringView func_name;
};

} // namespace logger

