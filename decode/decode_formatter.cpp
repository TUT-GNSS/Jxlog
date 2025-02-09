#include "decode_formatter.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "effective_msg.pb.h"

/**
 * @brief EffectiveMsg custom pattern
 *
 * %l log_level
 * %D timestamp date
 * %S timestamp seconds
 * %M timestamp milliseconds
 * %p process_id
 * %t thread_id
 * %# line
 * %F file_name
 * %f func_name
 * %v log_info
 *
 * such as: [%l][%D:%S][%p:%t][%F:%f:%#]%v
 */

// 格式化器基类：定义日志字段格式化的统一接口
class FlagFormatter {
 public:
  FlagFormatter() = default;
  virtual ~FlagFormatter() = default;

  // 将日志消息的特定字段格式化到目标字符串
  virtual void Format(const EffectiveMsg& flag, std::string& dest) = 0;
};

// 聚合格式化器：处理非占位符的普通字符
class AggregateFormatter final : public FlagFormatter {
 public:
  AggregateFormatter() = default;
  ~AggregateFormatter() override = default;

  // 添加普通字符到缓存字符串
  void AddCh(char ch) { str_ += ch; }

  // 将缓存的普通字符直接追加到目标字符串
  void Format(const EffectiveMsg& msg, std::string& dest) override { dest.append(str_); }

 private:
  std::string str_;  // 存储普通字符的缓存
};

// 日志级别格式化器：处理 %l 占位符
class LogLevelFormatter final : public FlagFormatter {
 public:
  void Format(const EffectiveMsg& msg, std::string& dest) override { 
    dest.append(msg.level());  // 将日志等级转换为字符串
  }
};

// 时间戳转日期工具函数
std::string MillisecondsToDateString(long long milliseconds) {
  // 将毫秒时间戳转换为系统时钟时间点
  auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(milliseconds));
  // 转换为C风格时间结构
  std::time_t time_tt = std::chrono::system_clock::to_time_t(tp);
  std::tm* timeinfo = std::localtime(&time_tt);
  // 格式化为 YYYY-MM-DD HH:MM:SS
  std::ostringstream oss;
  oss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

// 日期时间格式化器：处理 %D 占位符
class TimestampDateFormatter final : public FlagFormatter {
 public:
  void Format(const EffectiveMsg& msg, std::string& dest) override {
    dest.append(MillisecondsToDateString(msg.timestamp()));
  }
};

// 秒级时间戳格式化器：处理 %S 占位符
class TimestampSecondsFormatter final : public FlagFormatter {
 public:
  void Format(const EffectiveMsg& msg, std::string& dest) override {
    dest.append(std::to_string(msg.timestamp() / 1000));  // 转换为秒
  }
};

// 毫秒级时间戳格式化器：处理 %M 占位符
class TimestampMillisecondsFormatter final : public FlagFormatter {
 public:
  void Format(const EffectiveMsg& msg, std::string& dest) override { 
    dest.append(std::to_string(msg.timestamp()));  // 直接输出原始毫秒值
  }
};

// 进程ID格式化器：处理 %p 占位符
class ProcessIdFormatter final : public FlagFormatter {
 public:
  void Format(const EffectiveMsg& msg, std::string& dest) override { 
    dest.append(std::to_string(msg.pid())); 
  }
};

// 线程ID格式化器：处理 %t 占位符
class ThreadIdFormatter final : public FlagFormatter {
 public:
  void Format(const EffectiveMsg& msg, std::string& dest) override { 
    dest.append(std::to_string(msg.tid())); 
  }
};

// 行号格式化器：处理 %# 占位符
class LineFormatter final : public FlagFormatter {
 public:
  void Format(const EffectiveMsg& msg, std::string& dest) override { 
    dest.append(std::to_string(msg.line())); 
  }
};

// 文件名格式化器：处理 %F 占位符
class FileNameFormatter final : public FlagFormatter {
 public:
  void Format(const EffectiveMsg& msg, std::string& dest) override { 
    dest.append(msg.file_name());  // 直接追加文件名
  }
};

// 函数名格式化器：处理 %f 占位符
class FuncNameFormatter final : public FlagFormatter {
 public:
  void Format(const EffectiveMsg& msg, std::string& dest) override { 
    dest.append(msg.func_name()); 
  }
};

// 日志内容格式化器：处理 %v 占位符
class LogInfoFormatter final : public FlagFormatter {
 public:
  void Format(const EffectiveMsg& msg, std::string& dest) override { 
    dest.append(msg.log_info());  // 追加日志正文内容
  }
};

// 全局格式化器容器（注意：实际应封装在类中）
std::vector<std::unique_ptr<FlagFormatter>> flag_formatters_;

// 占位符解析函数
static void HandleFlag(char flag) {
  switch (flag) {
    case 'l': flag_formatters_.push_back(std::make_unique<LogLevelFormatter>()); break;
    case 'D': flag_formatters_.push_back(std::make_unique<TimestampDateFormatter>()); break;
    case 'S': flag_formatters_.push_back(std::make_unique<TimestampSecondsFormatter>()); break;
    case 'M': flag_formatters_.push_back(std::make_unique<TimestampMillisecondsFormatter>()); break;
    case 'p': flag_formatters_.push_back(std::make_unique<ProcessIdFormatter>()); break;
    case 't': flag_formatters_.push_back(std::make_unique<ThreadIdFormatter>()); break;
    case '#': flag_formatters_.push_back(std::make_unique<LineFormatter>()); break;
    case 'F': flag_formatters_.push_back(std::make_unique<FileNameFormatter>()); break;
    case 'f': flag_formatters_.push_back(std::make_unique<FuncNameFormatter>()); break;
    case 'v': flag_formatters_.push_back(std::make_unique<LogInfoFormatter>()); break;
    default:  // 处理未知占位符
      auto formatter = std::make_unique<AggregateFormatter>();
      formatter->AddCh('%');  // 保留%符号
      formatter->AddCh(flag); // 追加未知字符
      flag_formatters_.push_back(std::move(formatter));
      break;
  }
}

// 默认日志组合函数（传统格式）
std::string CombineLogMsg(const EffectiveMsg& msg) {
  std::string output;
  char buffer[1024] = {0};
  // 格式示例：[INFO][1651234567890][1234:5678][file.cpp:main:42]
  snprintf(buffer, sizeof(buffer), "[%s][%lld][%d:%d][%s:%s:%d]", 
          msg.level().data(), msg.timestamp(), msg.pid(), msg.tid(),
          msg.file_name().c_str(), msg.func_name().c_str(), msg.line());
  output.append(buffer);
  output.append(msg.log_info());  // 追加日志正文
  return output;
}

// 模式编译核心函数
void CompilePattern(const std::string& pattern) {
  auto end = pattern.end();
  std::unique_ptr<AggregateFormatter> user_chars;  // 临时聚合器
  flag_formatters_.clear();  // 清空原有格式化器

  for (auto it = pattern.begin(); it != end; ++it) {
    if (*it == '%') {  // 检测到占位符起始
      if (user_chars) {  // 提交缓存的普通字符
        flag_formatters_.push_back(std::move(user_chars));
      }
      if (++it == end) break;  // 跳过%并检查边界
      HandleFlag(*it);        // 处理占位符
    } else {  // 普通字符处理
      if (!user_chars) {
        user_chars = std::make_unique<AggregateFormatter>();
      }
      user_chars->AddCh(*it);  // 累积字符
    }
  }
  
  // 处理末尾的普通字符
  if (user_chars) {
    flag_formatters_.push_back(std::move(user_chars));
  }
}

// 解码格式化器类实现
void DecodeFormatter::SetPattern(const std::string& pattern) {
  CompilePattern(pattern);  // 编译自定义模式
}

void DecodeFormatter::Format(const EffectiveMsg& msg, std::string& dest) {
  if (!flag_formatters_.empty()) {  // 使用自定义格式化器
    for (auto& formatter : flag_formatters_) {
      formatter->Format(msg, dest);  // 依次执行格式化
    }
  } else {  // 使用默认格式
    dest.append(CombineLogMsg(msg));
  }
  dest.append("\n");  // 追加换行符
}

